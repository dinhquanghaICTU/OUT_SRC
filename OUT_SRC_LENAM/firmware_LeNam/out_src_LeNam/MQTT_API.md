# MQTT API - Thiết bị Lê Nam

Tài liệu này định nghĩa giao tiếp MQTT giữa firmware thiết bị Lê Nam và
server. Mọi thay đổi firmware và server phải tuân theo contract này.

## 1. Thông tin thiết bị

| Thuộc tính | Giá trị |
|---|---|
| Device ID | `190782` |
| Loại dữ liệu | Nhiệt độ LM35 và âm thanh MAX9814 |
| Actuator | Relay |
| Chu kỳ mặc định | 2 giây |
| MQTT broker hiện tại | `192.168.12.1:1883` |
| MQTT namespace | `iot/v1/devices/190782` |

ID `190782` hiện bị trùng với firmware Hoàng Anh. Chỉ một thiết bị được giữ ID
này; thiết bị còn lại bắt buộc phải được cấp ID khác trước khi chạy chung.

## 2. Danh sách topic

| Topic | Thiết bị | QoS | Retain | Mục đích |
|---|---|---:|---:|---|
| `iot/v1/devices/190782/telemetry` | Publish | 1 | Không | Gửi nhiệt độ và âm thanh |
| `iot/v1/devices/190782/config/desired` | Subscribe | 1 | Có | Nhận chu kỳ lấy mẫu và ngưỡng |
| `iot/v1/devices/190782/config/reported` | Publish | 1 | Có | Xác nhận cấu hình thực tế |
| `iot/v1/devices/190782/commands` | Subscribe | 1 | Không | Nhận lệnh điều khiển relay |
| `iot/v1/devices/190782/command-result` | Publish | 1 | Không | Trả kết quả thực hiện lệnh |
| `iot/v1/devices/190782/state` | Publish | 1 | Có | Trạng thái relay thực tế |
| `iot/v1/devices/190782/status` | Publish/LWT | 1 | Có | Báo online/offline |

## 3. Telemetry

Topic:

```text
iot/v1/devices/190782/telemetry
```

Payload:

```json
{
  "schema_version": 1,
  "device_id": "190782",
  "message_id": "190782-8821",
  "sequence": 8821,
  "uptime_ms": 502130,
  "firmware_version": "1.0.0",
  "metrics": {
    "temperature_c": 32.09,
    "sound_vpp": 0.135
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
| `metrics.temperature_c` | number | Có | Nhiệt độ LM35, đơn vị °C |
| `metrics.sound_vpp` | number | Có | Điện áp peak-to-peak, đơn vị V |

## 4. Cấu hình từ server

Server publish retained tới:

```text
iot/v1/devices/190782/config/desired
```

```json
{
  "config_version": 4,
  "sampling_interval_ms": 5000,
  "thresholds": {
    "temperature_c": {
      "warning_above": 40.0,
      "critical_above": 50.0
    },
    "sound_vpp": {
      "warning_above": 1.5
    }
  }
}
```

Firmware phải validate, lưu NVS/Preferences rồi publish retained tới
`config/reported`.

```json
{
  "config_version": 4,
  "status": "applied",
  "sampling_interval_ms": 5000,
  "thresholds": {
    "temperature_c": {
      "warning_above": 40.0,
      "critical_above": 50.0
    },
    "sound_vpp": {
      "warning_above": 1.5
    }
  }
}
```

Nếu không hợp lệ:

```json
{
  "config_version": 4,
  "status": "rejected",
  "error": "invalid_sampling_interval"
}
```

## 5. Điều khiển relay

Server publish tới:

```text
iot/v1/devices/190782/commands
```

Bật relay:

```json
{
  "command_id": "cmd-1058",
  "type": "relay.set",
  "params": {
    "state": true
  }
}
```

Tắt relay dùng cùng payload với `state` bằng `false`.

Firmware phải kiểm tra `command_id`, `type` và kiểu boolean của `state`. Sau khi
điều khiển phần cứng, publish tới `command-result`:

```json
{
  "command_id": "cmd-1058",
  "status": "succeeded",
  "state": {
    "relay": true
  }
}
```

Nếu lệnh không hợp lệ:

```json
{
  "command_id": "cmd-1058",
  "status": "rejected",
  "error": "unsupported_command"
}
```

## 6. Trạng thái thiết bị

Mỗi lần relay thay đổi do MQTT hoặc nút vật lý, publish retained tới `state`:

```json
{
  "relay": true,
  "changed_by": "command",
  "uptime_ms": 502130
}
```

`changed_by` nhận một trong `command`, `button`, `startup`.

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

Topic cũ `devices/{id}/relay/set` và `devices/{id}/relay/state` sẽ được thay bởi
`commands`, `command-result` và `state` sau khi cả server lẫn firmware đã nâng
cấp. Không đổi riêng một phía trong lúc hệ thống đang chạy.

## 8. Kiểm thử bằng Mosquitto

```bash
mosquitto_sub -h 192.168.12.1 -p 1883 -t 'iot/v1/devices/190782/#' -v
```

Bật relay:

```bash
mosquitto_pub -h 192.168.12.1 -p 1883 -q 1 \
  -t 'iot/v1/devices/190782/commands' \
  -m '{"command_id":"cmd-test-1","type":"relay.set","params":{"state":true}}'
```

Gửi cấu hình:

```bash
mosquitto_pub -h 192.168.12.1 -p 1883 -q 1 -r \
  -t 'iot/v1/devices/190782/config/desired' \
  -m '{"config_version":4,"sampling_interval_ms":5000,"thresholds":{"temperature_c":{"warning_above":40.0,"critical_above":50.0},"sound_vpp":{"warning_above":1.5}}}'
```
