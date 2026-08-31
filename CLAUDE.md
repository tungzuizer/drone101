# CLAUDE.md — ESP32-S3 Flight Controller Drone

## Bối cảnh dự án

Firmware điều khiển bay (flight controller) cho quadcopter tự chế dạng X, sử dụng vi điều khiển **ESP32-S3 (R16N8)**, lập trình bằng Arduino framework qua **PlatformIO**.

### Phần cứng
- **MCU**: ESP32-S3 R16N8 (16MB Flash Octal, 8MB PSRAM Octal).
- **IMU**: MPU6050 (I2C: 0x68).
- **Magnetometer**: HMC5883L / QMC5883L (I2C: 0x1E hoặc 0x0D).
- **Barometer**: BMP280 (I2C: 0x76 / 0x77).
- **PWM Driver**: PCA9685 16-channel 12-bit (I2C: 0x40) -> điều khiển 4 ESC.
- **ESC & Motors**: 4x ESC 30A (PWM 50Hz, 1000–2000µs), 4x Motor A2212 1000KV (khung quad X).
- **GPS**: ATGM336H (UART1: RX=GPIO17, TX=GPIO18, NMEA 0183).
- **Motor phụ**: L9110S (GPIO4, 5, 6, 7 — tuỳ chọn).

### Sơ đồ chân GPIO
- I2C (MPU6050, HMC/QMC5883L, BMP280, PCA9685): **SDA = GPIO8, SCL = GPIO9** (Cấp nguồn 3.3V cho các module I2C).
- UART GPS (UART1): **RX = GPIO17, TX = GPIO18**
- L9110S: **GPIO4, GPIO5, GPIO6, GPIO7**
- **Tránh dùng**: GPIO33–37 (PSRAM Octal SPI) và GPIO19/20 (USB D+/D-).

## ⚠️ Ràng buộc phần cứng & An toàn

1. **CHƯA CÓ bộ thu RC**: Điều khiển qua **Serial/USB** với kiến trúc interface trừu tượng `ControlInputSource` (dễ dàng cắm ESP-NOW / RC receiver sau này).
2. **Điện áp I2C**: GPIO ESP32-S3 chỉ chịu 3.3V. Cấp 3.3V cho toàn bộ module I2C.
3. **Dòng xả pin**: Pin LiPo 3S >= 20C (2200mAh), đầu nối XT60.
4. **HMC5883L vs QMC5883L**: Code tự phát hiện chip thật vs chip clone.
5. **PCA9685**: Độ phân giải PWM ~200 mức ở 50Hz.
6. **An toàn thử nghiệm**:
   - LUÔN THÁO CÁNH QUẠT khi thử nghiệm động cơ trên bàn test.
   - Không tự động arm khi khởi động.
   - Throttle giới hạn tối đa ban đầu ở 30%.

## Cấu trúc module dự kiến
- `src/main.cpp`
- `src/sensors/ImuSensor.h/.cpp` (MPU6050)
- `src/sensors/Magnetometer.h/.cpp` (HMC/QMC5883L)
- `src/sensors/Barometer.h/.cpp` (BMP280)
- `src/sensors/GpsReader.h/.cpp` (ATGM336H)
- `src/actuators/MotorController.h/.cpp` (PCA9685 + ESC)
- `src/control/AttitudeEstimator.h/.cpp` (Complementary / Madgwick filter)
- `src/control/PidController.h/.cpp`
- `src/control/MotorMixer.h/.cpp`
- `src/safety/FailsafeManager.h/.cpp`
- `src/control/ControlInputSource.h` + `SerialControlInput.h/.cpp`
- `src/config.h` (GPIO, PID constants, safety limits)

## Quy trình triển khai từng bước
0. **Checklist an toàn phần cứng** (Không viết code, chờ xác nhận).
1. Setup project PlatformIO + I2C scanner (nhận diện MPU6050, HMC/QMC5883L, BMP280, PCA9685).
2. Module MPU6050 + Calib offset.
3. Module Magnetometer + Tilt compensation.
4. Module BMP280 (Áp suất + Độ cao).
5. Attitude Estimator (Complementary Filter).
6. MotorController (PCA9685 + ESC calib + Arm limit).
7. ControlInputSource + SerialControlInput.
8. PID Controller (Roll/Pitch/Yaw).
9. Motor Mixer (Quad-X).
10. Failsafe Manager (Timeout, Angle tilt threshold, I2C fail).
11. GPS Reader (ATGM336H NMEA).
12. Main loop integration (250–500Hz non-blocking).
