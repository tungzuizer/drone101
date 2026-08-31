# HƯỚNG DẪN SỬ DỤNG CODE & CẨM NANG TUNE FLIGHT CONTROLLER DRONE ESP32-S3

Tài liệu này cung cấp toàn bộ quy trình từ cài đặt môi trường, nạp firmware, kiểm tra phần cứng, sử dụng công cụ **GCS Web Tuner**, giao thức lệnh Serial CLI, cho đến cẩm nang chi tiết từng bước **Tune hệ thống PID kép (Cascade PID)** cho Quadcopter ESP32-S3 (R16N8).

---

## 📑 MỤC LỤC
1. [Cấu trúc mã nguồn Firmware](#1-cấu-trúc-mã-nguồn-firmware)
2. [Cài đặt môi trường & Nạp Firmware (PlatformIO)](#2-cài-đặt-môi-trường--nạp-firmware-platformio)
3. [Quy trình kiểm tra phần cứng trên bàn Test (Bench Testing)](#3-quy-trình-kiểm-tra-phần-cứng-trên-bàn-test-bench-testing)
4. [Hướng dẫn sử dụng GCS Web Tuner (Chrome / Edge Web Serial)](#4-hướng-dẫn-sử-dụng-gcs-web-tuner-chrome--edge-web-serial)
5. [Tập lệnh điều khiển Serial CLI & Giao thức Telemetry](#5-tập-lệnh-điều-khiển-serial-cli--giao-thức-telemetry)
6. [Cẩm nang Tune PID chi tiết từng bước (Cascade PID Tuning)](#6-cẩm-nang-tune-pid-chi-tiết-từng-bước-cascade-pid-tuning)
7. [Bảng thông số PID khởi điểm khuyến nghị](#7-bảng-thông-số-pid-khởi-điểm-khuyến-nghị)
8. [Chẩn đoán lỗi & Xử lý sự cố thường gặp (Troubleshooting)](#8-chẩn-đoán-lỗi--xử-lý-sự-cố-thường-gặp-troubleshooting)

---

## 1. CẤU TRÚC MÃ NGUỒN FIRMWARE

Mã nguồn được thiết kế theo kiến trúc module hoá hướng đối tượng (OOP), độc lập phần cứng và không sử dụng hàm `delay()` trong luồng điều khiển:

```text
drone/
├── platformio.ini              # Cấu hình PlatformIO (ESP32-S3 R16N8, thư viện, cờ build)
├── src/
│   ├── main.cpp                # Vòng lặp chính 250Hz non-blocking (Fast/Medium/Slow Loop)
│   ├── config.h                # Khai báo chân GPIO, tần số, giới hạn an toàn, thông số PID mặc định
│   ├── sensors/
│   │   ├── ImuSensor.h/.cpp        # Driver MPU6050 (I2C 400kHz, DLPF 98Hz, hiệu chuẩn Bias)
│   │   ├── Magnetometer.h/.cpp     # Driver HMC5883L / QMC5883L (Tự nhận diện chip thật/clone)
│   │   ├── Barometer.h/.cpp        # Driver BMP280 (Áp suất khí quyển & ước tính độ cao)
│   │   └── GpsReader.h/.cpp        # Driver ATGM336H (UART1 NMEA 0183 9600 baud)
│   ├── actuators/
│   │   └── MotorController.h/.cpp  # Driver PCA9685 PWM 50-400Hz + ESP32 LEDC PWM trực tiếp
│   ├── control/
│   │   ├── AttitudeEstimator.h/.cpp# Thuật toán lọc tư thế Mahony AHRS (kết hợp Gyro, Accel, Mag)
│   │   ├── PidController.h/.cpp    # Bộ điều khiển PID (Anti-windup, D-filter, Derivative-on-measurement)
│   │   ├── MotorMixer.h/.cpp       # Bộ hòa trộn công suất Quad-X (Cascade Angle + Rate PID)
│   │   ├── ControlInputSource.h    # Interface trừu tượng nhận tín hiệu điều khiển
│   │   └── SerialControlInput.h/.cpp# Xử lý lệnh điều khiển qua Serial/USB & GCS Telemetry
│   └── safety/
│       └── FailsafeManager.h/.cpp  # Giám sát an toàn: Mất tín hiệu, quá góc nghiêng >45°, lỗi I2C
├── tools/
│   ├── tuner/
│   │   └── index.html          # Giao diện GCS Tuner Web (Đồ thị Realtime, 3D Attitude, Tune PID trực tiếp)
│   └── i2c_scanner/
│       └── main.cpp            # Tool quét địa chỉ I2C kiểm tra phần cứng ban đầu
└── docs/
    ├── WIRING_SCHEMATIC.md     # Sơ đồ đấu nối 50 đường dây vật lý & hệ thống tụ lọc kép
    ├── CIRCUIT_DIAGRAM.html    # Bản vẽ mạch điện SVG tương tác phân lớp
    └── USER_GUIDE_AND_TUNING.md# File hướng dẫn này
```

---

## 2. CÀI ĐẶT MÔI TRƯỜNG & NẠP FIRMWARE (PLATFORMIO)

### 2.1. Chuẩn bị phần mềm
1. Cài đặt **Visual Studio Code (VS Code)**.
2. Mở tab Extensions (`Ctrl+Shift+X`), tìm và cài đặt extension **PlatformIO IDE**.
3. Mở thư mục dự án `drone` trong VS Code.

### 2.2. Kiểm tra file cấu hình `platformio.ini`
Đảm bảo cấu hình đúng loại chip ESP32-S3 Octal Flash/PSRAM (R16N8):
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
board_build.arduino.memory_type = qio_opi  ; Octal Flash & Octal PSRAM
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1             ; Native USB Serial CDC
    -DBOARD_HAS_PSRAM
lib_deps =
    adafruit/Adafruit PWM Servo Driver Library@^3.0.2
    adafruit/Adafruit BMP280 Library@^2.6.8
    mikalhart/TinyGPSPlus@^1.0.3
```

### 2.3. Các lệnh Build & Nạp Firmware
* **Nạp code chính**:
  * Nhấn biểu tượng dấu tích `✓` (PlatformIO: Build) ở thanh dưới cùng để biên dịch.
  * Nhấn biểu tượng mũi tên `→` (PlatformIO: Upload) để nạp vào ESP32-S3 qua cáp USB.
  * Nhấn biểu tượng phích cắm `🔌` (PlatformIO: Serial Monitor) để xem log ở baudrate **115200**.

---

## 3. QUY TRÌNH KIỂM TRA PHẦN CỨNG TRÊN BÀN TEST (BENCH TESTING)

⚠️ **CẢNH BÁO AN TOÀN SỐNG CÒN**:
> **TUYỆT ĐỐI KHÔNG GẮN CÁNH QUẠT TRONG QUÁ TRÌNH THỬ NGHIỆM TRÊN BÀN TEST!**
> Cánh quạt 1045 quay 10.000 RPM có thể gây sát thương nghiêm trọng nếu motor bất ngờ tăng ga!

```
                  SƠ ĐỒ BỐ TRÍ ĐỘNG CƠ QUAD-X
                   
                    ĐẦU DRONE (MẶT TRƯỚC)
                            ▲
                   [M2 - CW]     [M1 - CCW]
                      (Trái)     (Phải)
                           \     /
                            \   /
                              X
                            /   \
                           /     \
                   [M4 - CCW]    [M3 - CW]
                      (Trái)     (Phải)
                    ĐUÔI DRONE (MẶT SAU)
```

### Bước 1: Quét Bus I2C
1. Nạp file `tools/i2c_scanner/main.cpp` để kiểm tra toàn bộ thiết bị I2C trên bus SDA (GPIO8) và SCL (GPIO9):
   * `0x68`: Cảm biến IMU MPU6050.
   * `0x1E` (hoặc `0x0D`): Cảm biến la bàn HMC5883L / QMC5883L.
   * `0x76` (hoặc `0x77`): Cảm biến áp suất BMP280.
   * `0x40`: Driver PWM PCA9685.
2. Nếu thiếu thiết bị nào, kiểm tra lại dây SDA, SCL, nguồn 3.3V và 2 trở kéo 4.7kΩ.

### Bước 2: Hiệu chuẩn Gyroscope & La Bàn (Cảm biến)
1. Đặt drone nằm yên trên mặt bàn phẳng tuyệt đối.
2. Gửi lệnh qua Serial: `CALIB GYRO` $\rightarrow$ Chờ 2 giây để mạch đo 500 mẫu offset tĩnh.
3. Cầm drone xoay đều các trục $360^\circ$ khi gửi lệnh `CALIB MAG` để hiệu chuẩn từ trường môi trường.

### Bước 3: Kiểm tra thứ tự và chiều quay của 4 Motor
Gửi lần lượt các lệnh test ga thấp (10% - 15%):
* `TEST M1 15` $\rightarrow$ Motor 1 (Trước Phải) phải quay **Ngược chiều kim đồng hồ (CCW)**.
* `TEST M2 15` $\rightarrow$ Motor 2 (Trước Trái) phải quay **Thuận chiều kim đồng hồ (CW)**.
* `TEST M3 15` $\rightarrow$ Motor 3 (Sau Phải) phải quay **Thuận chiều kim đồng hồ (CW)**.
* `TEST M4 15` $\rightarrow$ Motor 4 (Sau Trái) phải quay **Ngược chiều kim đồng hồ (CCW)**.
* `TEST M1 0` (hoặc `DISARM`) để dừng test.
* *(Nếu motor nào quay ngược chiều: Hoán đổi vị trí 2 trong 3 dây pha nối từ ESC vào motor đó)*.

---

## 4. HƯỚNG DẪN SỬ DỤNG GCS WEB TUNER (CHROME / EDGE WEB SERIAL)

Công cụ GCS Tuner tại `tools/tuner/index.html` được thiết kế chạy trực tiếp trên trình duyệt mà không cần cài đặt phần mềm bên ngoài:

### 4.1. Cách khởi chạy
1. Dùng trình duyệt **Google Chrome** hoặc **Microsoft Edge** (hỗ trợ Web Serial API).
2. Mở trực tiếp file: `tools/tuner/index.html`.
3. Cắm cáp USB nối ESP32-S3 với máy tính.
4. Nhấn nút **"Kết Nối USB"** ở góc phải màn hình $\rightarrow$ Chọn cổng COM của ESP32-S3 $\rightarrow$ Chọn Baudrate **115200**.

### 4.2. Các tính năng nổi bật trên GCS Web Tuner
* **Khối 3D Attitude Model**: Mô phỏng góc nghiêng Roll, Pitch, Yaw của Drone theo thời gian thực (nhờ thuật toán Mahony 250Hz).
* **Đồ thị sóng Realtime (Oscilloscope)**: Vẽ trực tiếp góc đo được vs góc đặt mục tiêu, tốc độ góc Gyro Rate và áp suất độ cao Barometer.
* **Thanh công suất 4 Motor (M1..M4)**: Hiển thị độ rộng xung PWM từ 1000µs đến 2000µs cấp cho từng ESC.
* **Bảng điều khiển Tune PID trực tiếp (Realtime PID Tuning)**:
  * Cho phép chỉnh $K_p, K_i, K_d$ của Roll, Pitch, Yaw.
  * Nhấn nút **"Gửi Lên Drone"** để cập nhật PID tức thì **mà không cần nạp lại firmware!**
* **Nút bấm điều khiển nhanh**: ARM, DISARM khẩn cấp, Calib Gyro, Calib Mag, Calib Baro, Test từng động cơ.

---

## 5. TẬP LỆNH ĐIỀU KHIỂN SERIAL CLI & GIAO THỨC TELEMETRY

Drone chấp nhận các lệnh văn bản thuần (ASCII kết thúc bằng `\n`) qua cổng Serial/USB:

### 5.1. Bảng tập lệnh cấu hình & điều khiển

| Cú pháp Lệnh (Command) | Tham số & Ví dụ | Giải thích Chức Năng |
| :--- | :--- | :--- |
| `ARM` | Không có | Kích hoạt động cơ (Chỉ cho phép khi cần ga = 0 và góc nghiêng $< 15^\circ$) |
| `DISARM` | Không có | **TẮT ĐỘNG CƠ KHẨN CẤP** (Đưa xung 4 ESC về 1000µs) |
| `RC <roll> <pitch> <yaw> <thr>` | `RC 0 0 0 25` | Gửi tín hiệu điều khiển ảo (-45°..+45°, ga 0..100%) |
| `MODE <mode>` | `MODE ANGLE` hoặc `MODE RATE` | Chuyển chế độ bay Tự cân bằng (Angle) hoặc Thao diễn (Rate/Acro) |
| `SET_PID ROLL_RATE <kp> <ki> <kd>` | `SET_PID ROLL_RATE 0.15 0.05 0.003` | Chỉnh thông số PID tốc độ góc Roll (Rate Loop) |
| `SET_PID PITCH_RATE <kp> <ki> <kd>`| `SET_PID PITCH_RATE 0.15 0.05 0.003`| Chỉnh thông số PID tốc độ góc Pitch (Rate Loop) |
| `SET_PID YAW_RATE <kp> <ki> <kd>`  | `SET_PID YAW_RATE 0.25 0.08 0.001`  | Chỉnh thông số PID tốc độ góc Yaw (Heading Loop) |
| `SET_PID ROLL_ANGLE <kp> <ki> <kd>`| `SET_PID ROLL_ANGLE 3.5 0.0 0.0`    | Chỉnh thông số PID góc nghiêng Roll (Angle Loop) |
| `SET_PID PITCH_ANGLE <kp> <ki> <kd>`|`SET_PID PITCH_ANGLE 3.5 0.0 0.0`   | Chỉnh thông số PID góc nghiêng Pitch (Angle Loop) |
| `TEST <M1..M4> <percent>` | `TEST M1 15` | Thử tải động cơ riêng lẻ trên bàn test (chỉ chạy khi Disarmed) |
| `CALIB GYRO` | Không có | Lấy mẫu 500 lần khử trôi Gyroscope |
| `CALIB MAG` | Không có | Bắt đầu chu trình xoay $360^\circ$ calib từ kế |
| `CALIB BARO` | Không có | Lấy mốc độ cao 0m tại mặt đất cho BMP280 |

### 5.2. Chuỗi dữ liệu Telemetry phát ra từ Drone (10Hz)
Định dạng gói tin gửi lên GCS Tuner:
```text
$TEL,<roll>,<pitch>,<yaw>,<rateRoll>,<ratePitch>,<rateYaw>,<throttle>,<m1>,<m2>,<m3>,<m4>,<alt>,<armed>,<fsState>
```
*Ví dụ thực tế:*
`$TEL,1.25,-0.45,180.20,0.02,-0.01,0.00,20.0,1180,1210,1190,1205,0.15,1,0`

---

## 6. CẨM NANG TUNE PID CHI TIẾT TỪNG BƯỚC (CASCADE PID TUNING)

Hệ thống điều khiển bay sử dụng cấu trúc **PID Kép 2 Tầng (Cascade Loop)**:
```
           +-------------------------------------------------------------------+
           |                  VÒNG LẶP ĐIỀU KHIỂN CASCADE 250Hz                |
           |                                                                   |
Góc Đặt    |  +-------------+  Tốc độ góc   +-------------+   Lực bù    +-----+|
Mục tiêu ──┼─>| ANGLE PID   |── mục tiêu ──>| RATE PID    |── Motor ───>| ESC ||
(Từ Remote)|  | (Vòng Ngoài)|   (deg/s)     | (Vòng Trong)|   Mixer     +-----+|
           |  +------+------+               +------+------+                    |
           |         ▲                             ▲                           |
           |         | Góc thực (Attitude)         | Vận tốc góc thực (Gyro)   |
           +---------+-----------------------------+---------------------------+
```

---

### 6.1. Ý nghĩa thực tế của $K_p$, $K_i$, $K_d$ trong điều khiển Drone

#### 1. Hệ số Tỉ Lệ ($K_p$ - Proportional) — "Lực Phản Ứng Tức Thời"
* **Bản chất**: Tạo lực phản kháng tỉ lệ thuận với độ lệch góc hiện tại.
* **Nếu $K_p$ quá thấp**: Drone phản hồi lờ đờ, bị trôi dạt, gió thổi là nghiêng không tự giữ được.
* **Nếu $K_p$ chuẩn**: Drone giữ góc chắc chắn, phản hồi tay ga dứt khoát.
* **Nếu $K_p$ quá cao**: Drone xuất hiện **rung dao động tần số cao (Fast Oscillation)**, phát ra tiếng rít rè rè ở motor.

#### 2. Hệ số Giảm Chấn ($K_d$ - Derivative) — "Bộ Giảm Xóc & Thắng Hãm"
* **Bản chất**: Dự đoán xu hướng thay đổi và hãm lại chuyển động trước khi vọt lố (Damping).
* **Nếu $K_d$ quá thấp**: Drone bị **vọt lố (Overshoot)** và lắc lư 2-3 nhịp trước khi đứng yên.
* **Nếu $K_d$ chuẩn**: Khi nhả cần lái, drone dừng lại ngay lập tức tại vị trí mong muốn như có phanh hãm.
* **Nếu $K_d$ quá cao**: Khuếch đại nhiễu rung cơ khí từ motor, làm **ESC và Motor bị nóng rực sau 30 giây bay!**

#### 3. Hệ số Tích Phân ($K_i$ - Integral) — "Triệt Tiêu Lực Lệch Trọng Tâm & Gió"
* **Bản chất**: Cộng dồn sai số theo thời gian để sinh lực bù cho các tác động ngoại cảnh liên tục (Gió tạt, lệch pin, cánh quạt không đều).
* **Nếu $K_i$ quá thấp**: Drone từ từ trôi nghiêng về một phía khi thả tay lái.
* **Nếu $K_i$ chuẩn**: Drone giữ nguyên góc nghiêng cố định dù có gió thổi nhẹ.
* **Nếu $K_i$ quá cao**: Xuất hiện **dao động lắc lư chậm tần số thấp (Slow Wobble)**.

---

### 6.2. Quy trình 5 bước Tune PID thực tế (Phương pháp thực chiến)

> 💡 **3 Giai đoạn thử nghiệm an toàn khuyến nghị**:
> 1. **Giai đoạn 1 (Khung treo 1 trục)**: Cố định trục Pitch trên giá đỡ, chỉ cho phép trục Roll tự do xoay để tìm thông số Roll mà không sợ lật tứ tung.
> 2. **Giai đoạn 2 (Test cầm tay bảo hộ)**: Đeo găng tay dày và kính bảo hộ, giữ chặt chân đáp dưới bụng drone, lên ga 25% và búng cần gạt để cảm nhận lực phản hồi.
> 3. **Giai đoạn 3 (Buộc dây hãm đất - Tether)**: Dùng 4 sợi dây dù dài 0.5m buộc 4 chân đáp cố định xuống cọc đất để hover thử nghiệm an toàn ngoài bãi cỏ.

#### BƯỚC 1: Tìm $K_p$ Vòng Trong (Roll Rate & Pitch Rate) — "Độ Cứng Vững"
1. Đặt $K_i = 0$, $K_d = 0$, Angle $K_p = 0$ (hoặc bay ở chế độ `ACRO/RATE`).
2. Đặt $K_p = 0.60$ làm mức khởi điểm.
3. Lên ga vừa đủ để drone bắt đầu nâng mình khỏi mặt đất (~25% - 30% ga).
4. Tăng dần $K_p$ mỗi lần `+0.15` qua GCS Tuner ($0.60 \rightarrow 0.75 \rightarrow 0.90 \rightarrow 1.05 \rightarrow 1.20 \rightarrow 1.35...$):
   * *Nếu thấy drone phản hồi lờ đờ, bị lật nghiêng trôi dạt* $\rightarrow$ **Thiếu $K_p$**, tiếp tục tăng.
   * *Nếu thấy drone bắt đầu xuất hiện **rung dao động nhanh tần số cao (Fast Oscillation 15-30Hz)** phát ra tiếng rít "zizz zizz"* $\rightarrow$ **Dư $K_p$**.
5. **Điểm chốt $K_p$ chuẩn**: Lấy giá trị tại điểm bắt đầu rung **nhân với $0.70$** (giảm bớt $30\%$).
   * *Ví dụ: Bắt đầu rung tại $K_p = 1.60 \rightarrow$ Chốt $K_p = 1.60 \times 0.70 = 1.12 \approx 1.20$.*

#### BƯỚC 2: Thêm $K_d$ Vòng Trong — "Phanh Hãm Quán Tính & Dập Tắt Rung Lắc"
1. Bắt đầu tăng $K_d$ từ giá trị `0.015`.
2. Tăng dần $K_d$ mỗi lần `+0.005` ($0.015 \rightarrow 0.020 \rightarrow 0.025 \rightarrow 0.030 \rightarrow 0.035$):
   * Búng nhẹ cần gạt Roll/Pitch rồi thả tay về giữa:
   * *Nếu drone dừng lại lập tức, không bị nảy giật lại (No Bounce-back)* $\rightarrow$ **$K_d$ đạt chuẩn**.
3. ⚠️ **KIỂM TRA NHIỆT ĐỘ MOTOR (BẮT BUỘC)**:
   * Sau mỗi lần thử 45 giây, đáp drone, ngắt ARM và **dùng tay chạm trực tiếp vào vỏ 4 motor**:
   * *Motor ấm nhẹ ($< 45^\circ\text{C}$)* $\rightarrow$ Hoàn hảo.
   * *Motor nóng rát tay ($> 60^\circ\text{C}$)* $\rightarrow$ **$K_d$ quá cao** (khuếch đại nhiễu rung cơ học làm ESC nhồi xung liên tục) $\rightarrow$ **Phải giảm ngay $K_d$ xuống 30%** và kiểm tra đệm cao su giảm chấn của MPU6050.

#### BƯỚC 3: Thêm $K_i$ Vòng Trong — "Khóa Góc Cố Định & Chống Gió Tạt"
1. Đặt $K_i = 0.02$ làm khởi điểm.
2. Tăng dần $K_i$ mỗi lần `+0.01` ($0.02 \rightarrow 0.03 \rightarrow 0.04 \rightarrow 0.05$):
   * Nghiêng cần lái tạo góc nghiêng $15^\circ$ rồi thả tay:
   * *Nếu drone giữ nguyên tư thế góc đó khi bay thẳng mà không bị trôi từ từ về mặt đất* $\rightarrow$ **$K_i$ đạt chuẩn**.
   * *Nếu thấy drone xuất hiện **dao động nhấp nhô chậm chạp (Slow Wobble 1-2Hz)*** $\rightarrow$ **Dư $K_i$**, giảm bớt $20\%$.

#### BƯỚC 4: Kích Hoạt $K_p$ Vòng Ngoài (Angle Loop) — "Độ Nhạy Tự Cân Bằng"
1. Chuyển sang chế độ bay **`ANGLE`** trên Web Cockpit hoặc Web Tuner.
2. Đặt Angle $K_p = 3.0$ (Angle $K_i = 0$, Angle $K_d = 0$).
3. Tăng dần $K_p$ mỗi lần `+0.5` ($3.0 \rightarrow 3.5 \rightarrow 4.0 \rightarrow 4.5 \rightarrow 5.0$):
   * Lấy tay nghiêng drone rồi buông ra: Drone phải **tự động bật thẳng đứng lại vị trí cân bằng ngang trong tích tắc**.
   * *Nếu drone phản hồi trả về chậm chạp* $\rightarrow$ Tăng Angle $K_p$.
   * *Nếu drone tự cân bằng nhưng bị lắc lư qua lại vài nhịp trước khi phẳng* $\rightarrow$ Giảm Angle $K_p$.

#### BƯỚC 5: Tune Trục Xoay Đầu (Yaw Rate Loop)
1. Trục Yaw luôn điều khiển trực tiếp tốc độ xoay deg/s (không dùng vòng Angle).
2. Đặt $K_p = 2.0$, $K_i = 0.05$, $K_d = 0.000$.
3. Tăng dần $K_p$ lên $2.5 - 3.0$ để mũi drone xoay dứt khoát theo cần lái.
4. Tăng $K_i$ lên $0.08 - 0.10$ để giữ hướng mũi cố định, không bị tự xoay đuôi do phản lực cánh quạt (Yaw Drift).
5. *Lưu ý*: Trục Yaw thường **luôn để $K_d = 0$** để tránh làm nóng motor vô ích.

---

## 7. BẢNG THÔNG SỐ PID KHỞI ĐIỂM KHUYẾN NGHỊ

*(Áp dụng cho khung Quad-X 450mm, Motor A2212 1000KV, Cánh 1045, ESC 30A, Pin 3S 2200mAh)*:

| Bộ Điều Khiển (Controller) | Trục (Axis) | $K_p$ | $K_i$ | $K_d$ | D-Filter $\alpha$ | Giới Hạn Max | Ghi Chú Kỹ Thuật |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Angle PID (Vòng ngoài)** | **Roll Angle** | `4.50` | `0.00` | `0.00` | — | $\pm 300^\circ/\text{s}$ | Tự động cân bằng trục ngang |
| **Angle PID (Vòng ngoài)** | **Pitch Angle** | `4.50` | `0.00` | `0.00` | — | $\pm 300^\circ/\text{s}$ | Tự động cân bằng trục dọc |
| **Rate PID (Vòng trong 250Hz)** | **Roll Rate** | `1.20` | `0.04` | `0.035` | `0.70` (LPF) | $\pm 400\,\mu\text{s}$ | Phản hồi tốc độ góc Roll (deg/s) |
| **Rate PID (Vòng trong 250Hz)** | **Pitch Rate** | `1.20` | `0.04` | `0.035` | `0.70` (LPF) | $\pm 400\,\mu\text{s}$ | Phản hồi tốc độ góc Pitch (deg/s) |
| **Rate PID (Vòng trong 250Hz)** | **Yaw Rate** | `2.50` | `0.08` | `0.000` | — | $\pm 400\,\mu\text{s}$ | Khóa hướng mũi drone (Heading) |

---

## 8. CHẨN ĐOÁN LỖI & XỬ LÝ SỰ CỐ THƯỜNG GẶP (TROUBLESHOOTING)

### 🔴 Sự cố 1: Vừa đẩy nhẹ ga, Drone lật úp ngay lập tức (Flip on takeoff)
* **Nguyên nhân 1**: Thứ tự cắm dây 4 motor bị sai vị trí (Ví dụ cắm nhầm Motor 1 sang Motor 2).
* **Nguyên nhân 2**: Chiều quay motor bị ngược (M1, M4 phải là CCW; M2, M3 phải là CW).
* **Nguyên nhân 3**: Chiều gắn cánh quạt bị ngược (Cánh R lắp nhầm sang cánh thường).
* **Nguyên nhân 4**: Chiều trục Gyroscope bị ngược dấu (Ví dụ nghiêng phải nhưng IMU lại báo nghiêng trái $\rightarrow$ PID càng bù lực làm lật mạnh hơn).

### 🔴 Sự cố 2: Động cơ kêu rít và rất nóng sau khi bay
* **Nguyên nhân**: Hệ số $K_d$ quá cao hoặc rung động cơ khí từ cánh quạt cong/mất cân bằng truyền thẳng vào chip MPU6050.
* **Cách khắc phục**:
  * Giảm $K_d$ xuống 30–50%.
  * Dùng băng keo xốp đệm chống rung 3M hoặc đệm cao su silicon gắn dưới đế MPU6050.
  * Cân bằng động cánh quạt (Prop Balancer).

### 🔴 Sự cố 3: Đang bay thì Drone mất nguồn hoặc ESP32-S3 tự Reset
* **Nguyên nhân**: Hiện tượng **Brownout Reset** do sụt áp 5V khi 4 motor ăn dòng lớn.
* **Cách khắc phục**:
  * Kiểm tra xem đã hàn **Tụ Hóa 100µF + Tụ Gốm 104** song song vào chân 5V/VIN của ESP32 chưa.
  * Kiểm tra nguồn UBEC 5V/3A có bị nóng quá mức hoặc dây nguồn cấp quá mảnh ($< 22\text{AWG}$) hay không.

### 🔴 Sự cố 4: Góc đo của Drone bị trôi dần (Drift) hoặc treo bus I2C
* **Nguyên nhân**: Nhiễu điện từ trường từ dây nguồn 11.1V đánh vào đường SDA/SCL.
* **Cách khắc phục**:
  * Đã gắn đủ **2 trở kéo 4.7kΩ** từ SDA và SCL lên nguồn 3.3V chưa?
  * Đã hàn tụ gốm 104 sát chân VCC-GND của MPU6050 chưa?
  * Xoắn đôi cặp dây SDA/SCL với dây GND khi đi dây dài.
