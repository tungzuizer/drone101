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
2. [Nguyên Lý Hoạt Động Của Code (Dễ Hiểu Cho Người Mới)](#-nguyên-lý-hoạt-động-của-code-dễ-hiểu-cho-người-mới)
3. [Kiến Trúc FreeRTOS Dual-Core & Vòng Lặp 250Hz](#-kiến-trúc-freertos-dual-core--vòng-lặp-250hz)
4. [Phân Công Nhiệm Vụ Từng File Mã Nguồn](#-phân-công-nhiệm-vụ-từng-file-mã-nguồn)
5. [Phần Cứng & Sơ Đồ Chân GPIO](#-phần-cứng--sơ-đồ-chân-gpio)
6. [Bố Trí Động Cơ Quad-X & Chiều Quay](#-bố-trí-động-cơ-quad-x--chiều-quay)
7. [Hệ Thống Tụ Lọc Kép & Chống Nhiễu](#-hệ-thống-tụ-lọc-kép--chống-nhiễu)
8. [Cài Đặt Môi Trường & Nạp Firmware](#-cài-đặt-môi-trường--nạp-firmware)
9. [Hướng Dẫn Tùy Chỉnh Code Trong config.h](#-hướng-dẫn-tùy-chỉnh-code-trong-configh)
10. [Quy Trình 5 Bước Thử Nghiệm Từ Bàn Test Đến Cất Cánh](#-quy-trình-5-bước-thử-nghiệm-từ-bàn-test-đến-cất-cánh)
11. [📱 Hướng Dẫn Điều Khiển Bằng Điện Thoại (Web Cockpit)](#-hướng-dẫn-điều-khiển-bằng-điện-thoại-smartphone-web-cockpit)
12. [💻 Hướng Dẫn Sử Dụng GCS Web Tuner](#-hướng-dẫn-sử-dụng-gcs-web-tuner)
13. [📟 Tập Lệnh Điều Khiển Serial CLI](#-tập-lệnh-điều-khiển-serial-cli)
14. [🎯 Cẩm Nang Tune PID Kép (Cascade PID Tuning Guide)](#-cẩm-nang-tune-pid-kép-cascade-pid-tuning-guide)
15. [📊 Bảng Thông Số PID Khởi Điểm Khuyến Nghị](#-bảng-thông-số-pid-khởi-điểm-khuyến-nghị)
16. [📚 Tài Liệu Chi Tiết Đi Kèm](#-tài-liệu-chi-tiết-đi-kèm)

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

## 🧠 NGUYÊN LÝ HOẠT ĐỘNG CỦA CODE (DỄ HIỂU CHO NGƯỜI MỚI)

Để một chiếc Drone có thể tự giữ thăng bằng trên không, hệ thống firmware hoạt động liên tục theo chu trình khép kín sau:

```text
┌──────────────────────────────────────────────────────────────────────────────────┐
│                         CHU TRÌNH CÂN BẰNG TỰ ĐỘNG                               │
└──────────────────────────────────────────────────────────────────────────────────┘

 [ CẢM BIẾN ] ──► MPU6050 đo gia tốc rung & tốc độ xoay (250 lần / giây)
       │
       ▼
 [ NÃO BỘ ] ──► Thuật toán Mahony AHRS tính ra góc nghiêng thật 3D: Roll, Pitch, Yaw
       │
       ▼
 [ SO SÁNH ] ──► So sánh: [Góc người lái muốn] vs [Góc máy bay đang nghiêng thật]
       │         ──► Tạo ra "Sai số" (Error)
       ▼
 [ BỘ PID ] ──► Tính toán lực cần bù trừ để đưa sai số về 0 (PID kép 2 vòng)
       │
       ▼
 [ MIXER ] ──► Bộ chia lực Quad-X cộng/trừ xung cho từng cánh (M1, M2, M3, M4)
       │
       ▼
 [ ĐỘNG CƠ ] ──► 4 ESC & Động cơ điều chỉnh tốc độ tức thì ──► Drone thăng bằng!
```

---

## ⏱️ KIẾN TRÚC FREERTOS DUAL-CORE & VÒNG LẶP 250HZ

Hệ thống tận dụng tối đa sức mạnh 2 nhân xử lý độc lập của vi điều khiển **ESP32-S3 (240MHz)** với kiến trúc bất đối xứng (Asymmetric Task Distribution):

```text
┌────────────────────────────────────────────────────────────────────────────────────┐
│              KIẾN TRÚC PHÂN CHIA 2 CORE FREERTOS TRÊN ESP32-S3                     │
└────────────────────────────────────────────────────────────────────────────────────┘

 ╔═══════════════════════════════════════╗      ╔═══════════════════════════════════════╗
 ║   CORE 0: NETWORKING & COMMS TASK     ║      ║   CORE 1: REAL-TIME FLIGHT TASK       ║
 ║         (Mức ưu tiên: Priority 1)     ║      ║        (Mức ưu tiên: Priority 24)     ║
 ╠═══════════════════════════════════════╣      ╠═══════════════════════════════════════╣
 ║ • Wi-Fi Soft-AP Server (ESP32-DRONE)  ║      ║ • 250Hz Fast Loop (Mỗi 4000µs)        ║
 ║ • WebServer (Cung cấp Web Cockpit)    ║      ║ • Đọc IMU MPU6050 (I2C 400kHz)        ║
 ║ • WebSockets Daemon (Uplink 40Hz)     ║      ║ • Mahony AHRS 9-DOF Attitude Filter   ║
 ║ • Broadcast Telemetry $T,... (25Hz)   ║      ║ • Cascade Dual-Loop PID Controller    ║
 ║ • Xử lý kết nối Captive Portal        ║      ║ • Motor Mixer Quad-X & PWM ESCs       ║
 ╚═══════════════════════════════════════╝      ╚═══════════════════════════════════════╝
                    │                                              ▲
                    ▼                                              │
 ┌────────────────────────────────────────────────────────────────────────────────────┐
 │  LOCK-FREE DOUBLE BUFFER MAILBOX (std::atomic<uint8_t> swap index — 0 lock latency)│
 └────────────────────────────────────────────────────────────────────────────────────┘
```

### 4 Tầng Thời Gian Thực Trên Core 1 (Multi-Rate Loop):
```text
+------------------------------------------------------------------------------------+
| 1. FAST LOOP (250Hz - Mỗi 4.0ms) ── VÒNG SINH TỬ ĐIỀU KHIỂN CÂN BẰNG               |
|    • Đọc dữ liệu MPU6050 (Gyro + Accel).                                          |
|    • Chạy bộ lọc Mahony AHRS ước lượng góc Roll, Pitch, Yaw.                      |
|    • Đọc snapshot lệnh từ điện thoại / RC (Lock-free atomic read < 0.1µs).        |
|    • Kiểm tra an toàn Failsafe (Góc nghiêng < 45°, mất sóng < 500ms).              |
|    • Chạy thuật toán Cascade PID & Bơm xung PWM ra 4 ESC động cơ.                 |
|    • Đẩy Telemetry vào Mailbox cho Core 0 phát về điện thoại.                     |
+------------------------------------------------------------------------------------+
| 2. MEDIUM LOOP (50Hz - Mỗi 20ms) ── VÒNG CẢM BIẾN PHỤ & LỆNH ĐIỀU KHIỂN             |
|    • Đọc Từ kế HMC5883L / QMC5883L (La bàn số định hướng Bắc).                    |
|    • Đọc Áp kế BMP280 (Tính toán độ cao khí áp).                                  |
|    • Nhận & Xử lý lệnh Tune PID từ máy tính gửi xuống.                            |
+------------------------------------------------------------------------------------+
| 3. SLOW LOOP (10Hz - Mỗi 100ms) ── VÒNG TRUYỀN DỮ LIỆU & GPS                       |
|    • Đọc gói tin tọa độ NMEA từ module GPS ATGM336H.                              |
|    • Đóng gói chuỗi Telemetry $TEL,... gửi về máy tính vẽ đồ thị 60fps.           |
+------------------------------------------------------------------------------------+
| 4. HEARTBEAT (1Hz - Mỗi 1.0s) ── NHỊP TIM HỆ THỐNG                                 |
|    • Nhấp nháy đèn LED trạng thái báo hệ thống đang hoạt động bình thường.        |
+------------------------------------------------------------------------------------+
```

---

## 📂 PHÂN CÔNG NHIỆM VỤ TỪNG FILE MÃ NGUỒN

| File Mã Nguồn | Vai Trò & Chức Năng Cụ Thể |
| :--- | :--- |
| **`src/main.cpp`** | **Tổng chỉ huy**: Quản lý vòng lặp bay 250Hz trên Core 1, khởi động FreeRTOS Comms Task trên Core 0, trọng tài nguồn điều khiển và đẩy Telemetry. |
| **`src/config.h`** | **Bảng cài đặt**: Nơi tập trung cấu hình chân GPIO, Wi-Fi Soft-AP, tần số vòng lặp, PID mặc định và giới hạn an toàn. |
| **`src/control/WifiControlInput.cpp`**| **Cầu nối Wi-Fi điện thoại**: Quản lý Soft-AP, WebServer, WebSockets 40Hz, kiến trúc Lock-Free Double Buffer trao đổi liên Core. |
| **`src/control/WebCockpitHtml.h`**| **Web Cockpit Nhúng (PROGMEM)**: Giao diện lái máy bay cảm ứng HTML5/JS/Canvas, Artificial Horizon 3D, thanh trượt Slide-to-Arm và nút Dừng khẩn cấp. |
| **`src/sensors/ImuSensor.cpp`** | **Mắt thần**: Giao tiếp MPU6050 qua I2C 400kHz, kích hoạt lọc số DLPF 98Hz và tự động trừ trôi điểm 0 (Gyro Bias). |
| **`src/sensors/Magnetometer.cpp`** | **La bàn**: Tự động nhận diện chip thật HMC5883L (0x1E) hay chip clone QMC5883L (0x0D), xoay 3D calib từ trường. |
| **`src/sensors/Barometer.cpp`** | **Đo độ cao**: Đọc BMP280, tự động lấy mẫu áp suất mặt đất làm mốc $0.0\text{m}$ khi khởi động. |
| **`src/sensors/GpsReader.cpp`** | **Định vị**: Đọc NMEA 0183 qua UART1 từ ATGM336H, phân tích kinh độ, vĩ độ, số lượng vệ tinh. |
| **`src/control/AttitudeEstimator.cpp`** | **Não bộ không gian**: Thuật toán Mahony AHRS khử trôi góc nghiêng bằng đại số Quaternion. |
| **`src/control/PidController.cpp`** | **Thuật toán PID**: Tính sai số $P, I, D$, tích hợp chống bão hòa tích phân (Anti-Windup) và lọc rung D-term LPF. |
| **`src/control/MotorMixer.cpp`** | **Bộ chia lực**: Tính toán ma trận hòa trộn cho Quadcopter khung X ($M_1, M_2, M_3, M_4$). |
| **`src/safety/FailsafeManager.cpp`** | **Bảo vệ an toàn**: Tự động ngắt motor khi mất sóng >500ms, góc nghiêng lật >45° hoặc kẹt I2C. |
| **`src/control/SerialControlInput.cpp`**| **Giao tiếp Serial**: Phân tích cú pháp tập lệnh CLI và gửi gói tin trạng thái cho GCS Web Tuner. |
| **`src/actuators/MotorController.cpp`** | **Cơ cấu chấp hành**: Phát xung PWM 50–400Hz (1000–2000µs) qua PCA9685 hoặc ESP32 hardware LEDC. |

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

## 🛠️ HƯỚNG DẪN TÙY CHỈNH CODE TRONG CONFIG.H

Mọi tùy chỉnh phần cứng và an toàn được tập trung tại file **`src/config.h`**. Khi cần thay đổi, bạn chỉ cần sửa giá trị tương ứng:

```cpp
// 1. Muốn đổi chân I2C cảm biến:
#define PIN_I2C_SDA         8       // Đổi chân Data SDA nếu hàn sang GPIO khác
#define PIN_I2C_SCL         9       // Đổi chân Clock SCL

// 2. Muốn đổi chân xuất xung PWM cho 4 ESC:
#define PIN_MOTOR_1         4       // ESC 1 (Trước Phải)
#define PIN_MOTOR_2         5       // ESC 2 (Trước Trái)
#define PIN_MOTOR_3         6       // ESC 3 (Sau Phải)
#define PIN_MOTOR_4         7       // ESC 4 (Sau Trái)

// 3. Muốn giới hạn ga an toàn khi mới tập bay:
#define MAX_TEST_THROTTLE_PERCENT 30    // Giới hạn ga tối đa trên bàn test là 30%

// 4. Muốn đổi góc nghiêng tự ngắt khẩn cấp (Emergency Tilt Cutoff):
#define MAX_TILT_ANGLE_DEG      45.0f   // Nghiêng quá 45 độ sẽ tự động tắt motor ngay lập tức
```

---

## 🚀 QUY TRÌNH 5 BƯỚC THỬ NGHIỆM TỪ BÀN TEST ĐẾN CẤT CÁNH

⚠️ **NGUYÊN TẮC AN TOÀN SỐNG CÒN**:
> **TUYỆT ĐỐI KHÔNG GẮN CÁNH QUẠT Ở BƯỚC 1, 2, 3 VÀ 4!**

```text
[ BƯỚC 1: QUÉT I2C ] ──► Nạp tool `i2c_scanner` kiểm tra nhận đủ 0x68, 0x1E, 0x76.
          │
          ▼
[ BƯỚC 2: CALIB TĨNH ] ──► Đặt drone trên mặt phẳng tĩnh, gửi lệnh `CALIB GYRO`.
          │
          ▼
[ BƯỚC 3: TEST MOTOR ] ──► Gửi lệnh `TEST_MOTOR M1 12` kiểm tra đúng chiều quay CW/CCW.
          │
          ▼
[ BƯỚC 4: TUNE TRÊN TAY ] ──► Cầm thân drone (hoặc treo dây), tăng ga 15-20%, tune Roll/Pitch Kp.
          │
          ▼
[ BƯỚC 5: GẮN CÁNH & BAY ] ──► Ra bãi cỏ rộng thoáng, gắn cánh 1045, Arm và bay thử nghiệm!
```

### Chi tiết từng bước thực hiện:

#### Bước 1: Quét kiểm tra toàn bộ địa chỉ I2C
Biên dịch và chạy tool I2C Scanner:
```bash
pio run -e i2c_scanner -t upload
pio device monitor -b 115200
```
*Kết quả chuẩn cần đạt:* `0x68` (MPU6050), `0x1E` hoặc `0x0D` (La bàn), `0x76` (BMP280).

#### Bước 2: Kiểm tra chiều quay 4 động cơ
Nạp lại firmware chính `esp32s3`. Mở Serial Monitor hoặc Web Tuner, gửi lệnh test từng motor ở mức ga thấp ($10\% - 15\%$):
```text
TEST_MOTOR M1 12   -> Motor 1 (Trước Phải) phải quay NGƯỢC chiều kim đồng hồ (CCW)
TEST_MOTOR M2 12   -> Motor 2 (Trước Trái) phải quay THUẬN chiều kim đồng hồ (CW)
TEST_MOTOR M3 12   -> Motor 3 (Sau Phải) phải quay THUẬN chiều kim đồng hồ (CW)
TEST_MOTOR M4 12   -> Motor 4 (Sau Trái) phải quay NGƯỢC chiều kim đồng hồ (CCW)
```
*Nếu motor nào quay ngược, ngắt pin và **đảo chéo 2 trong 3 dây pha** giữa ESC và Motor đó.*

---

## 📱 HƯỚNG DẪN ĐIỀU KHIỂN BẰNG ĐIỆN THOẠI (SMARTPHONE WEB COCKPIT)

Hệ thống tích hợp sẵn một **Web Cockpit máy bay chuyên nghiệp** lưu trong bộ nhớ Flash của ESP32-S3 (`PROGMEM`), không cần cài đặt App từ App Store hay CH Play, hoạt động độc lập ngay cả khi không có mạng Internet.

```text
┌────────────────────────────────────────────────────────────────────────────────────┐
│                    GIAO DIỆN ĐIỀU KHIỂN WEB COCKPIT TRÊN ĐIỆN THOẠI                │
└────────────────────────────────────────────────────────────────────────────────────┘

  [ 📡 WI-FI: CONNECTED (25Hz) ]   [ 🔋 PIN: 12.0V ]   [ 🛑 DỪNG KHẨN CẤP (KILL) ]
 ────────────────────────────────────────────────────────────────────────────────────
  [ CẦN TRÁI: GA + XOAY ]         [ CHÂN TRỜI NHÂN TẠO ]       [ CẦN PHẢI: LĂN + CHÚI ]
      ▲ Ga (Throttle)                   (3D Artificial                 ▲ Chúi (Pitch)
      │                                    Horizon)                    │
  ◄───┼───► Xoay (Yaw)                                             ◄───┼───► Lăn (Roll)
      │                                                                │
      ▼                                                                ▼
 ────────────────────────────────────────────────────────────────────────────────────
  [ CHỌN GA: 30% / 60% / 100% ]   [ 🔴 SLIDE TO ARM ──────► ]   [ MỐC ĐỘNG CƠ: M1-M4 ]
```

### 1. Cách kết nối và mở buồng lái trên điện thoại:
1. **Bật nguồn Drone**: Đợi 3 giây để ESP32-S3 phát mạng Wi-Fi Soft-AP.
2. **Kết nối Wi-Fi trên điện thoại**:
   * **Tên Wi-Fi (SSID)**: `ESP32-DRONE-FC`
   * **Mật khẩu**: `12345678`
3. **Mở giao diện Cockpit**:
   * **Tự động (Captive Portal)**: Sau khi kết nối Wi-Fi, điện thoại sẽ tự động mở màn hình đăng nhập hiển thị ngay Web Cockpit.
   * **Thủ công**: Mở trình duyệt (Safari / Chrome), truy cập địa chỉ IP: **`http://192.168.4.1`**

### 2. Bố trí điều khiển 2 cần ảo (Mode 2 Chuẩn Quốc Tế):
* **Cần bên TRÁI**:
  * **Trục dọc (Lên/Xuống)**: Điều chỉnh ga (**Throttle** $0\% - 100\%$). Cần có cơ chế giữ vị trí ga (không tự bật về giữa).
  * **Trục ngang (Trái/Phải)**: Điều chỉnh quay đầu hướng mũi drone (**Yaw** $-180^\circ/\text{s} \rightarrow +180^\circ/\text{s}$). Tự động trả về tâm $0$ khi thả tay.
* **Cần bên PHẢI**:
  * **Trục dọc (Lên/Xuống)**: Nghiêng tới trước / ngửa ra sau (**Pitch** $-45^\circ \rightarrow +45^\circ$). Tự động trả về tâm $0$ khi thả tay.
  * **Trục ngang (Trái/Phải)**: Nghiêng lượn sang trái / sang phải (**Roll** $-45^\circ \rightarrow +45^\circ$). Tự động trả về tâm $0$ khi thả tay.

### 3. Quy chuẩn an toàn khi bay bằng điện thoại:
* **Thanh trượt mở khóa (Slide-to-Arm)**: Để bật động cơ, người lái phải gạt thanh trượt từ trái sang phải. Hệ thống chỉ cho phép Arm khi **Cần Ga đang ở mức 0%** và **Góc nghiêng máy bay < 10°**.
* **Nút Dừng Khẩn Cấp (Emergency Kill Switch)**: Nút màu đỏ ở góc trên bên phải. Nhấn vào sẽ lập tức cắt xung 4 motor về 0 trong $0.001\text{s}$ bất kể tình huống.
* **Giới hạn mức ga 3 cấp độ (Flight Level)**:
  * `BEG (30%)`: Dành cho người mới tập bay trong phòng, trần ga khóa cứng ở mức tối đa 30%.
  * `SPT (60%)`: Dành cho bay ngoài trời với gió nhẹ, trần ga 60%.
  * `PRO (100%)`: Toàn bộ 100% công suất động cơ.
* **Cơ chế chống chạm nhầm**:
  * Nếu điện thoại bị **tắt màn hình**, **chuyển tab trình duyệt**, hoặc **có cuộc gọi đến**, hệ thống Web Cockpit tự động gửi lệnh ngắt ga và ngắt Arm ngay lập tức qua API `visibilitychange`.
  * Nếu **mất sóng Wi-Fi quá 500ms**, bộ phận Failsafe trên Core 1 của ESP32-S3 sẽ tự động kích hoạt chế độ hạ cánh an toàn (FS_LANDING) hoặc cắt motor.

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

## 🎯 CẨM NANG TUNE PID THỰC CHIẾN (REAL-WORLD PID TUNING GUIDE)

Hệ thống điều khiển drone sử dụng cấu trúc **Cascade 2-Loop (PID 2 Tầng Kép)**:
```text
Lệnh Góc (Roll/Pitch deg) ──► [ Angle PID (Vòng ngoài) ] ──► Tốc độ góc mục tiêu (deg/s)
                                                                     │
Đo Góc Mahony AHRS ──────────────────────────────────────────────────┘
                                                                     ▼
                                                          [ Rate PID (Vòng trong @ 250Hz) ] ──► Motor Mixer ──► ESC / Motor
                                                                     ▲
Tốc độ đo MPU6050 Gyro (deg/s) ──────────────────────────────────────┘
```

---

### 🛡️ 3 Giai Đoạn Thử Nghiệm An Toàn Trước Khi Bay Tự Do

```text
[ GIAI ĐOẠN 1: TEST TRÊN KHUNG 1 TRỤC ] ──► Cố định 1 trục (Pitch), chỉ cho drone nghiêng tự do trục Roll.
                  │
                  ▼
[ GIAI ĐOẠN 2: TEST CẦM TAY BẢO HỘ ]    ──► Cầm chặt chân đế dưới bụng, lên ga 25%, búng cần cảm nhận lực cản.
                  │
                  ▼
[ GIAI ĐOẠN 3: BUỘC DÂY HÃM ĐẤT (TETHER) ] ──► Buộc 4 góc chân đáp xuống đất bằng dây dù dài 0.5m, hover thử.
```

---

### 🛠️ Quy Trình 5 Bước Tune PID Thực Tế Chi Tiết:

#### 🟢 BƯỚC 1: Tìm $K_p$ Vòng Trong (Roll Rate & Pitch Rate) — "Độ Cứng Vững"
1. **Chuẩn bị**: Đặt $K_i = 0$, $K_d = 0$, Angle $K_p = 0$ (hoặc bay ở chế độ `ACRO/RATE`).
2. **Khởi điểm**: Đặt $K_p = 0.60$.
3. **Thực hiện**: Lên ga vừa đủ để drone bắt đầu nâng mình khỏi mặt đất (~25% - 30% ga).
4. **Tăng dần $K_p$**: Tăng mỗi lần `+0.15` ($0.60 \rightarrow 0.75 \rightarrow 0.90 \rightarrow 1.05 \rightarrow 1.20 \rightarrow 1.35...$):
   * *Nếu thấy drone phản hồi lờ đờ, bị lật nghiêng trôi dạt* $\rightarrow$ **Thiếu $K_p$**, tiếp tục tăng.
   * *Nếu thấy drone bắt đầu xuất hiện **rung dao động nhanh tần số cao (Fast Oscillation 15-30Hz)** phát ra tiếng rít "zizz zizz"* $\rightarrow$ **Dư $K_p$**.
5. **Điểm chốt $K_p$ chuẩn**: Lấy giá trị tại điểm bắt đầu rung **nhân với $0.70$** (giảm bớt $30\%$).
   * *Ví dụ: Rung tại $K_p = 1.60 \rightarrow$ Chốt $K_p = 1.60 \times 0.70 = 1.12 \approx 1.20$.*

---

#### 🟢 BƯỚC 2: Thêm $K_d$ Vòng Trong — "Phanh Hãm Quán Tính & Dập Tắt Rung Lắc"
1. **Khởi điểm**: Đặt $K_d = 0.015$.
2. **Tăng dần $K_d$**: Tăng mỗi lần `+0.005` ($0.015 \rightarrow 0.020 \rightarrow 0.025 \rightarrow 0.030 \rightarrow 0.035$):
   * Búng nhẹ cần gạt Roll/Pitch rồi thả tay về giữa:
   * *Nếu drone dừng lại lập tức, không bị nảy giật lại (No Bounce-back)* $\rightarrow$ **$K_d$ đạt chuẩn**.
3. ⚠️ **KIỂM TRA NHIỆT ĐỘ MOTOR (BẮT BUỘC)**:
   * Sau mỗi lần thử 45 giây, đáp drone, ngắt ARM và **dùng tay chạm trực tiếp vào vỏ 4 motor**:
   * *Motor ấm nhẹ ($< 45^\circ\text{C}$)* $\rightarrow$ Hoàn hảo.
   * *Motor nóng rát tay ($> 60^\circ\text{C}$)* $\rightarrow$ **$K_d$ quá cao** (khuếch đại nhiễu rung cơ học làm ESC nhồi xung liên tục) $\rightarrow$ **Phải giảm ngay $K_d$ xuống 30%** và kiểm tra đệm cao su giảm chấn của MPU6050.

---

#### 🟢 BƯỚC 3: Thêm $K_i$ Vòng Trong — "Khóa Góc Cố Định & Chống Gió Tạt"
1. **Khởi điểm**: Đặt $K_i = 0.02$.
2. **Tăng dần $K_i$**: Tăng mỗi lần `+0.01` ($0.02 \rightarrow 0.03 \rightarrow 0.04 \rightarrow 0.05$):
   * Nghiêng cần lái tạo góc nghiêng $15^\circ$ rồi thả tay:
   * *Nếu drone giữ nguyên tư thế góc đó khi bay thẳng mà không bị trôi từ từ về mặt đất* $\rightarrow$ **$K_i$ đạt chuẩn**.
   * *Nếu thấy drone xuất hiện **dao động nhấp nhô chậm chạp (Slow Wobble 1-2Hz)*** $\rightarrow$ **Dư $K_i$**, giảm bớt $20\%$.

---

#### 🟢 BƯỚC 4: Kích Hoạt $K_p$ Vòng Ngoài (Angle Loop) — "Độ Nhạy Tự Cân Bằng"
1. **Chuyển chế độ bay**: Chọn chế độ **`ANGLE`** trên Web Cockpit / GCS.
2. **Khởi điểm**: Đặt Angle $K_p = 3.0$ (Angle $K_i = 0$, Angle $K_d = 0$).
3. **Tăng dần $K_p$**: Tăng mỗi lần `+0.5` ($3.0 \rightarrow 3.5 \rightarrow 4.0 \rightarrow 4.5 \rightarrow 5.0$):
   * Lấy tay nghiêng drone rồi buông ra: Drone phải **tự động bật thẳng đứng lại vị trí cân bằng ngang trong tích tắc**.
   * *Nếu drone phản hồi trả về chậm chạp* $\rightarrow$ Tăng Angle $K_p$.
   * *Nếu drone tự cân bằng nhưng bị lắc lư qua lại vài nhịp trước khi phẳng* $\rightarrow$ Giảm Angle $K_p$.

---

#### 🟢 BƯỚC 5: Tune Trục Xoay Đầu (Yaw Rate Loop)
1. Trục Yaw không sử dụng vòng ngoài Angle (luôn điều khiển trực tiếp tốc độ xoay deg/s).
2. **Khởi điểm**: Đặt $K_p = 2.0$, $K_i = 0.05$, $K_d = 0.000$.
3. **Tăng dần**: Tăng $K_p$ lên $2.5 - 3.0$ để mũi drone xoay dứt khoát theo cần lái.
4. Tăng $K_i$ lên $0.08 - 0.10$ để giữ hướng mũi cố định, không bị tự xoay đuôi do phản lực cánh quạt (Yaw Drift).
5. *Lưu ý*: Trục Yaw thường **luôn để $K_d = 0$** để tránh làm nóng motor vô ích.

---

### 🩺 Bảng Tra Cứu Chẩn Đoán & "Bắt Bệnh" PID Qua Hiện Tượng Thực Tế

| Hiện Tượng Quan Sát Khi Bay | Nguyên Nhân Kỹ Thuật | Hành Động Khắc Phục Ngay |
| :--- | :--- | :--- |
| **Drone rung bần bật tần số cao (Fast Oscillation)** | $K_p$ Rate quá cao | Giảm $K_p$ Rate trục Roll/Pitch xuống $20\%$. |
| **Motor kêu rít rè rè và nóng bỏng tay sau khi bay** | $K_d$ Rate quá cao hoặc MPU6050 bị rung cơ học | Giảm $K_d$ Rate xuống $0.020 - 0.025$, chèn thêm đệm xốp 3M chống rung dưới chip MPU6050. |
| **Drone gật gù / lắc lư nhấp nhô chậm (1-2Hz)** | $K_p$ Angle quá cao hoặc $K_i$ Rate bị dồn tích phân | Giảm $K_p$ Angle từ $4.5 \rightarrow 3.5$; giảm nhẹ $K_i$ Rate. |
| **Khi nhả cần lái, drone bị nảy ngược lại (Bounce-back)** | Thiếu lực giảm chấn vi phân | Tăng $K_d$ Rate thêm $+0.005$. |
| **Drone tự trôi góc nghiêng sang một bên khi bay** | Lệch trọng tâm (Pin không đều) hoặc thiếu $K_i$ | Đặt pin vào chính giữa trọng tâm khung; tăng $K_i$ Rate từ $0.03 \rightarrow 0.05$. |
| **Rung lắc mạnh khi hạ độ cao thẳng đứng (Prop Wash)** | Drone rơi vào luồng gió xoáy của chính cánh quạt | Tăng nhẹ $K_d$ Rate và đảm bảo bộ lọc LPF D-term $\alpha = 0.70$ đang bật trong code. |

---

## 📊 BẢNG THÔNG SỐ PID KHỞI ĐIỂM CHUẨN

*Áp dụng chuẩn xác cho khung Quad-X 330–450mm, động cơ A2212 1000KV, cánh quạt 1045, ESC 30A và pin LiPo 3S 2200mAh:*

| Bộ Điều Khiển (Loop) | Trục (Axis) | $K_p$ | $K_i$ | $K_d$ | D-Filter $\alpha$ | Giới Hạn Max |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| **Angle PID (Vòng ngoài)** | **Roll Angle** | `4.50` | `0.00` | `0.00` | — | $\pm 300^\circ/\text{s}$ |
| **Angle PID (Vòng ngoài)** | **Pitch Angle** | `4.50` | `0.00` | `0.00` | — | $\pm 300^\circ/\text{s}$ |
| **Rate PID (Vòng trong 250Hz)** | **Roll Rate** | `1.20` | `0.04` | `0.035` | `0.70` (LPF) | $\pm 400\,\mu\text{s}$ |
| **Rate PID (Vòng trong 250Hz)** | **Pitch Rate** | `1.20` | `0.04` | `0.035` | `0.70` (LPF) | $\pm 400\,\mu\text{s}$ |
| **Rate PID (Vòng trong 250Hz)** | **Yaw Rate** | `2.50` | `0.08` | `0.000` | — | $\pm 400\,\mu\text{s}$ |

---

## ❓ KHI CHỈNH SỬA TRÊN WEB CÓ PHẢI NẠP LẠI CODE KHÔNG?

| Thao tác bạn thực hiện | Có cần nạp lại code (`Upload`) không? | Giải thích cơ chế hoạt động |
| :--- | :---: | :--- |
| **Chỉnh thông số PID / Ga / Calib trên Web Tuner** | ❌ **KHÔNG** *(khi đang bay thử)*<br>✅ **CÓ** *(khi muốn lưu vĩnh viễn)* | • Khi kéo thanh trượt trên Web, thông số được nạp thẳng vào **RAM** của ESP32-S3 qua WebSocket/Serial và có hiệu lực ngay lập tức để bay thử mà không cần ngắt kết nối.<br>• Khi ngắt nguồn/rút pin, bộ nhớ RAM sẽ bị reset về mặc định. Sau khi tìm được bộ PID chuẩn ưng ý, hãy mở file `src/control/MotorMixer.cpp` cập nhật lại các giá trị đó rồi nạp code 1 lần duy nhất để lưu vĩnh viễn vào bộ nhớ Flash. |
| **Sửa giao diện Web Cockpit (Nút bấm, màu sắc, HTML/CSS/JS)** | ✅ **BẮT BUỘC CÓ** | Toàn bộ giao diện Web Cockpit trên điện thoại được lưu dưới dạng chuỗi nhúng trong bộ nhớ Flash của ESP32-S3 tại file `src/control/WebCockpitHtml.h` (`PROGMEM`). Mọi thay đổi về nút bấm, bố cục hoặc logic JavaScript đều cần nạp lại firmware (`pio run -t upload`) thì điện thoại mới nhận giao diện mới. |

### 💡 Quy trình làm việc chuẩn (Standard Workflow):
```text
[ Chỉnh số PID trên Web ] ──(Bay thử thấy êm)──► [ Ghi lại số vào MotorMixer.cpp ] ──► [ Nạp code lưu vĩnh viễn ]
[ Sửa nút/Giao diện Web ] ──────────────────────► [ Sửa file WebCockpitHtml.h ]      ──► [ Nạp code vào ESP32 ]
```

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
