# ỨNG DỤNG GIÁM SÁT VÀ ĐIỀU KHIỂN TỐC ĐỘ ĐỘNG CƠ QUA SERIAL (ESP32)

Ứng dụng viết bằng **Python 3 (PyQt5 + pyqtgraph)** để điều khiển chiều quay và tốc độ động cơ DC có encoder (JGA25-370 + L298N + ESP32), đồng thời giám sát vận tốc thời gian thực qua giao thức **JSON Serial**.

---

## 🌟 Các Tính Năng Nổi Bật

1. **Điều khiển tốc độ động cơ**:
   - Thanh trượt mượt mà từ `-255` (Quay nghịch tối đa) đến `0` (Dừng) đến `+255` (Quay thuận tối đa).
   - Ô nhập số SpinBox để đặt chính xác giá trị PWM mong muốn.
   - Phím bấm nhanh tỉ lệ: `25%`, `50%`, `75%`, `100%` cho cả 2 chiều quay.
   - Tinh chỉnh bước nhỏ: `+5`, `-5`, `+20`, `-20`.
   - **Nút Dừng Khẩn Cấp (🛑 EMERGENCY STOP)** to rõ, ngắt động cơ ngay lập tức.
   - **Nút Reset Encoder**: Đặt lại số xung đếm được về 0.

2. **Giám sát thông số thời gian thực (Live Telemetry Cards)**:
   - 🚀 **Vận tốc quay (RPM)**: Hiển thị số lớn rõ nét kèm trạng thái quay thuận / nghịch / dừng.
   - ⚡ **Vận tốc dài**: Tính toán theo đường kính bánh xe (`m/s` và `cm/s`).
   - 🔢 **Xung Encoder**: Tổng số xung đếm được và độ biến thiên `Δ` xung/chu kỳ.
   - 🎚 **Công suất PWM**: Giá trị PWM thực tế và phần trăm công suất (%).

3. **Biểu đồ thống kê thời gian thực (Real-time Graph)**:
   - Sử dụng thư viện đồ họa phần cứng `pyqtgraph` cho tốc độ khung hình 60 FPS mượt mà.
   - Đường cong hiển thị tốc độ **RPM thực tế** (Màu Cyan sáng).
   - Đường hiển thị **Mục tiêu PWM** (Nét đứt màu vàng neon).
   - Đường hiển thị **Vận tốc dài cm/s** (Màu xanh lá).
   - Tùy chọn thời gian xem: 15s, 30s, 60s, 120s.
   - Nút Tạm dừng / Tiếp tục vẽ và nút Xóa biểu đồ.
   - **📁 Xuất dữ liệu CSV (Export CSV)**: Lưu toàn bộ lịch sử đo (Timestamp, PWM, RPM, Speed, Pulses) ra file CSV để làm báo cáo đồ án / nghiên cứu.

4. **🧪 Chế độ mô phỏng (Simulation Mode)**:
   - Cho phép test toàn bộ giao diện, các phím bấm và đồ thị vận tốc quay ngay cả khi chưa cắm phần cứng ESP32.
   - Mô phỏng động học động cơ DC thực tế (quán tính tăng tốc, giảm tốc, tích lũy xung encoder).

5. **Terminal giám sát JSON**:
   - Hiển thị toàn bộ các bản tin JSON nhận được từ ESP32 và lệnh gửi đi.

---

## 🚀 Hướng Dẫn Cài Đặt & Khởi Chạy

### 1. Cài đặt thư viện phụ thuộc (nếu chưa có):
```bash
pip install -r requirements.txt
```

### 2. Khởi chạy ứng dụng:
```bash
python3 main.py
# Hoặc chạy file script:
./run.sh
```

---

## 📡 Định Dạng Giao Tiếp JSON Serial (115200 baud)

### 1. ESP32 gửi lên App (Telemetry định kỳ mỗi 100ms):
```json
{
  "type": "telemetry",
  "rpm": 142.5,
  "speed_mps": 0.485,
  "speed_cms": 48.5,
  "pulses": 12450,
  "delta": 14,
  "pwm": 180,
  "dir": 1,
  "dt_ms": 100
}
```

### 2. App gửi xuống ESP32 (Lệnh điều khiển):
- **Cài đặt tốc độ:** `{"cmd": "set_speed", "pwm": 180}` (PWM từ `-255` đến `255`)
- **Dừng động cơ:** `{"cmd": "stop"}`
- **Reset xung:** `{"cmd": "reset_encoder"}`
- **Đổi tần số gửi:** `{"cmd": "set_rate", "interval": 100}` (ms)

### 3. ESP32 phản hồi xác nhận (ACK):
```json
{
  "type": "ack",
  "cmd": "set_speed",
  "status": "ok",
  "pwm": 180
}
```
