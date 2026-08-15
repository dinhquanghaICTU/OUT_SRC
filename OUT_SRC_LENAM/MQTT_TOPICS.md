# MQTT Topics

Tài liệu topic MQTT của firmware LeNam. Ví dụ bên dưới sử dụng thiết bị có
`PRODUCT_ID = 190782` và broker `192.168.12.1:1883`.

## Quy ước

```text
devices/{device_id}/sensor
devices/{device_id}/relay/set
devices/{device_id}/relay/state
devices/{device_id}/config
```

## Bảng topic

| Topic | ESP32 | Server | Nội dung | Retain |
|---|---|---|---|---|
| `devices/190782/sensor` | Publish | Subscribe | Nhiệt độ LM35 và biên độ âm thanh MAX9814 | Không |
| `devices/190782/relay/set` | Subscribe | Publish | Lệnh bật hoặc tắt relay | Không |
| `devices/190782/relay/state` | Publish | Subscribe | Trạng thái relay thực tế | Có |
| `devices/190782/config` | Subscribe | Publish | Cấu hình và ngưỡng của thiết bị | Nên có |

## Payload

### Dữ liệu cảm biến

Topic:

```text
devices/190782/sensor
```

Payload:

```json
{
  "device_id": "190782",
  "temperature_c": 32.09,
  "sound_vpp": 0.135
}
```

### Điều khiển relay

Topic:

```text
devices/190782/relay/set
```

Bật relay:

```json
{"state":true}
```

Tắt relay:

```json
{"state":false}
```

### Trạng thái relay

Topic:

```text
devices/190782/relay/state
```

Payload:

```json
{
  "device_id": "190782",
  "relay_state": true
}
```

## Lệnh test Ubuntu

Theo dõi toàn bộ topic của thiết bị:

```bash
mosquitto_sub -h localhost -p 1883 -t "devices/190782/#" -v
```

Chỉ đọc cảm biến:

```bash
mosquitto_sub -h localhost -p 1883 -t "devices/190782/sensor" -v
```

Chỉ đọc trạng thái relay:

```bash
mosquitto_sub -h localhost -p 1883 -t "devices/190782/relay/state" -v
```

Bật relay:

```bash
mosquitto_pub -h localhost -p 1883 \
  -t "devices/190782/relay/set" \
  -m '{"state":true}'
```

Tắt relay:

```bash
mosquitto_pub -h localhost -p 1883 \
  -t "devices/190782/relay/set" \
  -m '{"state":false}'
```

Server có thể nhận dữ liệu của tất cả thiết bị bằng wildcard:

```bash
mosquitto_sub -h localhost -p 1883 -t "devices/+/sensor" -v
```

```bash
mosquitto_sub -h localhost -p 1883 -t "devices/+/relay/state" -v
```
