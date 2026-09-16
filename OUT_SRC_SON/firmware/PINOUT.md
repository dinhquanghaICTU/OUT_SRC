# BẢN ĐỒ CẤU HÌNH CHÂN & SƠ ĐỒ ĐẤU NỐI PHẦN CỨNG (PINOUT)
## Dự án: Hệ thống Quản lý Máy bơm Nước & Đo Lưu Lượng (Firmware `sketch_aug15a`)

Tài liệu này mô tả chi tiết sơ đồ chân GPIO, cấu hình phần cứng và sơ đồ kết nối dây điện dạng **ASCII Art** cho bo mạch điều khiển ESP32 trong thư mục `firmware/sketch_aug15a`.

---

## 1. Bảng Tổng Hợp Cấu Hình Chân GPIO

| Ngoại vi / Thiết bị | Chân thiết bị | GPIO ESP32 | Chiều / Chế độ | Mức logic / Tín hiệu | Định nghĩa trong Code |
| :--- | :--- | :---: | :--- | :--- | :--- |
| **Relay Máy Bơm** | IN / Signal | **GPIO 26** | Digital OUTPUT | Active HIGH (1: BẬT, 0: TẮT) | `RELAY_PIN` (`relay.h`) |
| **Cảm Biến Lưu Lượng**<br>(YF-S201 Water Flow) | OUT / Pulse (Vàng) | **GPIO 27** | Digital INPUT<br>(Kéo lên `PULLUP`) | Ngắt sườn lên `RISING`<br>(~450 xung / Lít) | `WATER_FLOW_PIN` (`pump_peripheral.h`) |
| **Cảm Biến Siêu Âm**<br>(HC-SR04 / US-015) | TRIG | **GPIO 18** | Digital OUTPUT | Xung kích 10µs HIGH | `HCSR_TRIG` (`HCSR_04.cpp`) |
| **Cảm Biến Siêu Âm**<br>(HC-SR04 / US-015) | ECHO | **GPIO 5** | Digital INPUT | Độ rộng xung `pulseIn` (3.3V logic) | `HCSR_ECHO` (`HCSR_04.cpp`) |
| **LED Trạng Thái** | Anode (+) | **GPIO 2** | Digital OUTPUT | Active HIGH (LED On-board hoặc ngoài) | `LED_PIN` (`led.h`) |
| **Nguồn Cấp Hệ Thống** | VCC / VIN / GND | **VIN / 3V3 / GND**| Nguồn | VIN = 5V DC, 3V3 = 3.3V DC, GND chung | Bo mạch ESP32 |

---

## 2. Sơ Đồ Chân Bo Mạch ESP32 DevKit V1 (30 Chân)

Sơ đồ nhìn từ mặt trên bo mạch (ăng-ten Wi-Fi ở trên đỉnh, cổng Micro-USB ở dưới):

```text
                           ĂNG-TEN WI-FI PCB
                         ┌───────────────────┐
                         │   ESP-WROOM-32    │
                         │    [Dual Core]    │
                         │                   │
                   EN  ●─┤ 1              30 ├─●  GPIO 23
          GPIO 36 (VP) ●─┤ 2              29 ├─●  GPIO 22
          GPIO 39 (VN) ●─┤ 3              28 ├─●  GPIO 1  (TX0)
               GPIO 34 ●─┤ 4              27 ├─●  GPIO 3  (RX0)
               GPIO 35 ●─┤ 5              26 ├─●  GPIO 21
               GPIO 32 ●─┤ 6              25 ├─●  GPIO 19
               GPIO 33 ●─┤ 7              24 ├─●  GPIO 18 ──> [HC-SR04 TRIG]
               GPIO 25 ●─┤ 8              23 ├─●  GPIO 5  <── [HC-SR04 ECHO]
    [RELAY IN] GPIO 26 ●─┤ 9              22 ├─●  GPIO 17 (TX2)
  [FLOW SENSOR] GPIO 27 ●─┤ 10             21 ├─●  GPIO 16 (RX2)
               GPIO 14 ●─┤ 11             20 ├─●  GPIO 4
               GPIO 12 ●─┤ 12             19 ├─●  GPIO 2  ──> [STATUS LED]
               GPIO 13 ●─┤ 13             18 ├─●  GPIO 15
                   GND ●─┤ 14             17 ├─●  GND     ─── [MASS CHUNG]
              VIN (5V) ●─┤ 15             16 ├─●  3V3
                         └─────────┬─────────┘
                              [MICRO-USB]
```

---

## 3. Sơ Đồ Đấu Nối Chi Tiết Từng Khối (ASCII Wiring)

### 3.1. Khối Relay Điều Khiển Bơm Nước (GPIO 26)

```text
    +-------------------+                       +-------------------+
    |    ESP32 BOARD    |                       |    RELAY MODULE   |
    |                   |                       |     (1 Channel)   |
    |          VIN (5V) |======================>| VCC (DC+)         |
    |               GND |---------------------->| GND (DC-)         |
    |           GPIO 26 |---------------------->| IN / SIGNAL       |
    +-------------------+                       +---------+---------+
                                                          |
                                                 COM      |      NO
                                              +-----------+-----------+
                                              |                       |
                                              |       [ MÁY BƠM ]     |
                                              |       +---------+     |
                       [ NGUỒN TẢI ]          |       |  MOTOR  |     |
                       (220VAC / 12VDC)       |       +----+----+     |
                         Dây Nóng (L / +) ----+            |          |
                         Dây Nguội (N / -) ----------------+----------+
```
> **Nguyên lý hoạt động:**
> - `GPIO 26 = HIGH (1)`: Relay đóng tiếp điểm `COM` nối `NO` -> Bơm hoạt động.
> - `GPIO 26 = LOW (0)`: Relay nhả tiếp điểm -> Bơm dừng.

---

### 3.2. Khối Cảm Biến Lưu Lượng Nước YF-S201 (GPIO 27)

```text
    +-------------------+                       +-------------------+
    |    ESP32 BOARD    |                       |  FLOW SENSOR      |
    |                   |                       |    (YF-S201)      |
    |          VIN (5V) |======================>| Dây Đỏ   (VCC 5V) |
    |               GND |---------------------->| Dây Đen  (GND)    |
    |           GPIO 27 |<----------------------| Dây Vàng (SIGNAL) |
    +-------------------+                       +-------------------+
```
> **Thông số kỹ thuật:**
> - Cảm biến sử dụng hiệu ứng Hall, chân vàng tạo xung vuông tương ứng với cánh quạt quay.
> - Code sử dụng ngắt sườn lên `RISING` với trở kéo lên nội `INPUT_PULLUP`.
> - Hệ số quy đổi chuẩn: $F (Hz) = 7.5 \times Q (L/\text{phút})$, khoảng 450 xung = 1 Lít nước.

---

### 3.3. Khối Cảm Biến Siêu Âm Đo Mực Nước HC-SR04 (GPIO 18 & GPIO 5)

> ⚠️ **Lưu ý mức điện áp chân ECHO:** Module HC-SR04 thường chạy nguồn 5V nên chân ECHO phát tín hiệu logic 5V. Khuyến nghị thêm cầu phân áp (1kΩ / 2kΩ) hoặc dùng loại HC-SR04P chạy 3.3V để bảo vệ chân GPIO 5 của ESP32.

```text
    +-------------------+                       +-------------------+
    |    ESP32 BOARD    |                       |  HC-SR04 / US-015 |
    |                   |                       |                   |
    |          VIN (5V) |======================>| VCC               |
    |               GND |---+------------------>| GND               |
    |                   |   |                   |                   |
    |           GPIO 18 |---|------------------>| TRIG              |
    |                   |   |                   |                   |
    |           GPIO 5  |<--+-- [R1: 1kΩ] <-----| ECHO (5V Out)     |
    |                   |   |                   |                   |
    +-------------------+   +-- [R2: 2kΩ] --+   +-------------------+
                            |               |
                           GND             GND
```
> **Cơ chế đo:**
> - ESP32 kích xung HIGH 10µs vào `GPIO 18 (TRIG)`.
> - Đo thời gian phản xạ xung HIGH nhận được tại `GPIO 5 (ECHO)`.
> - Tự động bật bơm khi khoảng cách $\ge 35\text{ cm}$ (bể cạn nước).
> - Tự động ngắt bơm khi khoảng cách $\le 10\text{ cm}$ (bể đầy nước).

---

### 3.4. Khối LED Chỉ Báo Trạng Thái (GPIO 2)

```text
    +-------------------+
    |    ESP32 BOARD    |
    |                   |                       +-------------------+
    |            GPIO 2 |------ [R: 220Ω-330Ω] -| Anode (+)   LED   |
    |               GND |-----------------------| Cathode (-) [---] |
    +-------------------+                       +-------------------+
                                            (Hoặc dùng LED On-board GPIO 2)
```

**Mã trạng thái LED:**
- **Chớp nhanh (Fast Blink):** Chế độ cấu hình WiFi AP (`WIFI_MANAGER_AP_CONFIG`).
- **Chớp chậm (Slow Blink):** Đang kết nối WiFi hoặc đã có WiFi nhưng chưa kết nối MQTT.
- **Sáng liên tục (Solid ON):** Hệ thống hoạt động bình thường, kết nối WiFi & MQTT thành công.
- **Tắt (OFF):** Hệ thống ở trạng thái nghỉ hoặc mất nguồn.

---

## 4. Sơ Đồ Kết Nối Toàn Bộ Hệ Thống (Master System Wiring Diagram)

```text
                               +-----------------------------+
                               |     NGUỒN 5VDC / ADAPTER    |
                               +--------------+--------------+
                                              |
                                     +5V (DC) |  GND
                                     +--------+---+
                                     |            |
                                     |            |
+====================================v============v====================================+
|                                    ESP32 DEVKIT V1                                   |
|                                                                                      |
|  [VIN]          [GND]         [GPIO 26]      [GPIO 27]       [GPIO 18]     [GPIO 5]  |
+====|==============|==============|==============|================|============|======+
     |              |              |              |                |            |
     |              |              |              |                |            |
     |              |              |              |                |            |
     +--------------+-------+      |              |                |            |
     |              |       |      |              |                |            |
+----v----+    +----v----+  |      |              |                |            |
| VCC     |    | VCC     |  |      |              |                |            |
|         |    |         |  |      |              |                |            |
| HC-SR04 |    | YF-S201 |  |      |              |                |            |
| SIÊU ÂM |    | LƯU     |  |      |              |                |            |
|         |    | LƯỢNG   |  |      |              |                |            |
| GND TRIG|    | GND OUT |  |      |              |                |            |
+--|---|--+    +--|---|--+  |      |              |                |            |
   |   |          |   |     |      |              |                |            |
   |   +----------+---|-----+------+--------------+----------------+            |
   |   (Trig)     |   +---------------------------+                             |
   |              |   (Pulse)                                                   |
   |              |                                                             |
   |              |         +---------------------------------------------------+
   |              |         | (Echo qua cầu phân áp)
   |              |         |
   |              |    +----v----+
   |              |    | VCC     |
   |              |    |         |
   |              |    | RELAY   |
   |              +--->| GND  IN |<--- [GPIO 26]
   |                   +----+----+
   |                        | [COM]   [NO]
   |                        +---|-------|---+
   |                            |       |   |
   +----------------------------+       |   +--> Ra Tải (Máy Bơm Nước)
                                        +------> Nguồn Tải (AC / DC)
```

---

## 5. Vị Trí Định Nghĩa Chân Trong Mã Nguồn

Khi cần thay đổi chân kết nối phần cứng, chỉnh sửa tại các file tương ứng sau:

1. **Relay máy bơm:**
   - File: `firmware/sketch_aug15a/relay.h`
   - Dòng: `#define RELAY_PIN 26`
2. **Cảm biến lưu lượng (Flow sensor):**
   - File: `firmware/sketch_aug15a/pump_peripheral.h`
   - Dòng: `#define WATER_FLOW_PIN 27`
3. **Cảm biến siêu âm (HC-SR04):**
   - File: `firmware/sketch_aug15a/HCSR_04.cpp`
   - Dòng: `static constexpr uint8_t HCSR_TRIG = 18;`
   - Dòng: `static constexpr uint8_t HCSR_ECHO = 5;`
4. **LED trạng thái:**
   - File: `firmware/sketch_aug15a/led.h`
   - Dòng: `#define LED_PIN 2`
