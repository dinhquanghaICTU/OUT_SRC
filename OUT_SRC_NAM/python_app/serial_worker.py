import json
import time
import math
from PyQt5.QtCore import QThread, pyqtSignal
import serial
import serial.tools.list_ports

class SerialWorker(QThread):
    sig_telemetry = pyqtSignal(dict)      # Data telemetry tu ESP32
    sig_ack = pyqtSignal(dict)            # Phan hoi xac nhan lenh
    sig_connection_changed = pyqtSignal(bool, str) # Trang thai ket noi (status, port)
    sig_log = pyqtSignal(str)             # Chuoi log Serial
    sig_error = pyqtSignal(str)           # Thong bao loi

    def __init__(self, parent=None):
        super().__init__(parent)
        self.port_name = ""
        self.baudrate = 115200
        self.is_connected = False
        self.is_simulated = False
        self.running = True
        self.ser = None

        # Du lieu mo phong (Simulation)
        self.sim_pwm = 0
        self.sim_rpm = 0.0
        self.sim_pulses = 0
        self.sim_gear_ratio = 34.0
        self.sim_ppr = 11.0
        self.sim_wheel_dia = 0.065

    @staticmethod
    def get_available_ports():
        """Lay danh sach cac cong Serial hien co"""
        ports = serial.tools.list_ports.comports()
        result = []
        for p in ports:
            desc = f"{p.device} ({p.description})"
            result.append((p.device, desc))
        return result

    def connect_serial(self, port_name, baudrate=115200, simulated=False):
        """Yeu cau ket noi Serial hoac chay che do mo phong"""
        self.disconnect_serial()
        self.port_name = port_name
        self.baudrate = baudrate
        self.is_simulated = simulated

        if self.is_simulated:
            self.is_connected = True
            self.sig_connection_changed.emit(True, "MÔ PHỎNG (SIMULATION)")
            self.sig_log.emit("[INFO] Đã bật chế độ MÔ PHỎNG (Không cần cắm phần cứng)")
            return True

        try:
            self.ser = serial.Serial(
                port=self.port_name,
                baudrate=self.baudrate,
                timeout=0.1,
                write_timeout=0.5
            )
            # DTR/RTS reset nhe ESP32 neu can
            self.ser.setDTR(False)
            self.ser.setRTS(False)
            self.is_connected = True
            self.sig_connection_changed.emit(True, self.port_name)
            self.sig_log.emit(f"[INFO] Kết nối thành công tới {self.port_name} @ {self.baudrate} baud")
            return True
        except Exception as e:
            self.is_connected = False
            self.sig_connection_changed.emit(False, str(e))
            self.sig_error.emit(f"Không thể mở cổng {self.port_name}: {e}")
            return False

    def disconnect_serial(self):
        """Ngat ket noi Serial"""
        self.is_connected = False
        if self.ser and self.ser.is_open:
            try:
                # Gui lenh dung dong co truoc khi ngat
                self._send_command_raw('{"cmd":"stop"}\n')
                time.sleep(0.05)
                self.ser.close()
            except Exception:
                pass
        self.ser = None
        self.sig_connection_changed.emit(False, "")
        self.sig_log.emit("[INFO] Đã ngắt kết nối")

    def send_command(self, cmd_dict):
        """Gui lenh JSON xuong ESP32"""
        if not self.is_connected:
            return

        json_str = json.dumps(cmd_dict) + "\n"
        if self.is_simulated:
            self._handle_simulated_command(cmd_dict)
            return

        self._send_command_raw(json_str)

    def _send_command_raw(self, line):
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(line.encode('utf-8'))
                self.ser.flush()
                self.sig_log.emit(f"[TX] >> {line.strip()}")
            except Exception as e:
                self.sig_error.emit(f"Lỗi gửi dữ liệu: {e}")

    def _handle_simulated_command(self, cmd):
        """Xu ly lenh trong che do mo phong"""
        c = cmd.get("cmd", "")
        self.sig_log.emit(f"[SIM TX] >> {json.dumps(cmd)}")
        if c in ["set_speed", "speed"]:
            pwm = cmd.get("pwm", cmd.get("speed", 0))
            self.sim_pwm = max(-255, min(255, int(pwm)))
            ack = {"type": "ack", "cmd": "set_speed", "status": "ok", "pwm": self.sim_pwm}
            self.sig_ack.emit(ack)
            self.sig_log.emit(f"[SIM RX] << {json.dumps(ack)}")
        elif c == "stop":
            self.sim_pwm = 0
            ack = {"type": "ack", "cmd": "stop", "status": "ok", "pwm": 0}
            self.sig_ack.emit(ack)
            self.sig_log.emit(f"[SIM RX] << {json.dumps(ack)}")
        elif c == "reset_encoder":
            self.sim_pulses = 0
            ack = {"type": "ack", "cmd": "reset_encoder", "status": "ok", "pwm": self.sim_pwm}
            self.sig_ack.emit(ack)
            self.sig_log.emit(f"[SIM RX] << {json.dumps(ack)}")

    def stop_worker(self):
        self.running = False
        self.disconnect_serial()
        self.wait(1000)

    def run(self):
        """Vong lap doc du lieu Serial chay nen"""
        last_sim_time = time.time()

        while self.running:
            if not self.is_connected:
                time.sleep(0.05)
                continue

            # 1. Che do mo phong (Simulation Mode)
            if self.is_simulated:
                now = time.time()
                dt = now - last_sim_time
                if dt >= 0.1: # 100ms
                    last_sim_time = now
                    # Mo phong vat ly dong co DC (quan tinh bac 1: tau = 0.2s)
                    # Toc do toi da khoang 300 RPM o PWM 255
                    target_rpm = (self.sim_pwm / 255.0) * 330.0
                    alpha = 1.0 - math.exp(-dt / 0.25)
                    self.sim_rpm += alpha * (target_rpm - self.sim_rpm)

                    # Tinh xung encoder tich luy
                    total_ppr = self.sim_ppr * self.sim_gear_ratio
                    delta_pulses = int((self.sim_rpm / 60.0) * total_ppr * dt)
                    self.sim_pulses += delta_pulses

                    # Van toc m/s
                    speed_mps = (self.sim_rpm / 60.0) * (math.pi * self.sim_wheel_dia)
                    speed_cms = speed_mps * 100.0

                    telem = {
                        "type": "telemetry",
                        "rpm": round(self.sim_rpm, 1),
                        "speed_mps": round(speed_mps, 3),
                        "speed_cms": round(speed_cms, 1),
                        "pulses": self.sim_pulses,
                        "delta": delta_pulses,
                        "pwm": self.sim_pwm,
                        "dir": 1 if self.sim_pwm > 0 else (-1 if self.sim_pwm < 0 else 0),
                        "dt_ms": int(dt * 1000)
                    }
                    self.sig_telemetry.emit(telem)
                time.sleep(0.01)
                continue

            # 2. Che do Serial thuc te
            if self.ser and self.ser.is_open:
                try:
                    if self.ser.in_waiting > 0:
                        raw_bytes = self.ser.readline()
                        if not raw_bytes:
                            continue
                        line = raw_bytes.decode('utf-8', errors='ignore').strip()
                        if not line:
                            continue

                        # Log raw line
                        self.sig_log.emit(f"[RX] << {line}")

                        # Thu parse JSON
                        if line.startswith('{') and line.endswith('}'):
                            try:
                                data = json.loads(line)
                                msg_type = data.get("type", "")
                                if msg_type == "telemetry":
                                    self.sig_telemetry.emit(data)
                                elif msg_type == "ack":
                                    self.sig_ack.emit(data)
                                elif msg_type == "system":
                                    self.sig_log.emit(f"[SYSTEM] Thiết bị sẵn sàng: {line}")
                            except json.JSONDecodeError:
                                pass
                except Exception as e:
                    self.sig_error.emit(f"Lỗi đọc Serial: {e}")
                    time.sleep(0.1)
            else:
                time.sleep(0.05)
