# ICTU Environment Server

Qt 6 console service chạy trên Raspberry Pi, cung cấp HTTP API và lưu dữ liệu
cảm biến vào SQLite.

## Kiến trúc

```text
ESP32 -> MQTT -> Discovery service -> SQLite
                                  |
UI Qt trên Raspberry Pi -> HTTP API local -> Auth/validation -> Database
```

- `api/ApiServer.*`: route, JSON validation, Bearer token và HTTP status.
- `database/Database.*`: migration, prepared query, transaction và repository.
- `main.cpp`: cấu hình command line và vòng lặp Qt.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

Qt cần các module: `Core`, `Network`, `Sql`, `HttpServer`.

## Run

```bash
./build/OUT_SRC_TRUNGKIEN-SERVER \
  --port 8080 \
  --database /var/lib/ictu-environment/environment.db
```

Nếu bỏ `--database`, ứng dụng dùng thư mục dữ liệu chuẩn của Qt. Tài khoản
khởi tạo cho giai đoạn phát triển là `admin / 1`; phải đổi mật khẩu trước khi
triển khai ngoài mạng lab.

## API

| Method | Endpoint | Mục đích |
|---|---|---|
| GET | `/api/health` | Health check, không cần token |
| POST | `/api/auth/login` | Đăng nhập và lấy Bearer token |
| GET | `/api/devices/me` | Đọc thiết bị của tài khoản hiện tại |
| GET | `/api/devices/available` | Thiết bị online, chưa có owner |
| POST | `/api/devices/claim` | Liên kết một device ID với tài khoản hiện tại |
| POST | `/api/devices/release` | Gỡ thiết bị khỏi tài khoản hiện tại |
| POST | `/api/devices/relay` | Điều khiển relay của thiết bị thuộc user |
| PUT | `/api/devices/config` | Lưu và gửi ngưỡng riêng cho một thiết bị |
| GET | `/api/devices/history` | Log telemetry và trung bình theo device/ngày/tháng/năm |
| GET | `/api/admin/users` | Admin đọc danh sách user và device sở hữu |
| POST | `/api/admin/users` | Admin tạo tài khoản mới |
| PUT | `/api/admin/users/<username>` | Admin sửa username, mật khẩu, quyền, trạng thái |
| DELETE | `/api/admin/users/<username>` | Admin xóa user và gỡ các thiết bị đang sở hữu |
| DELETE | `/api/admin/users/<username>/devices/<deviceId>` | Admin gỡ một thiết bị khỏi tài khoản bất kỳ |
| POST | `/api/readings` | Ghi mẫu cảm biến |
| GET | `/api/readings/latest` | Mẫu mới nhất |
| GET | `/api/pressure/history?limit=100` | Lịch sử áp suất |
| GET | `/api/distance/history?limit=100` | Lịch sử khoảng cách |
| GET | `/api/alerts?limit=100` | Lịch sử cảnh báo |
| GET | `/api/config` | Đọc cấu hình ngưỡng |
| PUT | `/api/config` | Cập nhật cấu hình ngưỡng |

Các endpoint ngoài health/login yêu cầu:

```text
Authorization: Bearer <TOKEN>
```

## User và device ownership

Mỗi tài khoản có thể sở hữu nhiều thiết bị, nhưng mỗi `device_id` chỉ được thuộc
một tài khoản. Tính duy nhất của `device_id` được thực thi bằng unique constraint
trong SQLite, không phụ thuộc vào kiểm tra phía giao diện.

Claim thiết bị:

```http
POST /api/devices/claim
Authorization: Bearer <TOKEN>
Content-Type: application/json
```

```json
{
  "device_id": "150304",
  "name": "Thiết bị Trung Kiên"
}
```

Nếu device đã thuộc user khác, server trả `409 device_claimed` và device đó bị
loại khỏi API discovery nên không xuất hiện trên giao diện add của user khác.

Server subscribe `iot/v1/devices/+/telemetry` và
`iot/v1/devices/+/status` cùng `iot/v1/devices/+/state`. Telemetry mới cũng được coi là tín hiệu online. API
discovery chỉ trả thiết bị có bản tin trong 15 giây gần nhất và chưa được claim.

Mỗi telemetry hợp lệ được lưu trong `device_telemetry_log`. Ví dụ truy vấn lịch
sử một thiết bị theo tháng:

```http
GET /api/devices/history?device_id=190782&period=month&date=2026-08-09&limit=500
Authorization: Bearer <TOKEN>
```

`period` nhận `day`, `month` hoặc `year`. API chỉ cho đọc thiết bị thuộc access
token hiện tại và trả cả danh sách mẫu cùng `averages` của toàn khoảng đã chọn.

Điều khiển relay:

```json
{
  "device_id": "190782",
  "state": true
}
```

Server chỉ publish command MQTT sau khi xác nhận access token đang sở hữu device.
Trạng thái trên app lấy từ topic retained `iot/v1/devices/{id}/state`, không
lấy theo trạng thái nút vừa bấm.

Cấu hình ngưỡng riêng cho thiết bị:

```json
{
  "device_id": "150304",
  "config": {
    "sampling_interval_ms": 5000,
    "thresholds": {
      "uv_index": { "warning_above": 6, "critical_above": 8 },
      "pressure_hpa": { "min": 990, "max": 1030 }
    }
  }
}
```

Server kiểm tra quyền sở hữu, lưu cấu hình vào SQLite rồi publish retained QoS 1
tới `iot/v1/devices/{id}/config/desired`. Vì là retained message, thiết bị vẫn
nhận được cấu hình mới nhất sau khi kết nối lại MQTT.

Mặc định server kết nối Mosquitto tại `127.0.0.1:1883`. Có thể thay đổi:

```bash
./build/OUT_SRC_TRUNGKIEN-SERVER \
  --mqtt-host 127.0.0.1 \
  --mqtt-port 1883
```

## Triển khai toàn bộ trên Raspberry Pi

Theo mô hình triển khai cuối, Raspberry Pi chạy cả Mosquitto, server và UI Qt.
UI gọi API local tại `http://127.0.0.1:8080`; ESP32 vẫn kết nối MQTT bằng IP LAN
của Pi. Xem hướng dẫn triển khai tại:

```text
deploy/pi/README_PI_RUNTIME.md
```

Admin tạo user:

```json
{
  "username": "user_one",
  "password": "password123",
  "role": "viewer"
}
```

Mật khẩu phải có ít nhất 8 ký tự. Role hợp lệ là `viewer` hoặc `admin`.

## Bước tiếp theo

Tạo một `SensorWorker` dùng `QTimer` đọc BMP180 và ADC của GP2Y0A21YK0F.
Worker gọi trực tiếp `Database::insertReading()`; không cần tự gọi HTTP API
của chính server. API `POST /api/readings` chủ yếu phục vụ test hoặc nguồn dữ
liệu bên ngoài.
