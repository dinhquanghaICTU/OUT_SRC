# MQTT API - Thiết bị Trung Kiên

Tài liệu này định nghĩa giao tiếp MQTT giữa firmware thiết bị Trung Kiên và
server. Mọi thay đổi firmware và server phải tuân theo contract này.

## 1. Thông tin thiết bị

| Thuộc tính | Giá trị |
|---|---|
| Device ID | `150304` |
| Loại dữ liệu | UV và áp suất |
| Chu kỳ mặc định | 2 giây |
| MQTT broker hiện tại | `192.168.12.1:1883` |
| MQTT namespace | `iot/v1/devices/150304` |

`device_id` phải duy nhất và không được thay bằng tên hiển thị của thiết bị.

## 2. Danh sách topic

| Topic | Thiết bị | QoS | Retain | Mục đích |
|---|---|---:|---:|---|
| `iot/v1/devices/150304/telemetry` | Publish | 1 | Không | Gửi dữ liệu UV và áp suất |
| `iot/v1/devices/150304/config/desired` | Subscribe | 1 | Có | Nhận chu kỳ lấy mẫu và ngưỡng |
| `iot/v1/devices/150304/config/reported` | Publish | 1 | Có | Xác nhận cấu hình thực tế |
| `iot/v1/devices/150304/commands` | Subscribe | 1 | Không | Nhận lệnh tức thời |
| `iot/v1/devices/150304/command-result` | Publish | 1 | Không | Trả kết quả thực hiện lệnh |
| `iot/v1/devices/150304/status` | Publish/LWT | 1 | Có | Báo online/offline |

Server nhận telemetry của mọi thiết bị bằng wildcard:

```text
iot/v1/devices/+/telemetry
```

## 3. Telemetry

Topic:

```text
iot/v1/devices/150304/telemetry
```

Payload:

```json
{
  "schema_version": 1,
  "device_id": "150304",
  "message_id": "150304-1025",
  "sequence": 1025,
  "uptime_ms": 502130,
  "firmware_version": "1.0.0",
  "metrics": {
    "uv_voltage": 1.234,
    "uv_index": 3.2,
    "pressure_hpa": 1010.5
  }
}
```

| Trường | Kiểu | Bắt buộc | Mô tả |
|---|---|---:|---|
| `schema_version` | integer | Có | Phiên bản schema, hiện là `1` |
| `device_id` | string | Có | Phải bằng `150304` và khớp ID trong topic |
| `message_id` | string | Có | ID duy nhất để server chống ghi trùng |
| `sequence` | integer | Có | Bộ đếm tăng sau mỗi lần gửi |
| `uptime_ms` | integer | Có | Thời gian chạy từ lúc khởi động |
| `firmware_version` | string | Có | Phiên bản firmware |
| `metrics.uv_voltage` | number | Có | Điện áp cảm biến UV, đơn vị V |
| `metrics.uv_index` | number | Có | Chỉ số UV |
| `metrics.pressure_hpa` | number | Có | Áp suất, đơn vị hPa |

Không retain telemetry. Nếu đọc sensor lỗi, không gửi giá trị giả; có thể bỏ
metric bị lỗi và thêm `errors`, hoặc bỏ cả bản tin.

## 4. Cấu hình từ server

Server publish retained tới:

```text
iot/v1/devices/150304/config/desired
```

```json
{
  "config_version": 8,
  "sampling_interval_ms": 5000,
  "thresholds": {
    "uv_index": {
      "warning_above": 6.0,
      "critical_above": 8.0
    },
    "pressure_hpa": {
      "min": 990.0,
      "max": 1030.0
    }
  }
}
```

Firmware phải:

1. Parse và kiểm tra toàn bộ JSON.
2. Chỉ nhận `config_version` mới hơn phiên bản đã áp dụng.
3. Kiểm tra `sampling_interval_ms` trong khoảng `1000..3600000`.
4. Kiểm tra ngưỡng min nhỏ hơn max.
5. Lưu cấu hình hợp lệ vào NVS/Preferences.
6. Publish kết quả lên `config/reported`.

Xác nhận thành công:

```json
{
  "config_version": 8,
  "status": "applied",
  "sampling_interval_ms": 5000,
  "thresholds": {
    "uv_index": {
      "warning_above": 6.0,
      "critical_above": 8.0
    },
    "pressure_hpa": {
      "min": 990.0,
      "max": 1030.0
    }
  }
}
```

Từ chối cấu hình:

```json
{
  "config_version": 8,
  "status": "rejected",
  "error": "pressure_min_must_be_less_than_max"
}
```

## 5. Command

Topic đã dành sẵn cho lệnh tức thời. Hiện firmware chưa có actuator được xác
định nên không bắt buộc triển khai command. Không dùng topic config cho các
lệnh tức thời.

Khuôn payload chung:

```json
{
  "command_id": "cmd-1058",
  "type": "device.restart",
  "params": {}
}
```

Kết quả:

```json
{
  "command_id": "cmd-1058",
  "status": "succeeded"
}
```

`status` chỉ nhận `succeeded`, `failed` hoặc `rejected`.

## 6. Online/offline

Khi kết nối thành công, thiết bị publish retained:

```json
{
  "online": true,
  "firmware_version": "1.0.0"
}
```

Khi tạo MQTT client, cấu hình Last Will retained trên cùng topic:

```json
{
  "online": false
}
```

## 7. Macro firmware đề xuất

```cpp
#define MQTT_TOPIC_PREFIX "iot/v1/devices/" PRODUCT_ID
#define MQTT_TELEMETRY_TOPIC MQTT_TOPIC_PREFIX "/telemetry"
#define MQTT_CONFIG_DESIRED_TOPIC MQTT_TOPIC_PREFIX "/config/desired"
#define MQTT_CONFIG_REPORTED_TOPIC MQTT_TOPIC_PREFIX "/config/reported"
#define MQTT_COMMAND_TOPIC MQTT_TOPIC_PREFIX "/commands"
#define MQTT_COMMAND_RESULT_TOPIC MQTT_TOPIC_PREFIX "/command-result"
#define MQTT_STATUS_TOPIC MQTT_TOPIC_PREFIX "/status"
```

## 8. Kiểm thử bằng Mosquitto

Theo dõi toàn bộ dữ liệu của thiết bị:

```bash
mosquitto_sub -h 192.168.12.1 -p 1883 -t 'iot/v1/devices/150304/#' -v
```

Gửi cấu hình:

```bash
mosquitto_pub -h 192.168.12.1 -p 1883 -q 1 -r \
  -t 'iot/v1/devices/150304/config/desired' \
  -m '{"config_version":8,"sampling_interval_ms":5000,"thresholds":{"uv_index":{"warning_above":6.0,"critical_above":8.0},"pressure_hpa":{"min":990.0,"max":1030.0}}}'
```
