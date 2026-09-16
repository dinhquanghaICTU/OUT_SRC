# MQTT API - Thiết bị Hoàng Anh

Tài liệu này định nghĩa giao tiếp MQTT dự kiến giữa firmware thiết bị Hoàng
Anh và server. Firmware sử dụng BMP180 để đo nhiệt độ và áp suất.

## 1. Việc bắt buộc chốt

1. `PRODUCT_ID` là `HA-190782`, tách biệt với thiết bị Lê Nam.
2. Topic telemetry được tạo theo `PRODUCT_ID` trong `config.h`.
3. BMP180 dùng địa chỉ `0x77`, SDA GPIO21 và SCL GPIO22.
4. Telemetry gửi `temperature_c` và `pressure_hpa` mỗi `SAMPLE_INTERVAL_MS`.

Tài liệu dưới đây dùng biến `{device_id}` cho tới khi có ID chính thức. Không
được chạy đồng thời với thiết bị Lê Nam bằng ID `190782`.

## 2. Danh sách topic

| Topic | Thiết bị | QoS | Retain | Mục đích |
|---|---|---:|---:|---|
| `iot/v1/devices/{device_id}/telemetry` | Publish | 1 | Không | Gửi dữ liệu cảm biến |
| `iot/v1/devices/{device_id}/config/desired` | Subscribe | 1 | Có | Nhận chu kỳ lấy mẫu và ngưỡng |
| `iot/v1/devices/{device_id}/config/reported` | Publish | 1 | Có | Xác nhận cấu hình thực tế |
| `iot/v1/devices/{device_id}/commands` | Subscribe | 1 | Không | Nhận lệnh tức thời |
| `iot/v1/devices/{device_id}/command-result` | Publish | 1 | Không | Trả kết quả thực hiện lệnh |
| `iot/v1/devices/{device_id}/state` | Publish | 1 | Có | Trạng thái actuator thực tế |
| `iot/v1/devices/{device_id}/status` | Publish/LWT | 1 | Có | Báo online/offline |

## 3. Telemetry

Nếu phần cứng chính thức là BMP180, contract đề xuất là:

```json
{
  "schema_version": 1,
  "device_id": "<device_id>",
  "message_id": "<device_id>-1025",
  "sequence": 1025,
  "uptime_ms": 502130,
  "firmware_version": "1.0.0",
  "metrics": {
    "temperature_c": 31.25,
    "pressure_hpa": 1009.42
  }
}
```

| Trường | Kiểu | Bắt buộc | Mô tả |
|---|---|---:|---|
| `schema_version` | integer | Có | Phiên bản schema, hiện là `1` |
| `device_id` | string | Có | Phải khớp ID trong topic |
| `message_id` | string | Có | ID duy nhất để chống ghi trùng |
| `sequence` | integer | Có | Bộ đếm tăng sau mỗi lần gửi |
| `uptime_ms` | integer | Có | Thời gian chạy từ lúc khởi động |
| `firmware_version` | string | Có | Phiên bản firmware |
| `metrics.temperature_c` | number | Có | Nhiệt độ BMP180, đơn vị °C |
| `metrics.pressure_hpa` | number | Có | Áp suất BMP180, đơn vị hPa |

Nếu thiết bị thực tế đo UV hoặc sensor khác, phải sửa bảng và payload này trước
khi code; không giữ tên field copy từ firmware khác.

## 4. Cấu hình từ server

Server publish retained tới:

```text
iot/v1/devices/{device_id}/config/desired
```

Contract dành cho BMP180:

```json
{
  "config_version": 3,
  "sampling_interval_ms": 5000,
  "thresholds": {
    "temperature_c": {
      "min": 0.0,
      "max": 50.0
    },
    "pressure_hpa": {
      "min": 990.0,
      "max": 1030.0
    }
  }
}
```

Firmware phải validate, lưu NVS/Preferences và publish retained tới
`config/reported`:

```json
{
  "config_version": 3,
  "status": "applied",
  "sampling_interval_ms": 5000,
  "thresholds": {
    "temperature_c": {
      "min": 0.0,
      "max": 50.0
    },
    "pressure_hpa": {
      "min": 990.0,
      "max": 1030.0
    }
  }
}
```

Nếu không hợp lệ:

```json
{
  "config_version": 3,
  "status": "rejected",
  "error": "invalid_pressure_range"
}
```

## 5. Command và state

Chỉ triển khai khi đã xác định actuator thực tế. Khuôn command chung:

```json
{
  "command_id": "cmd-2001",
  "type": "actuator.set",
  "params": {
    "state": true
  }
}
```

Sau khi điều khiển phần cứng, thiết bị trả `command-result`:

```json
{
  "command_id": "cmd-2001",
  "status": "succeeded",
  "state": {
    "actuator": true
  }
}
```

Trước khi chốt actuator, không để server gửi command thật cho thiết bị này.

## 6. Online/offline

Khi online, publish retained tới `status`:

```json
{
  "online": true,
  "firmware_version": "1.0.0"
}
```

Last Will retained:

```json
{
  "online": false
}
```

## 7. Macro firmware đề xuất

Sau khi cấp `PRODUCT_ID` duy nhất:

```cpp
#define MQTT_TOPIC_PREFIX "iot/v1/devices/" PRODUCT_ID
#define MQTT_TELEMETRY_TOPIC MQTT_TOPIC_PREFIX "/telemetry"
#define MQTT_CONFIG_DESIRED_TOPIC MQTT_TOPIC_PREFIX "/config/desired"
#define MQTT_CONFIG_REPORTED_TOPIC MQTT_TOPIC_PREFIX "/config/reported"
#define MQTT_COMMAND_TOPIC MQTT_TOPIC_PREFIX "/commands"
#define MQTT_COMMAND_RESULT_TOPIC MQTT_TOPIC_PREFIX "/command-result"
#define MQTT_STATE_TOPIC MQTT_TOPIC_PREFIX "/state"
#define MQTT_STATUS_TOPIC MQTT_TOPIC_PREFIX "/status"
```

## 8. Kiểm thử bằng Mosquitto

Thay `<device_id>` bằng ID chính thức:

```bash
mosquitto_sub -h 192.168.12.1 -p 1883 \
  -t 'iot/v1/devices/<device_id>/#' -v
```

Gửi cấu hình BMP180 mẫu:

```bash
mosquitto_pub -h 192.168.12.1 -p 1883 -q 1 -r \
  -t 'iot/v1/devices/<device_id>/config/desired' \
  -m '{"config_version":3,"sampling_interval_ms":5000,"thresholds":{"temperature_c":{"min":0.0,"max":50.0},"pressure_hpa":{"min":990.0,"max":1030.0}}}'
```
