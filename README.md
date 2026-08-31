# 🚁 ESP32-S3 High-Reliability Flight Controller & GCS Web Tuner

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32--S3%20(R16N8)-red?style=for-the-badge&logo=espressif" alt="ESP32-S3">
  <img src="https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-blue?style=for-the-badge&logo=platformio" alt="PlatformIO">
  <img src="https://img.shields.io/badge/Loop%20Rate-250Hz%20(4ms)-green?style=for-the-badge" alt="250Hz">
  <img src="https://img.shields.io/badge/Architecture-Cascade%20Dual--Loop%20PID-orange?style=for-the-badge" alt="Cascade PID">
  <img src="https://img.shields.io/badge/Control-GCS%20Web%20Serial%20Tuner-purple?style=for-the-badge" alt="GCS Tuner">
</p>

Dự án firmware điều khiển bay (Flight Controller) chuyên dụng cho **Quadcopter khung X (Quad-X)**, xây dựng trên nền tảng vi điều khiển lõi kép **ESP32-S3 (R16N8: 16MB Flash, 8MB Octal PSRAM)**. Hệ thống tích hợp thuật toán lọc tư thế không gian **Mahony AHRS 9-DOF**, kiến trúc điều khiển **Cascade 2-Loop PID** (Vòng ngoài góc nghiêng + Vòng trong vận tốc góc), giao tiếp mặt đất **GCS Web Tuner** qua Web Serial API (Chrome/Edge) và hệ thống lọc nguồn chống nhiễu kép.

---

## 📑 MỤC LỤC
1. [Tính Năng Nổi Bật](#-tính-năng-nổi-bật)
2. [Cấu Trúc Thư Mục Dự Án](#-cấu-trúc-thư-mục-dự-án)
3. [Phần Cứng & Sơ Đồ Chân GPIO](#-phần-cứng--sơ-đồ-chân-gpio)
4. [Bố Trí Động Cơ Quad-X & Chiều Quay](#-bố-trí-động-cơ-quad-x--chiều-quay)
5. [Hệ Thống Tụ Lọc Kép & Chống Nhiễu](#-hệ-thống-tụ-lọc-kép--chống-nhiễu)
6. [Cài Đặt Môi Trường & Nạp Firmware](#-cài-đặt-môi-trường--nạp-firmware)
7. [Quy Trình Kiểm Tra Phần Cứng Trên Bàn Test (Bench Test)](#-quy-trình-kiểm-tra-phần-cứng-trên-bàn-test-bench-test)
8. [Hướng Dẫn Sử Dụng GCS Web Tuner](#-hướng-dẫn-sử-dụng-gcs-web-tuner)
9. [Tập Lệnh Điều Khiển Serial CLI](#-tập-lệnh-điều-khiển-serial-cli)
10. [Cẩm Nang Tune PID Kép (Cascade PID Tuning Guide)](#-cẩm-nang-tune-pid-kép-cascade-pid-tuning-guide)
11. [Bảng Thông Số PID Khởi Điểm](#-bảng-thông-số-pid-khởi-điểm)
12. [Tài Liệu Chi Tiết Đi Kèm](#-tài-liệu-chi-tiết-đi-kèm)

---

## 🌟 TÍNH NĂNG NỔI BẬT

* **Vòng lặp điều khiển 250Hz Non-blocking**: Xử lý đọc cảm biến, tính toán bộ lọc Mahony AHRS và thuật toán hòa trộn động cơ trong chu kỳ 4.0ms chính xác, không dùng `delay()`.
* **Mahony AHRS 9-DOF / 6-DOF Tự Động Thích Ứng**: Hợp nhất dữ liệu Gia tốc kế, Con quay hồi chuyển (MPU6050) và Từ kế (HMC5883L/QMC5883L) giúp ước lượng góc Roll/Pitch/Yaw chính xác, chống trôi góc tuyệt đối.
* **Kiến Trúc PID Kép (Cascade 2-Loop PID)**:
  * **Vòng ngoài (Angle Loop)**: Điều khiển góc nghiêng mục tiêu theo lệnh người lái ($\text{deg}$).
  * **Vòng trong (Rate Loop)**: Điều khiển vận tốc góc tức thời ($\text{deg/s}$) với **Derivative-on-Measurement** và bộ lọc thông thấp (D-term Low-Pass Filter) giúp loại bỏ rung động cơ.
* **GCS Web Tuner (HTML5 / Web Serial API)**: Giao diện trực quan chạy trực tiếp trên Google Chrome / Microsoft Edge, vẽ đồ thị sóng Realtime 60fps, mô hình 3D tư thế máy bay và thanh trượt Tune PID trực tiếp không cần biên dịch lại code.
* **Giám Sát An Toàn Failsafe Đa Cấp**: Tự động ngắt động cơ khẩn cấp khi: Mất sóng điều khiển (>500ms), lật drone quá góc an toàn (>45°), kẹt bus I2C, hoặc pin sụt dưới ngưỡng nguy hiểm.
* **Hệ Thống Lọc Nguồn Kép (Decoupling Capacitor Network)**: Quy chuẩn mắc tụ hóa $100\mu\text{F}$ + tụ gốm $104$ ($0.1\mu\text{F}$) triệt tiêu triệt để hiện tượng sụt áp (Brownout Reset) và xung ngược Back-EMF từ động cơ.

---

## 📁 CẤU TRÚC THƯ MỤC DỰ ÁN

```text
drone/
├── platformio.ini              # Cấu hình PlatformIO cho ESP32-S3 (R16N8)
├── README.md                   # Tài liệu tổng quan & hướng dẫn sử dụng chính (File này)
├── src/
│   ├── main.cpp                # Vòng lặp chính 250Hz (Fast / Medium / Slow Loop)
│   ├── config.h                # Khai báo chân GPIO, tần số, giới hạn an toàn & PID mặc định
│   ├── sensors/
│   │   ├── ImuSensor.h/.cpp        # Driver MPU6050 (I2C 400kHz, DLPF 98Hz, calib Bias)
│   │   ├── Magnetometer.h/.cpp     # Driver HMC5883L / QMC5883L (Tự nhận dạng chip thật/clone)
│   │   ├── Barometer.h/.cpp        # Driver BMP280 (Áp suất khí quyển & ước tính độ cao)
│   │   └── GpsReader.h/.cpp        # Driver GPS ATGM336H (UART1 NMEA 0183 @ 9600 baud)
│   ├── actuators/
│   │   └── MotorController.h/.cpp  # Driver điều khiển ESC qua PCA9685 / LEDC Hardware PWM
│   ├── control/
│   │   ├── AttitudeEstimator.h/.cpp# Thuật toán Mahony AHRS hợp nhất dữ liệu 9-DOF
│   │   ├── PidController.h/.cpp    # Bộ điều khiển PID (Anti-Windup, LPF D-term)
│   │   ├── MotorMixer.h/.cpp       # Hòa trộn công suất Quad-X và quản lý Cascade PID
│   │   ├── ControlInputSource.h    # Interface trừu tượng nhận lệnh điều khiển
│   │   └── SerialControlInput.h/.cpp# Xử lý lệnh Serial CLI & truyền gói tin Telemetry
│   └── safety/
│       └── FailsafeManager.h/.cpp  # Giám sát an toàn: Mất sóng, góc nghiêng >45°, lỗi cảm biến
├── tools/
│   ├── tuner/
│   │   └── index.html          # Giao diện GCS Web Tuner Realtime (Web Serial API)
│   └── i2c_scanner/
│       └── main.cpp            # Tool kiểm tra phát hiện địa chỉ phần cứng I2C
└── docs/
    ├── CIRCUIT_DIAGRAM.html    # Bản vẽ mạch điện SVG đồ họa tương tác phân lớp
    ├── WIRING_SCHEMATIC.md     # Cẩm nang 50 đường dây đấu nối chi tiết
    └── USER_GUIDE_AND_TUNING.md# Cẩm nang chuyên sâu quy trình Tune PID và bay thực địa
```

---

## 🔌 PHẦN CỨNG & SƠ ĐỒ CHÂN GPIO

### 1. Danh sách linh kiện chính
| Linh kiện | Model / Thông số | Giao tiếp | Vai trò |
| :--- | :--- | :--- | :--- |
| **Vi điều khiển** | ESP32-S3 DevKitC-1 (R16N8: 16MB Flash, 8MB PSRAM) | - | Bộ xử lý trung tâm Flight Controller |
| **IMU (Gia tốc + Con quay)** | MPU6050 (0x68) | I2C (400kHz) | Đo gia tốc 3 trục $\pm 4\text{g}$ & vận tốc góc $\pm 500^\circ/\text{s}$ |
| **Từ kế (La bàn số)** | HMC5883L (0x1E) / QMC5883L (0x0D) | I2C (400kHz) | Xác định hướng Bắc từ trường chống trôi Yaw |
| **Áp kế (Đo độ cao)** | BMP280 (0x76) | I2C (400kHz) | Đo áp suất khí quyển $\rightarrow$ Giữ độ cao (Altitude Hold) |
| **GPS** | ATGM336H / NEO-6M/8M | UART1 (9600) | Định vị tọa độ, hỗ trợ Position Hold & RTH |
| **ESC & Động cơ** | 4x ESC 30A + 4x Motor Brushless A2212 1000KV | PWM (50–400Hz) | Hệ thống động lực lực nâng Quadcopter |
| **Nguồn cấp** | Pin LiPo 3S 2200mAh 25C + UBEC 5V/3A rời | Power | Cấp nguồn động lực 11.1V và nguồn hạ áp 5.0V sạch |

### 2. Bảng ánh xạ chân GPIO ESP32-S3
```text
+-------------------------------------------------------------------------------+
|                             ESP32-S3 PIN MAPPING                              |
+-------------------+----------------+-----------+------------------------------+
| Tên Chân / Pin    | GPIO Pin       | Mức Điện  | Chức năng & Thiết bị kết nối |
+-------------------+----------------+-----------+------------------------------+
| PIN_I2C_SDA       | GPIO 8         | 3.3V      | I2C Data Bus (MPU, HMC, BMP) |
| PIN_I2C_SCL       | GPIO 9         | 3.3V      | I2C Clock Bus (400kHz)       |
| PIN_MOTOR_1       | GPIO 4         | 3.3V      | PWM ESC 1: Motor Trước Phải  |
| PIN_MOTOR_2       | GPIO 5         | 3.3V      | PWM ESC 2: Motor Trước Trái  |
| PIN_MOTOR_3       | GPIO 6         | 3.3V      | PWM ESC 3: Motor Sau Phải    |
| PIN_MOTOR_4       | GPIO 7         | 3.3V      | PWM ESC 4: Motor Sau Trái    |
| PIN_VBAT_SENSE    | GPIO 1 (ADC1)  | 0 - 3.1V  | Đo điện áp Pin qua cầu trở   |
| PIN_BUZZER        | GPIO 10        | 3.3V      | Còi chíp 5V (Qua Transistor) |
| PIN_ARM_LED       | GPIO 3         | 3.3V      | LED đỏ báo trạng thái Arm    |
| PIN_GPS_RX        | GPIO 17 (U1RX) | 3.3V      | Nối chân TXD của GPS ATGM336H|
| PIN_GPS_TX        | GPIO 18 (U1TX) | 3.3V      | Nối chân RXD của GPS ATGM336H|
| PIN_RC_RX         | GPIO 43 (U2RX) | 3.3V      | Bộ thu sóng ELRS / SBUS / RC |
| PIN_RC_TX         | GPIO 44 (U2TX) | 3.3V      | Telemetry gửi về tay cầm RC  |
+-------------------+----------------+-----------+------------------------------+
| ⛔ CẤM SỬ DỤNG   | GPIO 33..37    | -         | Dùng nội bộ cho Octal PSRAM  |
| ⛔ CẤM SỬ DỤNG   | GPIO 19, 20    | -         | Native USB D- / D+ (CDC)     |
+-------------------+----------------+-----------+------------------------------+
```

---

## 🛩️ BỐ TRÍ ĐỘNG CƠ QUAD-X & CHIỀU QUAY

Drone sử dụng cấu hình **Quadcopter chữ X (Quad-X)** tiêu chuẩn:

```text
                  ĐẦU DRONE (MẶT TRƯỚC)
                           ▲
                           │
       (CW - Thuận)                 (CCW - Ngược)
       [ Motor 2 ] ──── 330mm ──── [ Motor 1 ]
       (Trước Trái)                 (Trước Phải)
       GPIO 5 / ESC 2               GPIO 4 / ESC 1
            \                             /
             \                           /
              \                         /
               ─── [ ESP32-S3 FC ] ───
              /                         \
             /                           \
            /                             \
       [ Motor 4 ] ──────────────── [ Motor 3 ]
       (Sau Trái)                   (Sau Phải)
       GPIO 7 / ESC 4               GPIO 6 / ESC 3
      (CCW - Ngược)                  (CW - Thuận)
                           │
                           ▼
                  ĐUÔI DRONE (MẶT SAU)
```

### Quy tắc chiều quay và đảo pha động cơ:
* **Motor 1 (Trước Phải)**: Quay **CCW** (Ngược chiều kim đồng hồ) $\rightarrow$ Dùng cánh quạt thuận 1045 tiêu chuẩn. Nối thẳng 3 dây pha: $U \rightarrow A, V \rightarrow B, W \rightarrow C$.
* **Motor 2 (Trước Trái)**: Quay **CW** (Thuận chiều kim đồng hồ) $\rightarrow$ Dùng cánh quạt ngược 1045R. **Đảo 2 dây pha**: $U \rightarrow B, V \rightarrow A, W \rightarrow C$.
* **Motor 3 (Sau Phải)**: Quay **CW** (Thuận chiều kim đồng hồ) $\rightarrow$ Dùng cánh quạt ngược 1045R. **Đảo 2 dây pha**: $U \rightarrow B, V \rightarrow A, W \rightarrow C$.
* **Motor 4 (Sau Trái)**: Quay **CCW** (Ngược chiều kim đồng hồ) $\rightarrow$ Dùng cánh quạt thuận 1045 tiêu chuẩn. Nối thẳng 3 dây pha: $U \rightarrow A, V \rightarrow B, W \rightarrow C$.

---

## ⚡ HỆ THỐNG TỤ LỌC KÉP & CHỐNG NHIỄU

Để ngăn chặn hiện tượng vi điều khiển bị sập nguồn đột ngột (**Brownout Reset**) và triệt tiêu xung gai điện áp (**Back-EMF**) từ 4 động cơ khi tăng/giảm ga đột ngột, toàn bộ mạch được thiết kế theo quy chuẩn **Hệ thống tụ lọc kép**:

```text
+-----------------------+--------------------+--------------------+----------------------------------------+
| Vị Trí Mắc Tụ         | Loại Tụ & Trị Số   | Điện Áp Chịu Đựng  | Tác Dụng Kỹ Thuật                      |
+-----------------------+--------------------+--------------------+----------------------------------------+
| 1. Cọc Pin PDB Chính  | 1x 1000µF Low-ESR  | ≥ 25V (Khuyên 35V) | Hấp thụ xung Back-EMF từ 4 ESC 30A     |
| 2. Đầu 5V VIN ESP32   | 1x 100µF + 1x 104  | ≥ 10V (Tụ Hóa+Gốm) | Giữ phẳng nguồn nuôi vi điều khiển     |
| 3. Rail 3.3V Cảm Biến | 1x 100µF + 1x 104  | ≥ 6.3V             | Ổn định điện áp nguồn I2C Bus          |
| 4. VCC-GND MPU6050    | 1x 104 (0.1µF)     | ≥ 10V (Tụ Gốm Đĩa) | Triệt nhiễu cao tần Gyroscope/Accel    |
| 5. VCC-GND BMP280     | 1x 104 (0.1µF)     | ≥ 10V (Tụ Gốm Đĩa) | Lọc nguồn áp suất, chống nhảy độ cao   |
| 6. VCC-GND HMC5883L   | 1x 104 (0.1µF)     | ≥ 10V (Tụ Gốm Đĩa) | Lọc nguồn từ kế, chống trôi góc la bàn |
| 7. Cầu Phân Áp VBAT   | 1x 104 (0.1µF)     | ≥ 10V (Tụ Gốm Đĩa) | Song song trở 2.2kΩ lọc phẳng áp ADC   |
| 8. Đầu vào 4 ESC      | 4x 100µF / 25V     | ≥ 25V (Tụ Hóa)     | Ổn định dòng tức thời từng động cơ     |
+-----------------------+--------------------+--------------------+----------------------------------------+
```

> 💡 **Quy tắc phân cực Tụ Hóa**: Chân có vạch sọc xám ghi dấu âm (`-`) bắt buộc hàn vào **GND (Mass)**; chân còn lại (`+`) hàn vào cực dương $5\text{V}$ hoặc $3.3\text{V}$. Tụ gốm 104 không phân cực.

---

## 🛠️ CÀI ĐẶT MÔI TRƯỜNG & NẠP FIRMWARE

### 1. Yêu cầu phần mềm
1. Cài đặt **Visual Studio Code (VS Code)**.
2. Cài đặt extension **PlatformIO IDE** trong VS Code.
3. Cài đặt Driver USB-to-UART (CP210x / CH340 / ESP32-S3 USB CDC).

### 2. Biên dịch và nạp Firmware
Mở Terminal trong thư mục dự án và thực thi:

```bash
# 1. Biên dịch toàn bộ Firmware chính
pio run -e esp32s3

# 2. Nạp code vào ESP32-S3 qua cổng COM
pio run -e esp32s3 -t upload

# 3. Mở Serial Monitor để quan sát log khởi động (Baud 115200)
pio device monitor -b 115200
```

*(Hoặc sử dụng các nút bấm tiện ích `✓ Build`, `→ Upload`, `🔌 Monitor` ở thanh trạng thái dưới cùng của VS Code).*

---

## 🧪 QUY TRÌNH KIỂM TRA PHẦN CỨNG TRÊN BÀN TEST (BENCH TEST)

⚠️ **CẢNH BÁO AN TOÀN SỐNG CÒN**:
> **TUYỆT ĐỐI THÁO HẾT 4 CÁNH QUẠT KHI THỬ NGHIỆM TRÊN BÀN TEST!**

### Bước 1: Quét kiểm tra toàn bộ địa chỉ I2C
Biên dịch và chạy tool I2C Scanner để xác nhận vi điều khiển nhận diện đầy đủ các module:
```bash
pio run -e i2c_scanner -t upload
pio device monitor -b 115200
```
**Bảng kết quả chuẩn cần đạt:**
* `0x68`: IMU MPU6050
* `0x1E` (hoặc `0x0D`): Magnetometer HMC5883L / QMC5883L
* `0x76` (hoặc `0x77`): Barometer BMP280
* `0x40`: PWM Expander PCA9685 (nếu có gắn)

### Bước 2: Kiểm tra chiều quay từng động cơ qua lệnh CLI
Nạp lại firmware chính `esp32s3`. Mở Serial Monitor hoặc Web Tuner, gửi lệnh test từng motor ở mức ga thấp ($10\% - 15\%$):
```text
TEST_MOTOR M1 12   -> Kiểm tra Động cơ 1 quay NGƯỢC chiều kim đồng hồ (CCW)
TEST_MOTOR M2 12   -> Kiểm tra Động cơ 2 quay THUẬN chiều kim đồng hồ (CW)
TEST_MOTOR M3 12   -> Kiểm tra Động cơ 3 quay THUẬN chiều kim đồng hồ (CW)
TEST_MOTOR M4 12   -> Kiểm tra Động cơ 4 quay NGƯỢC chiều kim đồng hồ (CCW)
```
*Nếu motor nào quay ngược chiều quy định, ngắt pin và **đảo chéo 2 trong 3 dây pha** giữa ESC và Motor đó.*

---

## 💻 HƯỚNG DẪN SỬ DỤNG GCS WEB TUNER

Giao diện **GCS Web Tuner** nằm tại `tools/tuner/index.html`, sử dụng công nghệ Web Serial API của trình duyệt:

```text
tools/tuner/index.html
```

### Các bước kết nối và sử dụng:
1. Mở file `tools/tuner/index.html` bằng **Google Chrome** hoặc **Microsoft Edge**.
2. Cắm cáp USB kết nối ESP32-S3 với máy tính.
3. Nhấn nút **🔌 KẾT NỐI SERIAL**, chọn cổng COM của ESP32-S3 (Baudrate `115200`) và bấm **Connect**.
4. **Quan sát dữ liệu Realtime**:
   * **Giao diện 3D**: Mô hình Quadcopter 3D chuyển động trực quan theo góc Roll, Pitch, Yaw thực tế của Drone.
   * **Đồ thị sóng 60fps**: Theo dõi đường phản hồi góc, vận tốc góc Gyroscope, công suất xung 4 motor ($M_1, M_2, M_3, M_4$) và độ cao áp kế.
   * **Thanh trượt Tune PID Realtime**: Kéo thanh trượt $K_p, K_i, K_d$ hoặc nhập số trực tiếp $\rightarrow$ Nhấn **Lưu PID** để nạp trực tiếp vào RAM vi điều khiển mà không cần reset hay nạp lại code.

---

## 📟 TẬP LỆNH ĐIỀU KHIỂN SERIAL CLI

Giao thức điều khiển qua cổng Serial với tốc độ baud `115200`. Có thể gửi trực tiếp từ PlatformIO Serial Monitor, Arduino Serial Monitor, hoặc GCS Tuner:

| Lệnh Serial | Cú pháp mẫu | Ý nghĩa & Hành vi hệ thống |
| :--- | :--- | :--- |
| `ARM` | `ARM` | Kích hoạt hệ thống động cơ sẵn sàng bay (Yêu cầu Throttle = 0, góc nghiêng < 10°) |
| `DISARM` | `DISARM` | Ngắt toàn bộ 4 động cơ ngay lập tức, reset bộ tích phân PID |
| `SET_MODE` | `SET_MODE ANGLE` / `SET_MODE RATE` | Chuyển đổi chế độ bay: Tự cân bằng (Angle) hoặc Nhào lộn (Acro/Rate) |
| `SET_PID` | `SET_PID ROLL_RATE 1.20 0.05 0.015` | Cập nhật bộ 3 hệ số $K_p, K_i, K_d$ cho vòng Rate Roll |
| `SET_PID` | `SET_PID PITCH_RATE 1.20 0.05 0.015`| Cập nhật bộ 3 hệ số $K_p, K_i, K_d$ cho vòng Rate Pitch |
| `SET_PID` | `SET_PID YAW_RATE 2.50 0.10 0.000` | Cập nhật hệ số PID cho trục Yaw |
| `SET_PID` | `SET_PID ROLL_ANGLE 4.50 0.00 0.00` | Cập nhật $K_p$ cho vòng ngoài góc nghiêng Roll |
| `SET_PID` | `SET_PID PITCH_ANGLE 4.50 0.00 0.00`| Cập nhật $K_p$ cho vòng ngoài góc nghiêng Pitch |
| `SET_ATT` | `SET_ATT 0.0 5.0 0.0 25.0` | Gửi lệnh bay: Roll=0°, Pitch=5°, Yaw=0°, Throttle=25% |
| `TEST_MOTOR` | `TEST_MOTOR M1 15` | Chạy thử riêng Motor 1 ở mức ga 15% trong 3 giây |
| `CALIB` | `CALIB GYRO` | Lấy 500 mẫu tĩnh hiệu chuẩn điểm 0 của Con quay hồi chuyển (Yêu cầu đặt yên drone) |
| `CALIB` | `CALIB MAG` | Bắt đầu quy trình hiệu chuẩn từ trường xoay 3D la bàn số |
| `CALIB` | `CALIB BARO` | Lấy mẫu áp suất khí quyển tại mặt đất làm mốc độ cao $0.0\text{m}$ |
| `EMERGENCY` | `EMERGENCY` | **DỪNG KHẨN CẤP**: Cắt xung PWM toàn bộ motor lập tức |

---

## 🎯 CẨM NANG TUNE PID KÉP (CASCADE PID TUNING GUIDE)

Hệ thống điều khiển drone sử dụng cấu trúc **Cascade 2-Loop**:
```text
Lệnh Góc (Roll/Pitch deg) ──► [ Angle PID (Vòng ngoài) ] ──► Tốc độ góc mục tiêu (deg/s)
                                                                     │
Đo Góc Mahony AHRS ──────────────────────────────────────────────────┘
                                                                     ▼
                                                          [ Rate PID (Vòng trong) ] ──► Motor Mixer ──► ESC / Motor
                                                                     ▲
Tốc độ đo MPU6050 Gyro (deg/s) ──────────────────────────────────────┘
```

### Quy trình Tune chuẩn 5 bước:

#### Bước 1: Tune Vận Tốc Góc Roll Rate & Pitch Rate ($K_p$)
1. Chọn chế độ bay `RATE` (Acro Mode). Đặt $K_i = 0, K_d = 0$.
2. Tăng dần $K_p$ của Rate PID từ $0.4 \rightarrow 0.8 \rightarrow 1.2$.
3. Cầm drone trên tay (đeo găng tay bảo hộ) hoặc treo khung dây thử nghiệm, lắc nhẹ thân drone:
   * Nếu drone **chống lại lực lắc yếu ớt** $\rightarrow$ Thiếu $K_p$, tiếp tục tăng.
   * Nếu drone **rung bần bật tần số cao (Fast Oscillation)** $\rightarrow$ Dư $K_p$, giảm bớt $20\%$.

#### Bước 2: Thêm Vi Phân Giảm Chấn Rate $K_d$
1. Bắt đầu tăng $K_d$ từ $0.005 \rightarrow 0.010 \rightarrow 0.018$.
2. Thành phần $K_d$ giúp dập tắt dao động và triệt tiêu quán tính khi drone dừng xoay.
3. *Lưu ý*: Không tăng $K_d$ quá cao vì sẽ làm nóng động cơ do khuếch đại nhiễu rung cơ khí.

#### Bước 3: Thêm Tích Phân Giữ Vận Tốc Rate $K_i$
1. Tăng dần $K_i$ từ $0.02 \rightarrow 0.05 \rightarrow 0.08$.
2. $K_i$ giúp drone duy trì góc bay ổn định khi bị gió thổi lệch hoặc trọng tâm không nằm chính giữa.

#### Bước 4: Tune Vòng Ngoài Điều Khiển Góc (Angle Loop $K_p$)
1. Chuyển sang chế độ `ANGLE` (Tự cân bằng).
2. Tăng $K_p$ của Roll Angle & Pitch Angle từ $2.5 \rightarrow 4.0 \rightarrow 5.0$.
3. Khi nghiêng drone rồi thả tay ra, drone phải **bật trở lại vị trí nằm ngang nhanh chóng và dứt khoát** mà không bị lắc lư qua lại quá 1 chu kỳ.

#### Bước 5: Tune Trục Xoay Yaw Rate
1. Đặt Angle PID cho Yaw = 0 (Trục Yaw điều khiển trực tiếp Rate).
2. Tăng $K_p$ Yaw Rate từ $1.5 \rightarrow 2.8$ và $K_i$ từ $0.05 \rightarrow 0.15$. Thông thường không cần dùng $K_d$ cho trục Yaw ($K_d = 0$).

---

## 📊 BẢNG THÔNG SỐ PID KHỞI ĐIỂM

Dành cho khung Quadcopter 330–450mm, động cơ A2212 1000KV, cánh quạt 1045 và pin 3S:

| Vòng Điều Khiển (Loop) | $K_p$ (Proportional) | $K_i$ (Integral) | $K_d$ (Derivative) | Giới Hạn Tích Phân ($I_{\text{max}}$) | Ngõ Ra Max |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Roll Rate (Trục Lăn)** | `1.200` | `0.050` | `0.015` | `200.0` | `300.0` |
| **Pitch Rate (Trục Chúi)** | `1.200` | `0.050` | `0.015` | `200.0` | `300.0` |
| **Yaw Rate (Trục Xoay)** | `2.500` | `0.100` | `0.000` | `200.0` | `300.0` |
| **Roll Angle (Góc Lăn)** | `4.500` | `0.000` | `0.000` | `0.0` | `250.0 deg/s` |
| **Pitch Angle (Góc Chúi)**| `4.500` | `0.000` | `0.000` | `0.0` | `250.0 deg/s` |

---

## 📚 TÀI LIỆU CHI TIẾT ĐI KÈM

Để tra cứu chuyên sâu từng phần, vui lòng tham khảo các tài liệu trong thư mục `docs/`:

1. **[docs/CIRCUIT_DIAGRAM.html](docs/CIRCUIT_DIAGRAM.html)**:
   * Sơ đồ mạch điện đồ họa SVG trực quan 3D, tích hợp bộ lọc phân lớp dây thông minh và hệ thống hiển thị tụ điện nhấp nháy tương tác.
2. **[docs/WIRING_SCHEMATIC.md](docs/WIRING_SCHEMATIC.md)**:
   * Bảng kê chi tiết 50 đường dây đấu nối vật lý, quy chuẩn tiết diện dây AWG, phân cực nguồn và các lưu ý an toàn phần cứng.
3. **[docs/USER_GUIDE_AND_TUNING.md](docs/USER_GUIDE_AND_TUNING.md)**:
   * Cẩm nang toàn tập hướng dẫn lập trình, giải thích chi tiết thuật toán Mahony AHRS, quy chuẩn mã lỗi và xử lý sự cố bay thực tế.

---

<p align="center">
  <b>Chúc bạn chế tạo và thử nghiệm chuyến bay đầu tiên thành công & an toàn! 🚁✨</b>
</p>
