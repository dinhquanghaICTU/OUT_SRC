# Chạy toàn bộ hệ thống trên Raspberry Pi

Mô hình triển khai tách process để UI mượt hơn:

```text
ESP32 -> WiFi -> Mosquitto MQTT trên Pi -> OUT_SRC_TRUNGKIEN-SERVER -> SQLite
                                                        |
                                                        v
                                           OUT_SRC_TRUNGKIEN UI Qt trên Pi
```

Pi chạy ba phần:

- Mosquitto MQTT broker: `0.0.0.0:1883` để ESP32 kết nối vào.
- Server C++/Qt: `127.0.0.1:8080`, nhận MQTT, lưu SQLite, trả API.
- UI Qt: chạy trực tiếp trên màn hình Pi, gọi API local `http://127.0.0.1:8080`.

## Cài Mosquitto cho ESP32 truy cập từ LAN

Tạo `/etc/mosquitto/conf.d/ictu.conf`:

```conf
listener 1883 0.0.0.0
allow_anonymous true
persistence true
persistence_location /var/lib/mosquitto/
```

Restart:

```bash
sudo systemctl restart mosquitto
sudo systemctl enable mosquitto
sudo ss -ltnp | grep ':1883'
```

Kết quả đúng phải có `0.0.0.0:1883`, không chỉ `127.0.0.1:1883`.

## Cài server chạy nền

Copy binary server vào:

```bash
sudo install -m 755 OUT_SRC_TRUNGKIEN-SERVER /usr/local/bin/OUT_SRC_TRUNGKIEN-SERVER
sudo mkdir -p /var/lib/ictu-environment
sudo cp deploy/pi/systemd/ictu-environment-server.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ictu-environment-server
sudo systemctl restart ictu-environment-server
sudo systemctl status ictu-environment-server
```

Server mặc định:

```text
HTTP API: 127.0.0.1:8080
MQTT:     127.0.0.1:1883
SQLite:   /var/lib/ictu-environment/environment.db
```

## Cài UI tự mở khi Pi vào desktop

Copy binary UI vào:

```bash
sudo install -m 755 OUT_SRC_TRUNGKIEN /usr/local/bin/OUT_SRC_TRUNGKIEN
mkdir -p ~/.config/autostart
cp deploy/pi/autostart/ictu-environment-ui.desktop ~/.config/autostart/
```

Sau khi reboot, Pi vào desktop là UI tự mở.

Chạy thử thủ công:

```bash
/usr/local/bin/OUT_SRC_TRUNGKIEN
```

## Cấu hình ESP32

ESP32 không dùng `127.0.0.1`. ESP32 phải trỏ MQTT tới IP LAN của Pi, ví dụ:

```text
MQTT host: 192.168.12.66
MQTT port: 1883
```

Trong app UI, API dùng `127.0.0.1:8080` vì UI chạy cùng máy Pi với server.

## Kiểm tra nhanh

Health API:

```bash
curl http://127.0.0.1:8080/api/health
```

Nghe MQTT từ ESP32:

```bash
mosquitto_sub -h 127.0.0.1 -p 1883 -t 'iot/v1/devices/+/telemetry' -v
```

Log server:

```bash
journalctl -u ictu-environment-server -f
```
