# HƯỚNG DẪN CẤU HÌNH CHẠY SONG SONG KẾT NỐI WI-FI VÀ PHÁT ACCESS POINT (AP-STA) TRÊN RASPBERRY PI

Tài liệu này ghi lại toàn bộ các bước và câu lệnh thực tế đã được cấu hình thành công trên Raspberry Pi bằng **NetworkManager** với một chip Wi-Fi duy nhất (`wlan0`).

---

## 1. Nguyên lý hoạt động
- **`wlan0` (Client / STA mode)**: Kết nối vào mạng Wi-Fi có sẵn (ví dụ: `VanPhong`) để lấy mạng Internet.
- **`wlan1` (Virtual AP / Hotspot)**: Giao diện mạng ảo được tạo trên nền chip vật lý `wlan0` để phát mạng Wi-Fi (ví dụ: `ICTU_VANPHONG_AP`) và tự động chia sẻ Internet (NAT/DHCP) cho các thiết bị kết nối vào.

---

## 2. Các bước cấu hình chi tiết

### Bước 1: Kích hoạt NetworkManager và cài đặt công cụ cần thiết
```bash
# 1. Mở khóa Wi-Fi nếu bị chặn
sudo rfkill unblock all

# 2. Bật dịch vụ NetworkManager
sudo systemctl enable --now NetworkManager

# 3. Cài đặt công cụ quản lý wireless (iw)
sudo apt update && sudo apt install -y iw
```

---

### Bước 2: Kết nối Raspberry Pi vào mạng Wi-Fi chính (Lấy Internet)
*(Thay `TEN_WIFI_NHA` và `MAT_KHAU_WIFI` bằng thông tin Wi-Fi thực tế)*:
```bash
sudo nmcli dev wifi connect "TEN_WIFI_NHA" password "MAT_KHAU_WIFI"
```

---

### Bước 3: Tạo cấu hình phát Hotspot `ICTU_VANPHONG_AP`
```bash
# 1. Tạo profile AP gắn vào interface ảo wlan1
sudo nmcli con add type wifi ifname wlan1 con-name ICTU_VANPHONG_AP autoconnect no ssid ICTU_VANPHONG_AP

# 2. Cấu hình chế độ AP, bảo mật WPA2 và chia sẻ Internet
sudo nmcli con modify ICTU_VANPHONG_AP 802-11-wireless.mode ap 802-11-wireless.band bg
sudo nmcli con modify ICTU_VANPHONG_AP wifi-sec.key-mgmt wpa-psk
sudo nmcli con modify ICTU_VANPHONG_AP wifi-sec.psk "12345678"
sudo nmcli con modify ICTU_VANPHONG_AP ipv4.method shared
```

---

### Bước 4: Tạo Systemd Service để tự động phát AP khi khởi động máy
Tạo dịch vụ `virtual-ap.service` để tự động tạo `wlan1` và bật AP sau mỗi lần boot:

```bash
sudo tee /etc/systemd/system/virtual-ap.service << 'EOF'
[Unit]
Description=Auto Create Virtual AP (ICTU_VANPHONG_AP)
After=network.target NetworkManager.service
Wants=NetworkManager.service

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStartPre=/bin/sleep 5
ExecStart=/bin/bash -c "iw dev wlan0 interface add wlan1 type __ap && ip link set wlan1 up && nmcli device set wlan1 managed yes && nmcli con up id ICTU_VANPHONG_AP ifname wlan1"

[Install]
WantedBy=multi-user.target
EOF
```

Kích hoạt và chạy service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable virtual-ap.service
sudo systemctl restart virtual-ap.service
```

---

### Bước 5 (Tùy chọn): Tạo Dispatcher Script cho NetworkManager
Đảm bảo khi `wlan0` kết nối mạng thì tự động kích hoạt `wlan1`:

```bash
sudo tee /etc/NetworkManager/dispatcher.d/00-virtualwlan1 << 'EOF'
#!/bin/sh
IFACE="$1"
ACTION="$2"
HOTSPOT_CON="ICTU_VANPHONG_AP"

if [ "$IFACE" = "wlan0" ]; then
    if [ "$ACTION" = "up" ]; then
        iw dev wlan0 interface add wlan1 type __ap 2>/dev/null
        ip link set wlan1 up 2>/dev/null
        sleep 1
        nmcli device set wlan1 managed yes 2>/dev/null
        nmcli con up id "$HOTSPOT_CON" ifname wlan1
    fi

    if [ "$ACTION" = "down" ]; then
        nmcli con down id "$HOTSPOT_CON" 2>/dev/null
        iw dev wlan1 del 2>/dev/null
    fi
fi

exit 0
EOF

# Phân quyền chuẩn cho script
sudo chown root:root /etc/NetworkManager/dispatcher.d/00-virtualwlan1
sudo chmod 755 /etc/NetworkManager/dispatcher.d/00-virtualwlan1
sudo systemctl enable --now NetworkManager-dispatcher.service
```

---

## 3. Các lệnh kiểm tra trạng thái hoạt động

Kiểm tra danh sách mạng đang hoạt động:
```bash
nmcli con show --active
```
*Kết quả chuẩn:*
- `wlan0`: Đang kết nối mạng Wi-Fi chính (Client).
- `wlan1`: Đang chạy `ICTU_VANPHONG_AP` (Hotspot).

Kiểm tra danh sách interface phần cứng & ảo:
```bash
nmcli device
iw dev
```

---

## 4. Lưu ý quan trọng
1. **Báo lỗi trên giao diện Desktop (Taskbar UI)**:
   - Trên Raspberry Pi Desktop cũ, icon mạng mặc định (`lxplug-network`) dùng cho `dhcpcd` nên có thể hiện dòng thông báo *"No wireless LAN interfaces found"*.
   - **Thực tế:** Mạng bên dưới vẫn hoạt động 100% bình thường. Có thể cài thêm `network-manager-gnome` (`nm-applet`) nếu muốn có icon chuẩn NetworkManager.
2. **Kênh tần số (Channel)**: Mạng phát `ICTU_VANPHONG_AP` sẽ tự động chạy cùng tần số/kênh (channel) với mạng Wi-Fi mà `wlan0` kết nối.
3. **Băng thông**: Do dùng chung 1 chip thu/phát, tốc độ truyền tải tối đa sẽ bị chia đôi so với khi chỉ kết nối đơn.
