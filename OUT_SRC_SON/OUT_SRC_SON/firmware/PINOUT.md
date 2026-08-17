# ESP32 firmware pinout

Tài liệu này mô tả chân kết nối đang được khai báo trong firmware tại
`sketch_aug3a`. Các số chân trong bảng là số **GPIO/IO**, không phải số thứ tự
chân vật lý trên bo mạch.

## Pinout toàn bộ ESP32 DevKit V1

Sơ đồ dưới đây áp dụng cho bản **DOIT ESP32 DevKit V1 30 chân** phổ biến. Đặt
bo mạch thẳng đứng, anten ở phía trên và cổng Micro-USB ở phía dưới:

```text
                         ANTEN WI-FI
                    ┌─────────────────┐
              EN  ● │                 │ ● GPIO23  <- BMP180 SCL
     GPIO36 / VP  ● │                 │ ● GPIO22  <- BMP180 SDA
     GPIO39 / VN  ● │    ESP-WROOM    │ ● GPIO1   TX0
          GPIO34  ● │      -32        │ ● GPIO3   RX0
   <- HỒNG NGOẠI     │                 │
          GPIO35  ● │                 │ ● GPIO21
          GPIO32  ● │                 │ ● GPIO19
          GPIO33  ● │                 │ ● GPIO18
          GPIO25  ● │                 │ ● GPIO5
          GPIO26  ● │                 │ ● GPIO17  TX2
          GPIO27  ● │                 │ ● GPIO16  RX2
          GPIO14  ● │                 │ ● GPIO4   <- CÒI
          GPIO12  ● │                 │ ● GPIO2
          GPIO13  ● │                 │ ● GPIO15
             GND  ● │                 │ ● GND
             VIN  ● │    MICRO-USB    │ ● 3V3
                    └────────┬────────┘
                             USB
```

> Nếu bo thực tế có 36 hoặc 38 chân thì vị trí vật lý sẽ khác. Hãy đối chiếu
> chữ `DOIT ESP32 DEVKIT V1` và đếm đủ 15 chân mỗi bên trước khi đấu dây.

### Chức năng các nhóm chân

| Nhóm | Chân | Ghi chú |
|---|---|---|
| Nguồn | `VIN`, `3V3`, `GND` | `VIN` thường là nguồn 5 V; GPIO chỉ chịu logic 3,3 V |
| Điều khiển | `EN` | Kéo thấp để reset chip |
| Input-only | GPIO34, GPIO35, GPIO36, GPIO39 | Không dùng làm output; không có pull-up/pull-down nội |
| ADC | GPIO32–39, GPIO25–27, GPIO0, GPIO2, GPIO4, GPIO12–15 | ADC2 có thể xung đột khi Wi-Fi hoạt động |
| DAC | GPIO25, GPIO26 | Hai ngõ ra DAC tích hợp |
| I2C mặc định | GPIO21 SDA, GPIO22 SCL | Firmware này đã đổi thành SDA GPIO22, SCL GPIO23 |
| SPI thường dùng | GPIO5, GPIO18, GPIO19, GPIO23 | SS, SCK, MISO, MOSI theo cấu hình thường gặp |
| UART0 | GPIO1 TX0, GPIO3 RX0 | Dùng nạp firmware và Serial Monitor |
| UART2 | GPIO17 TX2, GPIO16 RX2 | Có thể dùng giao tiếp thiết bị ngoài |
| Strapping/boot | GPIO0, GPIO2, GPIO4, GPIO5, GPIO12, GPIO15 | Tránh để ngoại vi ép sai mức lúc khởi động |

GPIO6 đến GPIO11 thường nối với flash bên trong module ESP-WROOM-32 và không
được đưa ra hàng chân DevKit V1; không sử dụng chúng cho cảm biến.

## Bảng chân đang sử dụng

| Thiết bị | Chân thiết bị | Chân ESP32 | Hướng | Khai báo trong code |
|---|---|---:|---|---|
| BMP180/GY-68 | SDA | GPIO22 | I/O, I2C data | `BMP180_SDA_PIN` |
| BMP180/GY-68 | SCL | GPIO23 | Output, I2C clock | `BMP180_SCL_PIN` |
| Cảm biến hồng ngoại | OUT/SIGNAL | GPIO34 | Input | `CAM_BIEN_HONG_NGOAI` |
| Còi/buzzer | IN/SIGNAL | GPIO4 | Output | `RING_PIN` |
| LED trạng thái | IN/anode | GPIO2 | Output | `LED_PIN` |
| Nút cấu hình | Một đầu nút | GPIO15 | Input pull-up | `BUTTON_PIN` |
| Tất cả module | GND | GND | Nguồn | GND chung |

## Sơ đồ đấu nối

```text
ESP32                         BMP180 / GY-68
-----                         -------------
3V3   ----------------------> VCC
GND   ----------------------> GND
GPIO22 ---------------------> SDA
GPIO23 ---------------------> SCL

ESP32                         Cảm biến hồng ngoại
-----                         --------------------
3V3   ----------------------> VCC (*)
GND   ----------------------> GND
GPIO34 <--------------------- OUT / SIGNAL

ESP32                         Buzzer module
-----                         -------------
Nguồn theo module ----------> VCC
GND   ----------------------> GND
GPIO4 ----------------------> IN / SIGNAL

ESP32                         Nút cấu hình
-----                         ------------
GPIO15 ---------------------- Một đầu nút
GND    ---------------------- Đầu nút còn lại
```

(*) Chỉ cấp nguồn theo đúng thông số của module cảm biến. Điện áp đưa vào GPIO
của ESP32 không được vượt quá 3,3 V.

## Mức logic trong firmware

### Cảm biến hồng ngoại

Firmware hiện coi tín hiệu mức thấp là phát hiện vật cản:

```c
#define VAT_CAN_ACTIVE_LEVEL 0
```

| GPIO34 | Trạng thái hiện tại |
|---:|---|
| `LOW` / `0` | Phát hiện vật cản |
| `HIGH` / `1` | Không phát hiện vật cản |

GPIO34 là chân chỉ có chức năng input và không có pull-up/pull-down nội tích hợp
như nhiều GPIO khác của ESP32. Module phải cung cấp mức logic ổn định hoặc cần
điện trở ngoài phù hợp.

### Còi

| GPIO4 | Trạng thái hiện tại |
|---:|---|
| `LOW` / `0` | Tắt còi |
| `HIGH` / `1` | Bật còi |

Nếu module còi thực tế kích hoạt mức thấp (active-low), cần đảo hai mức trên
trong `ring.c`.

## Lưu ý theo trạng thái code hiện tại

- `Wire.begin(22, 23)` đã khởi tạo bus I2C cho BMP180.
- Địa chỉ I2C thường gặp của BMP180 là `0x77`; nên xác minh bằng I2C scanner
  trên phần cứng thực tế.
- `HW_BMP180.c` hiện tạo nhiệt độ và áp suất mô phỏng; firmware chưa đọc thanh
  ghi thật từ BMP180.
- GPIO34 hiện được đọc bằng `gpio_get_level()`, nghĩa là tín hiệu digital.
- Nếu cảm biến khoảng cách là **GP2Y0A21YK0F** với đầu ra analog, phải dùng ADC
  (`analogRead` hoặc ADC API), hiệu chuẩn điện áp thành khoảng cách và bảo đảm
  điện áp đầu vào nằm trong giới hạn của ESP32. Cách đọc digital hiện tại không
  trả về khoảng cách theo centimet.
- GPIO4 là chân strapping trên ESP32. Mạch còi không được kéo chân này về mức
  gây sai trạng thái trong lúc ESP32 khởi động. Nếu gặp lỗi boot, nên chuyển còi
  sang một GPIO output thông thường còn trống.

## Nơi thay đổi pin trong source

```text
sketch_aug3a/BMP180.h : GPIO22, GPIO23, GPIO34
sketch_aug3a/ring.h   : GPIO4
```

Sau khi đổi chân, build lại firmware bằng `Ctrl+Shift+B` trong VS Code.
