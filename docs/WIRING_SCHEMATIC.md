# SƠ ĐỒ ĐẤU NỐI MẠCH ĐIỆN VẬT LÝ TOÀN DIỆN & PHÂN TÍCH LOGIC PHẦN CỨNG
## ESP32-S3 (R16N8) FLIGHT CONTROLLER (QUAD-X DRONE)

Tài liệu này cung cấp toàn bộ sơ đồ phân bổ chân (44 chân), kiến trúc phân phối nguồn, bảng ánh xạ 47+ đường dây vật lý đơn lẻ, tích hợp hệ thống tụ lọc kép (**Tụ gốm 104 $0.1\mu\text{F}$ & Tụ hóa $100\mu\text{F}$ / $1000\mu\text{F}$**), giải thích logic mạch điện và phân tích các biện pháp triệt tiêu rủi ro xung đột điện áp / nhiễu động cơ trên Drone Quad-X.

---

## 1. BẢNG QUY HOẠCH TOÀN BỘ CHÂN ESP32-S3 DEVKITC-1 (44 PINS PINOUT)

| Số Chân | Ký hiệu Pin | Loại Chân | Mức Điện Áp | Kết Nối Thiết Bị | Chức Năng Vật Lý & Logic |
| :---: | :--- | :---: | :---: | :--- | :--- |
| **1** | **3V3 (VOUT)** | Power OUT | 3.3V | **VCC Rail Cảm Biến** | Cấp nguồn 3.3V cho MPU6050, BMP280, HMC/QMC5883L, GPS, PCA9685 VCC *(Kèm tụ hóa 100µF + tụ 104)* |
| **2** | **EN / RST** | Control | 3.3V | Nút nhấn Reset | Reset vi điều khiển (Tích hợp sẵn tụ/trở kéo trên DevKit) |
| **3** | **GPIO 4** | Output | 3.3V | **ESC 1 Signal (M1)** | Ngõ ra xung PWM/DShot trực tiếp điều khiển Motor 1 (Trước Phải - CCW) |
| **4** | **GPIO 5** | Output | 3.3V | **ESC 2 Signal (M2)** | Ngõ ra xung PWM/DShot trực tiếp điều khiển Motor 2 (Trước Trái - CW) |
| **5** | **GPIO 6** | Output | 3.3V | **ESC 3 Signal (M3)** | Ngõ ra xung PWM/DShot trực tiếp điều khiển Motor 3 (Sau Phải - CW) |
| **6** | **GPIO 7** | Output | 3.3V | **ESC 4 Signal (M4)** | Ngõ ra xung PWM/DShot trực tiếp điều khiển Motor 4 (Sau Trái - CCW) |
| **7** | **GPIO 1** | ADC1_CH0 | 0–3.1V | **Cầu phân áp VBAT (10k/2.2k)** | Đo điện áp Pin LiPo 3S (11.1V–12.6V) *(Mắc song song tụ 104 lọc mượt tín hiệu)* |
| **8** | **GPIO 2** | ADC1_CH1 | 0–3.1V | **Cảm biến dòng (Current Sense)**| Đo dòng điện tiêu thụ toàn hệ thống từ PDB Shunt Resistor *(Mắc kèm tụ 104)* |
| **9** | **GPIO 3** | Output | 3.3V | **Đèn Arm LED (Đỏ)** | Sáng liên tục khi Động cơ đã ARM, chớp tắt khi chưa sẵn sàng |
| **10** | **GPIO 8** | I2C SDA | 3.3V | **SDA Bus (Trở kéo 4.7kΩ)** | Tuyến truyền dữ liệu I2C cho MPU6050, HMC/QMC5883L, BMP280, PCA9685 |
| **11** | **GPIO 9** | I2C SCL | 3.3V | **SCL Bus (Trở kéo 4.7kΩ)** | Tuyến xung đồng hồ I2C tốc độ cao 400kHz |
| **12** | **GPIO 10** | Output | 3.3V | **Active Buzzer 5V** | Điều khiển còi chíp báo Arm/Disarm, cảnh báo pin yếu, tìm drone thất lạc (Qua NPN S8050) |
| **13** | **GPIO 11** | GPIO | 3.3V | *Dự phòng (Aux PWM 1)* | Kênh điều khiển Servo Gimbal hoặc LED dải phụ |
| **14** | **GPIO 12** | GPIO | 3.3V | *Dự phòng (Aux PWM 2)* | Kênh điều khiển Thả tải / Rơ le phụ |
| **15** | **GPIO 13** | GPIO | 3.3V | *Dự phòng (Optical Flow)* | Giao tiếp cảm biến đo dòng quang học / Tránh vật cản |
| **16** | **GPIO 14** | GPIO | 3.3V | *Dự phòng (LiDAR I2C/UART)* | Cảm biến khoảng cách laser đo độ cao siêu chính xác |
| **17** | **GPIO 17** | U1RXD | 3.3V | **GPS ATGM336H TXD** | Nhận chuỗi dữ liệu định vị NMEA ($GNGGA, $GNRMC) 9600 baud |
| **18** | **GPIO 18** | U1TXD | 3.3V | **GPS ATGM336H RXD** | Gửi lệnh cấu hình tần số quét vệ tinh lên 5Hz/10Hz |
| **19** | **GPIO 43** | U2RXD | 3.3V | **RC Receiver Signal (ELRS/SBUS)**| Nhận tín hiệu điều khiển từ tay cầm (CRSF / SBUS / IBUS 115200 baud) |
| **20** | **GPIO 44** | U2TXD | 3.3V | **RC Telemetry Out** | Gửi thông số bay ngược về màn hình tay cầm điều khiển |
| **21** | **GPIO 48** | Data Out | 3.3V | **WS2812 RGB LED (Status)** | Đèn LED đa màu báo trạng thái Cân bằng Gyro, GPS 3D Lock, Lỗi Failsafe |
| **22** | **5V / VIN** | Power IN | 5.0V | **Nguồn UBEC 5V/3A** | Cấp nguồn chính toàn bộ mạch ESP32-S3 *(Mắc tụ hóa 100µF + tụ 104 chống reset brownout)* |
| **23..26**| **GND (Mass)**| Power GND | 0V | **Common Ground System** | Nối chung toàn bộ Mass của Pin, PDB, ESC, Cảm biến, GPS, Tụ lọc và Buzzer |
| **27..31**| **GPIO 33..37**| SPI Octal | 3.3V | ⚠️ **CẤM DÙNG (Bị khóa)** | Chân bus tốc độ cao nội bộ kết nối Octal SPI Flash 16MB & PSRAM 8MB |
| **32, 33**| **GPIO 19, 20**| USB D-/D+ | 3.3V | ⚠️ **NATIVE USB CDC** | Cổng nạp Firmware và truyền dữ liệu Telemetry GCS Tuner lên máy tính |

---

## 2. HỆ THỐNG TỤ LỌC KÉP: TỤ GỐM 104 & TỤ HÓA 100µF

Khi 4 động cơ không chổi than A2212 hoạt động và 4 ESC chuyển mạch băm xung ở tần số cao, chúng sinh ra **2 loại nhiễu phá hoại**:
1. **Nhiễu xung nhọn cao tần (High-frequency Voltage Spikes / Ringing)**: Do cuộn cảm động cơ và diode ESC tạo ra, làm tê liệt đường truyền I2C của MPU6050, BMP280.
2. **Sụt áp đột ngột (Voltage Sag / Brownout)**: Khi thốc ga mạnh, dòng điện tăng vọt khiến điện áp 5V và 3.3V bị sụt trong vài micro-giây, kích hoạt mạch bảo vệ Brownout Reset khiến ESP32-S3 khởi động lại ngay giữa trời.

### 2.1. Phối hợp Tụ gốm 104 và Tụ hóa 100µF (Dual Decoupling Filter)
```
                                 BỘ LỌC NGUỒN KÉP (DUAL FILTER)
                  
  Nguồn VCC (+) ────────┬──────────────────┬─────────────────> Cấp cho ESP32 / Cảm biến
                        │                  │
                     ┌──┴──┐            ┌──┴──┐
                     │ 104 │ Gốm        │  +  │ Tụ Hóa 100µF
                     │0.1uF│ (Không cực)│     │ (Chân dài / Không sọc)
                     └──┬──┘            └──┬──┘
                        │                  │ - (Chân có vạch sọc xám)
  Mass GND  (-) ────────┴──────────────────┴─────────────────> GND Chung
```

### 2.2. Chi tiết Vị Trí & Tác Dụng Của Từng Tụ

| STT | Vị Trí Lắp Đặt | Loại Tụ & Giá Trị | Điện Áp Chịu Đựng | Mục Đích Kỹ Thuật |
| :---: | :--- | :--- | :--- | :--- |
| **1** | **Đầu vào 5V/VIN của ESP32-S3** | 1x Tụ hóa **100µF** + 1x Tụ gốm **104** | $\ge 10\text{V}$ (10V/16V/25V) | **Chống Brownout Reset ESP32-S3** khi còi kêu, GPS thu phát hoặc motor đề-pa |
| **2** | **Rail nguồn 3.3V nuôi Cảm biến I2C** | 1x Tụ hóa **100µF** + 1x Tụ gốm **104** | $\ge 6.3\text{V}$ (10V/16V/25V) | Giữ phẳng tuyệt đối điện áp 3.3V cho MPU6050, BMP280, HMC5883L |
| **3** | **Sát chân VCC-GND module MPU6050** | 1x Tụ gốm **104 ($0.1\mu\text{F}$)** | $\ge 10\text{V}$ | Triệt nhiễu cao tần, giúp Gyro/Accel không bị rung giật hoặc trôi góc (drift) |
| **4** | **Sát chân VCC-GND module BMP280** | 1x Tụ gốm **104 ($0.1\mu\text{F}$)** | $\ge 10\text{V}$ | Lọc nhiễu cảm biến khí áp kế, tránh nhảy độ cao ảo |
| **5** | **Sát chân VCC-GND module HMC5883L** | 1x Tụ gốm **104 ($0.1\mu\text{F}$)** | $\ge 10\text{V}$ | Giữ nguồn từ kế ổn định, hạn chế nhiễu la bàn |
| **6** | **Sát chân VCC-GND module GPS ATGM336H**| 1x Tụ gốm **104 ($0.1\mu\text{F}$)** | $\ge 10\text{V}$ | Ổn định nguồn cho bộ khuếch đại LNA bắt sóng vệ tinh |
| **7** | **Bộ lọc RC chân đo Pin GPIO 1 (VBAT)** | 1x Tụ gốm **104 ($0.1\mu\text{F}$)** | $\ge 10\text{V}$ | Mắc song song trở $R_2$ ($2.2\text{k}\Omega$) $\rightarrow$ Tạo bộ lọc thông thấp làm phẳng điện áp ADC |
| **8** | **Ngõ vào 4 ESC trên PDB (Tùy chọn)** | 4x Tụ hóa **100µF** (hoặc 1x 1000µF)| ⚠️ **$\ge 25\text{V}$ BẮT BUỘC** | Hấp thụ xung Back-EMF từ cuộn dây động cơ trả ngược về mạch |

---

## 3. KIẾN TRÚC PHÂN PHỐI NGUỒN ĐIỆN VẬT LÝ TOÀN HỆ THỐNG

```
                       +-------------------------------+
                       |   PIN LIPO 3S (11.1V - 12.6V) |
                       |      2200mAh >= 25C (XT60)    |
                       +---------------+---------------+
                                       |
                   [ Dây nguồn chính 12-14 AWG Đỏ/Đen ]
                                       v
                    +-------------------------------------+
                    |   POWER DISTRIBUTION BOARD (PDB)    |
                    |   ⚡ Tụ chính: 1000µF/25V Low-ESR   |
                    +---+--------+--------+--------+------+
                        |        |        |        |      |
      +-----------------+        |        |        |      +----------------+
      | V_BAT (11.1V)            |        |        |      | V_BAT (11.1V)  | V_BAT (11.1V)
      v                          v        v        v      v                v
+-----------+              +-------------------------+  +--------------+ +---------------+
| ESC 1 30A |              |   ESC 2, 3, 4 (30A)     |  | Cầu phân áp  | | UBEC 5V/3A   |
| (Front-R) |              | (Front-L, Rear-R, Rear-L|  | 10k / 2.2k   | | (Hạ áp xung) |
| [Tụ 100uF]|              | [Tụ 100uF mỗi ESC]      |  | + Tụ gốm 104 | +-------+-------+
+-----+-----+              +------------+------------+  +-------+------+         | 5.0V
      |                                 |                       |                v
      | 5V BEC (CẮT DÂY ĐỎ)             | (CẮT DÂY ĐỎ)          | V_ADC     +----+----+ [Lọc kép 5V:
      | Đã có UBEC 5V/3A cấp nguồn      | CẮT DÂY ĐỎ            v (0-2.27V) | 5.0V Rail| Tụ 100µF
      +-----------------+---------------+                  +----+----+    +----+----+  + Tụ 104]
                        |                                  | GPIO 1  |         |
                        +----------------------------------+ (ADC1)  |         +----------+
                        |                                  +---------+         |          |
                        v                                                      v          v
         +-------------------------------------------------------------+ +----------+ +-----------+
         |                 ĐƯỜNG NGUỒN 5.0V HỆ THỐNG                   | | PCA9685  | | Còi Chíp  |
         +--------------+------------------------------+---------------+ | V+ Power | | Buzzer 5V |
                        |                              |                 +----------+ +-----------+
            +-----------+-----------+                  | 5V nuôi RC
            | 5.0V + Tụ 100µF & 104 | 5.0V             v
            v                       v          +---------------+
    +---------------+       +---------------+  | RC Receiver   |
    | ESP32-S3 DEV  |       | Active Buzzer |  | (ELRS / SBUS) |
    | (Chân 5V/VIN) |       | (Qua NPN Q1)  |  +---------------+
    +-------+-------+       +---------------+
            | 3.3V LDO nội cực sạch (Max 600mA)
            v
    +---------------+ [Lọc kép 3.3V: Tụ hóa 100µF + Tụ gốm 104]
    | ĐƯỜNG 3.3V    |
    +-------+-------+
            |
   +--------+--------+----------------+----------------+
   | 3.3V + Tụ 104   | 3.3V + Tụ 104  | 3.3V + Tụ 104  | 3.3V + Tụ 104
   v                 v                v                v
+------+          +------+         +------+      +-----------+
| MPU  |          | HMC/ |         | BMP  |      | GPS       |
| 6050 |          | QMC  |         | 280  |      | ATGM336H  |
+------+          +------+         +------+      +-----------+
```

---

## 4. DANH MỤC CHI TIẾT 50 ĐƯỜNG DÂY & ĐIỂM HÀN VẬT LÝ

| STT | Phân Loại Tuyến | Tên Đường Dây / Linh Kiện | Điểm Đầu (From) | Điểm Cuối (To) | Tiết Diện & Màu Sắc | Ý Nghĩa Kỹ Thuật |
| :---: | :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | Nguồn Động Lực | Cực Dương Pin (+) | Jack XT60 Đực (+) | Trạm hàn Dương PDB (+) | 12–14 AWG Silicon Đỏ | Dẫn dòng xả đỉnh 45A–60A |
| **2** | Nguồn Động Lực | Cực Âm Pin (-) | Jack XT60 Đực (-) | Trạm hàn Âm PDB (-) | 12–14 AWG Silicon Đen | Tuyến mass nguồn chính |
| **3** | Nguồn Động Lực | Nguồn ESC 1 (+) | Trạm PDB ESC1 (+) | Dây Đỏ nguồn ESC 1 | 14–16 AWG Đỏ | Cấp 11.1V cho ESC 1 (M1) |
| **4** | Nguồn Động Lực | Nguồn ESC 1 (-) | Trạm PDB ESC1 (-) | Dây Đen nguồn ESC 1 | 14–16 AWG Đen | Tuyến mass công suất ESC 1 |
| **5** | Nguồn Động Lực | Nguồn ESC 2 (+) | Trạm PDB ESC2 (+) | Dây Đỏ nguồn ESC 2 | 14–16 AWG Đỏ | Cấp 11.1V cho ESC 2 (M2) |
| **6** | Nguồn Động Lực | Nguồn ESC 2 (-) | Trạm PDB ESC2 (-) | Dây Đen nguồn ESC 2 | 14–16 AWG Đen | Tuyến mass công suất ESC 2 |
| **7** | Nguồn Động Lực | Nguồn ESC 3 (+) | Trạm PDB ESC3 (+) | Dây Đỏ nguồn ESC 3 | 14–16 AWG Đỏ | Cấp 11.1V cho ESC 3 (M3) |
| **8** | Nguồn Động Lực | Nguồn ESC 3 (-) | Trạm PDB ESC3 (-) | Dây Đen nguồn ESC 3 | 14–16 AWG Đen | Tuyến mass công suất ESC 3 |
| **9** | Nguồn Động Lực | Nguồn ESC 4 (+) | Trạm PDB ESC4 (+) | Dây Đỏ nguồn ESC 4 | 14–16 AWG Đỏ | Cấp 11.1V cho ESC 4 (M4) |
| **10**| Nguồn Động Lực | Nguồn ESC 4 (-) | Trạm PDB ESC4 (-) | Dây Đen nguồn ESC 4 | 14–16 AWG Đen | Tuyến mass công suất ESC 4 |
| **11**| 12 Dây Pha Động Cơ | Pha U - Motor 1 | Cọc ra ESC 1 (U) | Động cơ A2212 (M1-A) | 18 AWG Xanh dương | Pha A động cơ M1 (CCW) |
| **12**| 12 Dây Pha Động Cơ | Pha V - Motor 1 | Cọc ra ESC 1 (V) | Động cơ A2212 (M1-B) | 18 AWG Vàng | Pha B động cơ M1 (CCW) |
| **13**| 12 Dây Pha Động Cơ | Pha W - Motor 1 | Cọc ra ESC 1 (W) | Động cơ A2212 (M1-C) | 18 AWG Đỏ/Đen | Pha C động cơ M1 (CCW) |
| **14**| 12 Dây Pha Động Cơ | Pha U - Motor 2 | Cọc ra ESC 2 (U) | Động cơ A2212 (M2-B) | 18 AWG Xanh dương | **Đảo pha A-B** để M2 quay CW |
| **15**| 12 Dây Pha Động Cơ | Pha V - Motor 2 | Cọc ra ESC 2 (V) | Động cơ A2212 (M2-A) | 18 AWG Vàng | **Đảo pha A-B** để M2 quay CW |
| **16**| 12 Dây Pha Động Cơ | Pha W - Motor 2 | Cọc ra ESC 2 (W) | Động cơ A2212 (M2-C) | 18 AWG Đỏ/Đen | Pha C động cơ M2 |
| **17**| 12 Dây Pha Động Cơ | Pha U - Motor 3 | Cọc ra ESC 3 (U) | Động cơ A2212 (M3-B) | 18 AWG Xanh dương | **Đảo pha A-B** để M3 quay CW |
| **18**| 12 Dây Pha Động Cơ | Pha V - Motor 3 | Cọc ra ESC 3 (V) | Động cơ A2212 (M3-A) | 18 AWG Vàng | **Đảo pha A-B** để M3 quay CW |
| **19**| 12 Dây Pha Động Cơ | Pha W - Motor 3 | Cọc ra ESC 3 (W) | Động cơ A2212 (M3-C) | 18 AWG Đỏ/Đen | Pha C động cơ M3 |
| **20**| 12 Dây Pha Động Cơ | Pha U - Motor 4 | Cọc ra ESC 4 (U) | Động cơ A2212 (M4-A) | 18 AWG Xanh dương | Pha A động cơ M4 (CCW) |
| **21**| 12 Dây Pha Động Cơ | Pha V - Motor 4 | Cọc ra ESC 4 (V) | Động cơ A2212 (M4-B) | 18 AWG Vàng | Pha B động cơ M4 (CCW) |
| **22**| 12 Dây Pha Động Cơ | Pha W - Motor 4 | Cọc ra ESC 4 (W) | Động cơ A2212 (M4-C) | 18 AWG Đỏ/Đen | Pha C động cơ M4 (CCW) |
| **23**| Nguồn Hạ Áp UBEC | Nguồn Vào UBEC (+) | Trạm hàn PDB (+) | Dây Đỏ vào UBEC | 20 AWG Đỏ | Cấp 11.1V từ PDB vào UBEC |
| **24**| Nguồn Hạ Áp UBEC | Nguồn Vào UBEC (-) | Trạm hàn PDB (-) | Dây Đen vào UBEC | 20 AWG Đen | Mass nguồn UBEC |
| **25**| Nguồn 5V Hệ Thống | 5V Chính ESP32 | Đầu ra UBEC 5V (+) | ESP32-S3 Chân 5V/VIN | 22 AWG Đỏ | Cấp nguồn nuôi bo mạch ESP32 |
| **26**| Nguồn 5V Hệ Thống | 5V Nguồn PCA9685 | Đầu ra UBEC 5V (+) | PCA9685 Cọc V+ | 22 AWG Đỏ | Cấp nguồn công suất cho driver |
| **27**| Common Ground | Mass Chung Hệ Thống | Đầu ra UBEC GND (-) | ESP32 GND & Common Rail | 22 AWG Đen | Tuyến mass chuẩn (Common Ground) |
| **28**| Nguồn 3.3V Logic | 3.3V Nuôi Cảm Biến | Chân 3V3 của ESP32-S3 | Rail VCC Cảm biến | 24–26 AWG Vàng | Nguồn 3.3V LDO nội sạch |
| **29**| I2C Bus 400kHz | I2C Data Line (SDA) | ESP32-S3 GPIO 8 | Chân SDA (MPU, BMP, HMC, PCA) | 26 AWG Xanh lá | Tuyến truyền dữ liệu số I2C |
| **30**| I2C Bus 400kHz | I2C Clock Line (SCL) | ESP32-S3 GPIO 9 | Chân SCL (MPU, BMP, HMC, PCA) | 26 AWG Xanh dương | Tuyến xung nhịp 400kHz |
| **31**| Trở kéo I2C Pull-up | Trở kéo R1 (SDA) | Đường SDA (GPIO 8) | Đường nguồn 3.3V | Trở 4.7kΩ (1/4W) | Kéo áp SDA giữ xung sắc nét |
| **32**| Trở kéo I2C Pull-up | Trở kéo R2 (SCL) | Đường SCL (GPIO 9) | Đường nguồn 3.3V | Trở 4.7kΩ (1/4W) | Kéo áp SCL chống méo sóng |
| **33**| Chân Cấu Hình | MPU6050 AD0 | Chân AD0 MPU6050 | GND | Dây câu ngắn | Cố định địa chỉ I2C = 0x68 |
| **34**| Chân Cấu Hình | BMP280 SDO | Chân SDO BMP280 | GND | Dây câu ngắn | Cố định địa chỉ I2C = 0x76 |
| **35**| Chân Cấu Hình | BMP280 CSB | Chân CSB BMP280 | Đường 3.3V | Dây câu ngắn | Ép BMP280 chạy giao thức I2C |
| **36**| Chân Cấu Hình | PCA9685 OE | Chân OE PCA9685 | GND | Dây câu ngắn | Luôn kích hoạt đầu ra PWM |
| **37**| Tuyến Điều Khiển Motor | PWM ESC 1 | ESP32 GPIO 4 (hoặc PCA CH0)| Dây Trắng/Cam ESC 1 | Dây tín hiệu Servo | Xung PWM 50–400Hz điều khiển M1 |
| **38**| Tuyến Điều Khiển Motor | PWM ESC 2 | ESP32 GPIO 5 (hoặc PCA CH1)| Dây Trắng/Cam ESC 2 | Dây tín hiệu Servo | Xung PWM 50–400Hz điều khiển M2 |
| **39**| Tuyến Điều Khiển Motor | PWM ESC 3 | ESP32 GPIO 6 (hoặc PCA CH2)| Dây Trắng/Cam ESC 3 | Dây tín hiệu Servo | Xung PWM 50–400Hz điều khiển M3 |
| **40**| Tuyến Điều Khiển Motor | PWM ESC 4 | ESP32 GPIO 7 (hoặc PCA CH3)| Dây Trắng/Cam ESC 4 | Dây tín hiệu Servo | Xung PWM 50–400Hz điều khiển M4 |
| **41**| Giám Sát Điện Áp Pin | VBAT ADC Sense | Cầu phân áp PDB (+) | ESP32-S3 GPIO 1 (ADC1_CH0)| 26 AWG Tím | Cầu trở 10kΩ / 2.2kΩ đo áp Pin 3S |
| **42**| Âm Thanh & Báo Động | Tín hiệu Còi Buzzer | ESP32-S3 GPIO 10 | Cực B Transistor NPN (S8050)| 26 AWG Cam | Kích còi kêu bíp khi Arm/Lỗi |
| **43**| Chỉ Báo Trạng Thái | Đèn Arm LED (Đỏ) | ESP32-S3 GPIO 3 | Anode (+) LED Đỏ (Qua trở 330Ω)| 26 AWG Đỏ mỏng | Sáng khi động cơ đã ARM |
| **44**| Định Vị GPS UART1 | GPS TXD $\rightarrow$ RX1 | GPS ATGM336H TXD | ESP32-S3 GPIO 17 (U1RXD) | 26 AWG Hồng | Đọc câu định vị NMEA 9600 baud |
| **45**| Định Vị GPS UART1 | GPS RXD $\leftarrow$ TX1 | GPS ATGM336H RXD | ESP32-S3 GPIO 18 (U1TXD) | 26 AWG Xanh lam | Gửi lệnh cấu hình tần số GPS |
| **46**| Bộ Thu Tay Cầm RC | RC Signal $\rightarrow$ RX2 | Tay thu ELRS/SBUS Signal | ESP32-S3 GPIO 43 (U2RXD) | 26 AWG Trắng | Nhận tín hiệu điều khiển RC |
| **47**| Bộ Thu Tay Cầm RC | RC Telemetry $\leftarrow$ TX2 | Tay thu ELRS Telemetry | ESP32-S3 GPIO 44 (U2TXD) | 26 AWG Vàng mỏng | Hồi tiếp dữ liệu bay về Remote |
| **48**| **Tụ Lọc Chống Brownout**| **Tụ Hóa 100µF + Tụ 104**| Chân 5V/VIN của ESP32 | Chân GND của ESP32 | Hàn sát chân | Triệt tiêu sụt áp 5V nuôi MCU |
| **49**| **Tụ Lọc Cảm Biến I2C** | **Tụ Hóa 100µF + Tụ 104**| Rail nguồn 3.3V | Rail nguồn GND | Hàn trên đường 3.3V | Giữ phẳng nguồn nuôi IMU/BMP |
| **50**| **Tụ Lọc Mượt Pin ADC** | **Tụ Gốm 104 (0.1µF)** | ESP32 GPIO 1 (VBAT) | GND | Hàn song song $R_2$ | Lọc nhiễu số đọc điện áp Pin |

---

## 5. HƯỚNG DẪN HÀN & LẮP ĐẶT THỰC TẾ AN TOÀN

1. **Quy tắc phân cực Tụ hóa 100µF**:
   - Chân có **vạch màu xám/trắng kèm dấu trừ (-)** hoặc chân ngắn hơn $\rightarrow$ BẮT BUỘC hàn vào **GND (Mass)**.
   - Chân dài hơn (không có vạch) $\rightarrow$ Hàn vào cực dương **5V hoặc 3.3V**.
2. **Quy tắc điện áp tụ hóa**:
   - Mắc vào nguồn **5V hoặc 3.3V**: Dùng tụ $100\mu\text{F}$ loại **10V, 16V, 25V, 35V**.
   - Mắc vào nguồn **Pin LiPo 3S 11.1V–12.6V (tại PDB)**: BẮT BUỘC dùng tụ hóa **từ 25V trở lên (25V, 35V, 50V)**. *Không dùng tụ 10V/16V vào nguồn Pin vì sẽ bị nổ khi động cơ trả xung ngược!*
3. **Quy tắc tụ gốm 104 ($0.1\mu\text{F}$)**:
   - Tụ gốm không phân cực $\rightarrow$ Hàn càng sát chân VCC-GND của MPU6050, BMP280, ESP32 càng tốt (chiều dài chân tụ $< 5\text{mm}$).
