import sys
import os
import csv
import time
from collections import deque
from datetime import datetime

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QGridLayout, QLabel, QPushButton, QSlider, QSpinBox, QComboBox,
    QCheckBox, QGroupBox, QSplitter, QTextEdit, QFileDialog, QMessageBox,
    QFrame, QSizePolicy
)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont, QColor, QIcon

import pyqtgraph as pg

# Import worker thread
from serial_worker import SerialWorker

# Cấu hình pyqtgraph tối ưu hiệu năng
pg.setConfigOption('background', '#111722')
pg.setConfigOption('foreground', '#94a3b8')
pg.setConfigOption('antialias', True)

DARK_STYLESHEET = """
QMainWindow {
    background-color: #0b0f17;
    color: #e2e8f0;
    font-family: 'Segoe UI', 'Ubuntu', 'Helvetica Neue', sans-serif;
}

QWidget {
    color: #e2e8f0;
    font-size: 13px;
}

QGroupBox {
    background-color: #141b27;
    border: 1px solid #233045;
    border-radius: 10px;
    margin-top: 14px;
    padding-top: 14px;
    font-weight: bold;
    color: #38bdf8;
    font-size: 13px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 14px;
    padding: 0 6px;
    background-color: #141b27;
    border-radius: 4px;
}

QComboBox {
    background-color: #1b2434;
    border: 1px solid #2e405b;
    border-radius: 6px;
    padding: 6px 12px;
    color: #f8fafc;
    min-width: 120px;
}
QComboBox:hover {
    border-color: #38bdf8;
}
QComboBox::drop-down {
    border: none;
    width: 20px;
}

QPushButton {
    background-color: #1e293b;
    border: 1px solid #334155;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: 600;
    color: #f1f5f9;
}
QPushButton:hover {
    background-color: #2e3f59;
    border-color: #38bdf8;
}
QPushButton:pressed {
    background-color: #0f172a;
}
QPushButton:disabled {
    background-color: #131924;
    color: #475569;
    border-color: #1e293b;
}

/* Nút kết nối */
QPushButton#btn_connect {
    background-color: #0284c7;
    border: 1px solid #38bdf8;
    color: white;
}
QPushButton#btn_connect:hover {
    background-color: #0369a1;
}

QPushButton#btn_disconnect {
    background-color: #dc2626;
    border: 1px solid #f87171;
    color: white;
}
QPushButton#btn_disconnect:hover {
    background-color: #b91c1c;
}

/* Nút Dừng khẩn cấp */
QPushButton#btn_estop {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ef4444, stop:1 #dc2626);
    border: 2px solid #fca5a5;
    border-radius: 8px;
    font-size: 15px;
    font-weight: 800;
    color: white;
    padding: 12px;
}
QPushButton#btn_estop:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #dc2626, stop:1 #991b1b);
}

/* Nút Preset */
QPushButton.preset-btn {
    background-color: #1e293b;
    border: 1px solid #334155;
    padding: 6px 10px;
    font-size: 12px;
}
QPushButton.preset-btn:hover {
    background-color: #0284c7;
    border-color: #38bdf8;
    color: white;
}

/* Nút Reset encoder */
QPushButton#btn_reset_enc {
    background-color: #334155;
    border: 1px solid #475569;
}
QPushButton#btn_reset_enc:hover {
    background-color: #475569;
    border-color: #cbd5e1;
}

QSlider::groove:horizontal {
    border: 1px solid #233045;
    height: 10px;
    background: #0f172a;
    border-radius: 5px;
}

QSlider::sub-page:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #38bdf8);
    border-radius: 5px;
}

QSlider::handle:horizontal {
    background: #38bdf8;
    border: 2px solid #ffffff;
    width: 22px;
    margin-top: -6px;
    margin-bottom: -6px;
    border-radius: 11px;
}
QSlider::handle:horizontal:hover {
    background: #7dd3fc;
}

QSpinBox {
    background-color: #1b2434;
    border: 1px solid #2e405b;
    border-radius: 6px;
    padding: 6px 10px;
    color: #f8fafc;
    font-weight: bold;
    font-size: 14px;
}
QSpinBox:focus {
    border-color: #38bdf8;
}

QCheckBox {
    spacing: 8px;
    color: #cbd5e1;
}
QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 1px solid #334155;
    background-color: #1e293b;
}
QCheckBox::indicator:checked {
    background-color: #0284c7;
    border-color: #38bdf8;
}

QTextEdit {
    background-color: #0a0e17;
    border: 1px solid #1e293b;
    border-radius: 6px;
    color: #a5f3fc;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 11px;
}
"""

class MetricCard(QFrame):
    """Thẻ hiển thị thông số kỹ thuật dạng số lớn rực rỡ"""
    def __init__(self, title, unit, color="#38bdf8", parent=None):
        super().__init__(parent)
        self.setStyleSheet(f"""
            QFrame {{
                background-color: #141c29;
                border: 1px solid #233045;
                border-radius: 10px;
                padding: 10px;
            }}
            QFrame:hover {{
                border-color: {color};
            }}
        """)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(10, 8, 10, 8)
        layout.setSpacing(4)

        self.lbl_title = QLabel(title.upper())
        self.lbl_title.setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: bold; letter-spacing: 1px;")

        val_layout = QHBoxLayout()
        val_layout.setSpacing(4)

        self.lbl_value = QLabel("0.0")
        self.lbl_value.setStyleSheet(f"color: {color}; font-size: 26px; font-weight: 800; font-family: monospace;")

        self.lbl_unit = QLabel(unit)
        self.lbl_unit.setStyleSheet("color: #64748b; font-size: 13px; font-weight: 600; padding-bottom: 2px;")
        self.lbl_unit.setAlignment(Qt.AlignBottom)

        val_layout.addWidget(self.lbl_value)
        val_layout.addWidget(self.lbl_unit)
        val_layout.addStretch()

        self.lbl_subtext = QLabel("--")
        self.lbl_subtext.setStyleSheet("color: #64748b; font-size: 11px;")

        layout.addWidget(self.lbl_title)
        layout.addLayout(val_layout)
        layout.addWidget(self.lbl_subtext)

    def set_value(self, val_str, subtext=""):
        self.lbl_value.setText(val_str)
        if subtext:
            self.lbl_subtext.setText(subtext)


class MotorDashboard(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("HỆ THỐNG GIÁM SÁT VÀ ĐIỀU KHIỂN TỐC ĐỘ ĐỘNG CƠ ESP32 - NAM PROJECT")
        self.resize(1280, 820)
        self.setMinimumSize(1000, 680)

        # Worker Serial
        self.worker = SerialWorker()
        self.worker.sig_telemetry.connect(self.on_telemetry_received)
        self.worker.sig_ack.connect(self.on_ack_received)
        self.worker.sig_connection_changed.connect(self.on_connection_changed)
        self.worker.sig_log.connect(self.log_terminal)
        self.worker.sig_error.connect(self.on_error)
        self.worker.start()

        # Dữ liệu đồ thị (Rolling Buffer)
        self.max_points = 300  # ~30s dữ liệu ở 10Hz
        self.time_data = deque(maxlen=self.max_points)
        self.rpm_data = deque(maxlen=self.max_points)
        self.speed_data = deque(maxlen=self.max_points)
        self.pwm_data = deque(maxlen=self.max_points)

        self.all_history = []  # Lưu toàn bộ phiên để xuất CSV
        self.start_time = time.time()
        self.graph_paused = False

        # Throttle gửi lệnh slider (tránh ngập serial khi kéo chuột)
        self.slider_send_timer = QTimer(self)
        self.slider_send_timer.setSingleShot(True)
        self.slider_send_timer.timeout.connect(self.send_slider_speed)

        # Xây dựng giao diện
        self.init_ui()

        # Áp dụng stylesheet
        self.setStyleSheet(DARK_STYLESHEET)

        # Load danh sách cổng COM ban đầu
        self.refresh_ports()

    def init_ui(self):
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        root_layout = QVBoxLayout(main_widget)
        root_layout.setContentsMargins(14, 14, 14, 14)
        root_layout.setSpacing(12)

        # 1. THANH TRẠNG THÁI KẾT NỐI (TOP BAR)
        root_layout.addWidget(self.create_top_bar())

        # 2. KHU VỰC CHÍNH (SPLITTER: TRÁI - ĐIỀU KHIỂN & ĐO LƯỜNG, PHẢI - ĐỒ THỊ & LOG)
        splitter = QSplitter(Qt.Horizontal)
        splitter.setHandleWidth(8)

        left_panel = self.create_left_panel()
        right_panel = self.create_right_panel()

        splitter.addWidget(left_panel)
        splitter.addWidget(right_panel)
        splitter.setStretchFactor(0, 4)
        splitter.setStretchFactor(1, 6)

        root_layout.addWidget(splitter, 1)

    def create_top_bar(self):
        bar = QFrame()
        bar.setStyleSheet("""
            QFrame {
                background-color: #141b27;
                border: 1px solid #233045;
                border-radius: 10px;
                padding: 6px 12px;
            }
        """)
        layout = QHBoxLayout(bar)
        layout.setContentsMargins(8, 4, 8, 4)
        layout.setSpacing(12)

        # Tiêu đề & Logo
        title_label = QLabel("⚡ ESP32 MOTOR CONTROLLER")
        title_label.setStyleSheet("font-size: 15px; font-weight: 900; color: #38bdf8; letter-spacing: 0.5px;")
        layout.addWidget(title_label)

        layout.addSpacing(15)

        # Cổng COM
        layout.addWidget(QLabel("Cổng Serial:"))
        self.combo_ports = QComboBox()
        layout.addWidget(self.combo_ports)

        self.btn_refresh = QPushButton("🔄")
        self.btn_refresh.setToolTip("Quét lại danh sách cổng Serial")
        self.btn_refresh.setFixedWidth(38)
        self.btn_refresh.clicked.connect(self.refresh_ports)
        layout.addWidget(self.btn_refresh)

        # Baudrate
        layout.addWidget(QLabel("Baudrate:"))
        self.combo_baud = QComboBox()
        self.combo_baud.addItems(["115200", "57600", "9600", "230400", "460800"])
        self.combo_baud.setCurrentText("115200")
        layout.addWidget(self.combo_baud)

        # Checkbox Chế độ mô phỏng
        self.chk_simulate = QCheckBox("🧪 Chế độ mô phỏng")
        self.chk_simulate.setToolTip("Bật chế độ mô phỏng để test app đầy đủ mà không cần cắm ESP32 thực tế")
        layout.addWidget(self.chk_simulate)

        # Nút Kết nối
        self.btn_connect = QPushButton("KẾT NỐI")
        self.btn_connect.setObjectName("btn_connect")
        self.btn_connect.clicked.connect(self.toggle_connection)
        layout.addWidget(self.btn_connect)

        layout.addStretch()

        # Đèn báo trạng thái LED
        self.lbl_status_led = QLabel("● CHƯA KẾT NỐI")
        self.lbl_status_led.setStyleSheet("color: #ef4444; font-weight: bold; font-size: 13px;")
        layout.addWidget(self.lbl_status_led)

        return bar

    def create_left_panel(self):
        panel = QWidget()
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(12)

        # 1. BẢNG THẺ METRICS (4 THẺ)
        metrics_group = QGroupBox("CHỈ SỐ ĐO LƯỜNG THỰC TẾ (TELEMETRY)")
        grid = QGridLayout(metrics_group)
        grid.setSpacing(10)

        self.card_rpm = MetricCard("Vận tốc quay", "RPM", color="#00f0ff")
        self.card_speed = MetricCard("Vận tốc dài", "m/s", color="#10b981")
        self.card_pulses = MetricCard("Xung Encoder", "pulses", color="#f59e0b")
        self.card_pwm = MetricCard("Công suất PWM", "PWM", color="#ec4899")

        grid.addWidget(self.card_rpm, 0, 0)
        grid.addWidget(self.card_speed, 0, 1)
        grid.addWidget(self.card_pulses, 1, 0)
        grid.addWidget(self.card_pwm, 1, 1)

        layout.addWidget(metrics_group)

        # 2. KHU VỰC ĐIỀU KHIỂN TỐC ĐỘ (SPEED CONTROLLER)
        ctrl_group = QGroupBox("BẢNG ĐIỀU KHIỂN TỐC ĐỘ ĐỘNG CƠ")
        ctrl_layout = QVBoxLayout(ctrl_group)
        ctrl_layout.setSpacing(14)

        # Thanh trượt và Spinbox
        slider_layout = QHBoxLayout()
        slider_layout.setSpacing(12)

        lbl_rev = QLabel("⏪ -100%")
        lbl_rev.setStyleSheet("color: #94a3b8; font-weight: bold;")
        lbl_fwd = QLabel("+100% ⏩")
        lbl_fwd.setStyleSheet("color: #94a3b8; font-weight: bold;")

        self.slider_speed = QSlider(Qt.Horizontal)
        self.slider_speed.setRange(-255, 255)
        self.slider_speed.setValue(0)
        self.slider_speed.setTickPosition(QSlider.TicksBelow)
        self.slider_speed.setTickInterval(51) # 20% mỗi nấc
        self.slider_speed.valueChanged.connect(self.on_slider_moved)

        self.spin_speed = QSpinBox()
        self.spin_speed.setRange(-255, 255)
        self.spin_speed.setValue(0)
        self.spin_speed.setFixedWidth(90)
        self.spin_speed.valueChanged.connect(self.on_spin_changed)

        slider_layout.addWidget(lbl_rev)
        slider_layout.addWidget(self.slider_speed, 1)
        slider_layout.addWidget(lbl_fwd)
        slider_layout.addWidget(self.spin_speed)

        ctrl_layout.addLayout(slider_layout)

        # Hiển thị % tốc độ hiện tại
        self.lbl_percent = QLabel("Tốc độ cài đặt: 0 / 255 (0.0% - ĐANG DỪNG)")
        self.lbl_percent.setAlignment(Qt.AlignCenter)
        self.lbl_percent.setStyleSheet("color: #38bdf8; font-size: 13px; font-weight: bold;")
        ctrl_layout.addWidget(self.lbl_percent)

        # Phím đặt trước tốc độ (PRESET BUTTONS)
        preset_title = QLabel("Phím đặt tốc độ nhanh:")
        preset_title.setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: bold;")
        ctrl_layout.addWidget(preset_title)

        # Hàng nút quay thuận
        fwd_box = QHBoxLayout()
        fwd_box.setSpacing(6)
        fwd_box.addWidget(QLabel("Thuận: "))
        for pct, val in [("25%", 64), ("50%", 128), ("75%", 192), ("100%", 255)]:
            btn = QPushButton(pct)
            btn.setProperty("class", "preset-btn")
            btn.clicked.connect(lambda _, v=val: self.set_speed_value(v))
            fwd_box.addWidget(btn)
        ctrl_layout.addLayout(fwd_box)

        # Hàng nút quay nghịch
        rev_box = QHBoxLayout()
        rev_box.setSpacing(6)
        rev_box.addWidget(QLabel("Nghịch: "))
        for pct, val in [("25%", -64), ("50%", -128), ("75%", -192), ("100%", -255)]:
            btn = QPushButton(pct)
            btn.setProperty("class", "preset-btn")
            btn.clicked.connect(lambda _, v=val: self.set_speed_value(v))
            rev_box.addWidget(btn)
        ctrl_layout.addLayout(rev_box)

        # Nút tinh chỉnh bước nhỏ (+-5, +-20)
        step_box = QHBoxLayout()
        step_box.setSpacing(6)
        step_box.addWidget(QLabel("Chỉnh nấc: "))
        for label, delta in [("-20", -20), ("-5", -5), ("+5", 5), ("+20", 20)]:
            btn = QPushButton(label)
            btn.setProperty("class", "preset-btn")
            btn.clicked.connect(lambda _, d=delta: self.step_speed_value(d))
            step_box.addWidget(btn)
        ctrl_layout.addLayout(step_box)

        # Nút Thao tác chính: DỪNG KHẨN CẤP & RESET ENCODER
        btn_action_layout = QHBoxLayout()
        btn_action_layout.setSpacing(10)

        self.btn_estop = QPushButton("🛑 DỪNG KHẨN CẤP (STOP)")
        self.btn_estop.setObjectName("btn_estop")
        self.btn_estop.clicked.connect(self.emergency_stop)

        self.btn_reset_enc = QPushButton("🔄 RESET ENCODER")
        self.btn_reset_enc.setObjectName("btn_reset_enc")
        self.btn_reset_enc.clicked.connect(self.reset_encoder)

        btn_action_layout.addWidget(self.btn_estop, 2)
        btn_action_layout.addWidget(self.btn_reset_enc, 1)

        ctrl_layout.addLayout(btn_action_layout)

        # Cài đặt chu kỳ gửi telemetry
        rate_layout = QHBoxLayout()
        rate_layout.addWidget(QLabel("Tần số gửi Telemetry:"))
        self.combo_rate = QComboBox()
        self.combo_rate.addItems(["50 ms (20 Hz)", "100 ms (10 Hz)", "200 ms (5 Hz)", "500 ms (2 Hz)"])
        self.combo_rate.setCurrentIndex(1)
        self.combo_rate.currentIndexChanged.connect(self.on_rate_changed)
        rate_layout.addWidget(self.combo_rate)
        rate_layout.addStretch()
        ctrl_layout.addLayout(rate_layout)

        layout.addWidget(ctrl_group)
        layout.addStretch()

        return panel

    def create_right_panel(self):
        panel = QWidget()
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(10)

        # 1. BIỂU ĐỒ THỜI GIAN THỰC
        graph_group = QGroupBox("BIỂU ĐỒ GIÁM SÁT VẬN TỐC THỜI GIAN THỰC")
        g_layout = QVBoxLayout(graph_group)
        g_layout.setContentsMargins(10, 12, 10, 10)
        g_layout.setSpacing(8)

        # Thanh công cụ đồ thị
        toolbar = QHBoxLayout()
        toolbar.setSpacing(8)

        self.btn_pause = QPushButton("⏸ Tạm dừng đồ thị")
        self.btn_pause.clicked.connect(self.toggle_graph_pause)
        toolbar.addWidget(self.btn_pause)

        self.btn_clear = QPushButton("🗑 Xóa dữ liệu")
        self.btn_clear.clicked.connect(self.clear_graph_data)
        toolbar.addWidget(self.btn_clear)

        toolbar.addWidget(QLabel("Thời gian xem:"))
        self.combo_window = QComboBox()
        self.combo_window.addItems(["15 giây", "30 giây", "60 giây", "120 giây"])
        self.combo_window.setCurrentIndex(1)
        self.combo_window.currentIndexChanged.connect(self.on_window_changed)
        toolbar.addWidget(self.combo_window)

        toolbar.addStretch()

        self.btn_export = QPushButton("📁 Xuất dữ liệu (CSV)")
        self.btn_export.setStyleSheet("background-color: #0d9488; border-color: #2dd4bf; color: white;")
        self.btn_export.clicked.connect(self.export_csv)
        toolbar.addWidget(self.btn_export)

        g_layout.addLayout(toolbar)

        # Khung đồ thị pyqtgraph
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.showGrid(x=True, y=True, alpha=0.25)
        self.plot_widget.setLabel('left', 'Vận tốc quay (RPM)', color='#38bdf8', size='12pt')
        self.plot_widget.setLabel('bottom', 'Thời gian (giây)', color='#94a3b8', size='12pt')
        self.plot_widget.addLegend(offset=(10, 10))

        # Đường cong RPM thực tế (Cyan sáng)
        pen_rpm = pg.mkPen(color='#00f0ff', width=2.5)
        self.curve_rpm = self.plot_widget.plot(name='RPM thực tế (vòng/phút)', pen=pen_rpm)

        # Đường cong PWM / Target (Đường đứt nét màu vàng)
        pen_pwm = pg.mkPen(color='#facc15', width=1.5, style=Qt.DashLine)
        self.curve_pwm = self.plot_widget.plot(name='Lệnh PWM (-255 -> 255)', pen=pen_pwm)

        # Đường cong Vận tốc dài cm/s (Xanh lá)
        pen_speed = pg.mkPen(color='#10b981', width=2.0)
        self.curve_speed = self.plot_widget.plot(name='Vận tốc dài (cm/s)', pen=pen_speed)

        g_layout.addWidget(self.plot_widget, 1)
        layout.addWidget(graph_group, 3)

        # 2. KHUNG LOG TERMINAL DỮ LIỆU JSON
        log_group = QGroupBox("TERMINAL TRUYỀN NHẬN SERIAL / JSON RAW DATA")
        log_layout = QVBoxLayout(log_group)
        log_layout.setContentsMargins(10, 10, 10, 10)
        log_layout.setSpacing(6)

        self.txt_log = QTextEdit()
        self.txt_log.setReadOnly(True)
        self.txt_log.setMaximumHeight(150)
        log_layout.addWidget(self.txt_log)

        log_btn_layout = QHBoxLayout()
        self.chk_autoscroll = QCheckBox("Tự động cuộn")
        self.chk_autoscroll.setChecked(True)
        log_btn_layout.addWidget(self.chk_autoscroll)

        log_btn_layout.addStretch()

        btn_clear_log = QPushButton("Xóa Log")
        btn_clear_log.setFixedWidth(80)
        btn_clear_log.clicked.connect(self.txt_log.clear)
        log_btn_layout.addWidget(btn_clear_log)

        log_layout.addLayout(log_btn_layout)

        layout.addWidget(log_group, 1)

        return panel

    def refresh_ports(self):
        """Quét và cập nhật danh sách cổng Serial"""
        self.combo_ports.clear()
        ports = SerialWorker.get_available_ports()
        if ports:
            for dev, desc in ports:
                self.combo_ports.addItem(f"{dev}", dev)
        else:
            self.combo_ports.addItem("Không tìm thấy cổng COM", "")

    def toggle_connection(self):
        if not self.worker.is_connected:
            simulated = self.chk_simulate.isChecked()
            port = self.combo_ports.currentData() or self.combo_ports.currentText()
            baud = int(self.combo_baud.currentText())

            if not simulated and (not port or "Không" in port):
                QMessageBox.warning(self, "Chưa chọn cổng", "Vui lòng cắm ESP32 và chọn cổng Serial, hoặc tích chọn 'Chế độ mô phỏng' để test!")
                return

            self.worker.connect_serial(port, baud, simulated=simulated)
        else:
            self.worker.disconnect_serial()

    def on_connection_changed(self, connected, info):
        if connected:
            self.btn_connect.setText("NGẮT KẾT NỐI")
            self.btn_connect.setObjectName("btn_disconnect")
            self.lbl_status_led.setText(f"● ĐÃ KẾT NỐI: {info}")
            self.lbl_status_led.setStyleSheet("color: #22c55e; font-weight: bold; font-size: 13px;")
            self.combo_ports.setEnabled(False)
            self.combo_baud.setEnabled(False)
            self.chk_simulate.setEnabled(False)
        else:
            self.btn_connect.setText("KẾT NỐI")
            self.btn_connect.setObjectName("btn_connect")
            self.lbl_status_led.setText("● CHƯA KẾT NỐI")
            self.lbl_status_led.setStyleSheet("color: #ef4444; font-weight: bold; font-size: 13px;")
            self.combo_ports.setEnabled(True)
            self.combo_baud.setEnabled(True)
            self.chk_simulate.setEnabled(True)

            # Reset các thẻ giá trị về 0
            self.card_rpm.set_value("0.0", "Đã ngắt")
            self.card_speed.set_value("0.000", "0.0 cm/s")
            self.card_pwm.set_value("0", "0.0%")
            self.card_pulses.set_value("0", "Δ 0")

        self.btn_connect.style().unpolish(self.btn_connect)
        self.btn_connect.style().polish(self.btn_connect)

    def on_slider_moved(self, value):
        self.spin_speed.blockSignals(True)
        self.spin_speed.setValue(value)
        self.spin_speed.blockSignals(False)
        self.update_percent_label(value)

        # Kích hoạt debounce timer gửi lệnh (tránh spam serial quá nhiều)
        self.slider_send_timer.start(40)

    def on_spin_changed(self, value):
        self.slider_speed.blockSignals(True)
        self.slider_speed.setValue(value)
        self.slider_speed.blockSignals(False)
        self.update_percent_label(value)
        self.send_speed(value)

    def set_speed_value(self, val):
        self.spin_speed.setValue(val)

    def step_speed_value(self, delta):
        cur = self.spin_speed.value()
        new_val = max(-255, min(255, cur + delta))
        self.spin_speed.setValue(new_val)

    def send_slider_speed(self):
        val = self.slider_speed.value()
        self.send_speed(val)

    def send_speed(self, val):
        self.worker.send_command({"cmd": "set_speed", "pwm": val})

    def emergency_stop(self):
        self.spin_speed.setValue(0)
        self.worker.send_command({"cmd": "stop"})
        self.log_terminal("[ACTION] >> DỪNG KHẨN CẤP ĐỘNG CƠ")

    def reset_encoder(self):
        self.worker.send_command({"cmd": "reset_encoder"})
        self.log_terminal("[ACTION] >> ĐẶT LẠI XUNG ENCODER VỀ 0")

    def update_percent_label(self, val):
        pct = (abs(val) / 255.0) * 100.0
        if val > 0:
            status = f"QUAY THUẬN ({val} / 255 | +{pct:.1f}%)"
            color = "#00f0ff"
        elif val < 0:
            status = f"QUAY NGHỊCH ({val} / 255 | -{pct:.1f}%)"
            color = "#f43f5e"
        else:
            status = "ĐANG DỪNG (0 / 255 | 0%)"
            color = "#94a3b8"

        self.lbl_percent.setText(f"Tốc độ cài đặt: {status}")
        self.lbl_percent.setStyleSheet(f"color: {color}; font-size: 13px; font-weight: bold;")

    def on_rate_changed(self, idx):
        rates = [50, 100, 200, 500]
        interval = rates[idx]
        self.worker.send_command({"cmd": "set_rate", "interval": interval})

    def on_telemetry_received(self, data):
        """Xử lý gói tin JSON telemetry nhận được từ ESP32"""
        rpm = float(data.get("rpm", 0.0))
        speed_mps = float(data.get("speed_mps", 0.0))
        speed_cms = float(data.get("speed_cms", speed_mps * 100.0))
        pulses = int(data.get("pulses", 0))
        delta = int(data.get("delta", 0))
        pwm = int(data.get("pwm", 0))
        dir_val = int(data.get("dir", 0))

        # Cập nhật Thẻ hiển thị
        dir_str = "QUAY THUẬN (CW)" if dir_val > 0 else ("QUAY NGHỊCH (CCW)" if dir_val < 0 else "DỪNG")
        self.card_rpm.set_value(f"{abs(rpm):.1f}", dir_str)
        self.card_speed.set_value(f"{abs(speed_mps):.3f}", f"{abs(speed_cms):.1f} cm/s")
        self.card_pulses.set_value(f"{pulses:,}", f"Δ {delta:+d} xung")
        pwm_pct = (abs(pwm) / 255.0) * 100.0
        self.card_pwm.set_value(f"{pwm}", f"{pwm_pct:.1f}% công suất")

        # Cập nhật dữ liệu đồ thị
        if not self.graph_paused:
            rel_t = time.time() - self.start_time
            self.time_data.append(rel_t)
            self.rpm_data.append(rpm)
            self.speed_data.append(speed_cms)
            self.pwm_data.append(pwm)

            # Lưu vào danh sách toàn bộ phiên để xuất CSV
            self.all_history.append({
                "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3],
                "time_s": round(rel_t, 3),
                "pwm": pwm,
                "rpm": rpm,
                "speed_mps": speed_mps,
                "speed_cms": speed_cms,
                "pulses": pulses,
                "delta": delta
            })

            # Vẽ lên plot
            self.curve_rpm.setData(list(self.time_data), list(self.rpm_data))
            self.curve_pwm.setData(list(self.time_data), list(self.pwm_data))
            self.curve_speed.setData(list(self.time_data), list(self.speed_data))

    def on_ack_received(self, data):
        cmd = data.get("cmd", "")
        status = data.get("status", "")
        pwm = data.get("pwm", 0)
        self.log_terminal(f"[ACK] Xác nhận: Lệnh '{cmd}' -> {status} (PWM: {pwm})")

    def toggle_graph_pause(self):
        self.graph_paused = not self.graph_paused
        if self.graph_paused:
            self.btn_pause.setText("▶ Tiếp tục đồ thị")
            self.btn_pause.setStyleSheet("background-color: #0284c7; color: white;")
        else:
            self.btn_pause.setText("⏸ Tạm dừng đồ thị")
            self.btn_pause.setStyleSheet("")

    def clear_graph_data(self):
        self.time_data.clear()
        self.rpm_data.clear()
        self.speed_data.clear()
        self.pwm_data.clear()
        self.curve_rpm.clear()
        self.curve_pwm.clear()
        self.curve_speed.clear()
        self.start_time = time.time()
        self.log_terminal("[INFO] Đã xóa dữ liệu hiển thị trên đồ thị")

    def on_window_changed(self, idx):
        windows = [150, 300, 600, 1200]  # Số điểm tương ứng 15s, 30s, 60s, 120s ở 10Hz
        self.max_points = windows[idx]
        self.time_data = deque(self.time_data, maxlen=self.max_points)
        self.rpm_data = deque(self.rpm_data, maxlen=self.max_points)
        self.speed_data = deque(self.speed_data, maxlen=self.max_points)
        self.pwm_data = deque(self.pwm_data, maxlen=self.max_points)

    def export_csv(self):
        if not self.all_history:
            QMessageBox.information(self, "Thông báo", "Chưa có dữ liệu đo lường nào để xuất CSV!")
            return

        default_name = f"motor_telemetry_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        path, _ = QFileDialog.getSaveFileName(self, "Lưu file dữ liệu CSV", default_name, "CSV Files (*.csv)")
        if not path:
            return

        try:
            with open(path, mode="w", newline="", encoding="utf-8") as f:
                fieldnames = ["timestamp", "time_s", "pwm", "rpm", "speed_mps", "speed_cms", "pulses", "delta"]
                writer = csv.DictWriter(f, fieldnames=fieldnames)
                writer.writeheader()
                writer.writerows(self.all_history)

            QMessageBox.information(self, "Thành công", f"Đã xuất thành công {len(self.all_history)} dòng dữ liệu ra file:\n{path}")
            self.log_terminal(f"[EXPORT] Đã xuất file CSV: {path}")
        except Exception as e:
            QMessageBox.critical(self, "Lỗi", f"Không thể lưu file CSV: {e}")

    def log_terminal(self, text):
        self.txt_log.append(text)
        if self.chk_autoscroll.isChecked():
            cursor = self.txt_log.textCursor()
            cursor.movePosition(cursor.End)
            self.txt_log.setTextCursor(cursor)

    def on_error(self, err_msg):
        self.log_terminal(f"<font color='#f43f5e'>[ERROR] {err_msg}</font>")

    def closeEvent(self, event):
        self.worker.stop_worker()
        event.accept()


def main():
    # Kích hoạt chia tỉ lệ DPI cao
    QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, True)
    QApplication.setAttribute(Qt.AA_UseHighDpiPixmaps, True)

    app = QApplication(sys.argv)
    app.setApplicationName("ESP32 Motor Controller & Telemetry")

    window = MotorDashboard()
    window.show()

    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
