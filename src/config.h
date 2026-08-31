#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// CẤU HÌNH PHẦN CỨNG & CHÂN GPIO ESP32-S3 (R16N8)
// =============================================================================

// --- I2C BUS ---
// Dùng chung cho: MPU6050, HMC5883L / QMC5883L, BMP280, PCA9685
#define PIN_I2C_SDA         8
#define PIN_I2C_SCL         9
#define I2C_FREQUENCY       400000  // 400kHz (I2C Fast Mode)

// --- GPS UART (UART1) ---
// Module ATGM336H NMEA 0183
#define PIN_GPS_RX          17      // Nối với TX của ATGM336H
#define PIN_GPS_TX          18      // Nối với RX của ATGM336H
#define GPS_BAUDRATE        9600

// --- MOTOR DRIVER DC (L9110S - Tuỳ chọn / Chức năng phụ) ---
#define PIN_L9110S_IA       4
#define PIN_L9110S_IB       5
#define PIN_L9110S_IC       6
#define PIN_L9110S_ID       7

// --- CÁC CHÂN CẤM SỬ DỤNG TRÊN ESP32-S3 R16N8 ---
// GPIO 33-37: Dùng cho Octal SPI PSRAM / Flash
// GPIO 19-20: Dùng cho USB D- / D+

// =============================================================================
// ĐỊA CHỈ I2C MẶC ĐỊNH CỦA CÁC CẢM BIẾN & THIẾT BỊ
// =============================================================================
#define I2C_ADDR_MPU6050_PRI    0x68    // AD0 = GND (Mặc định)
#define I2C_ADDR_MPU6050_ALT    0x69    // AD0 = 3.3V
#define I2C_ADDR_HMC5883L       0x1E    // Chip từ kế HMC5883L thật
#define I2C_ADDR_QMC5883L       0x0D    // Chip từ kế QMC5883L (Clone phổ biến)
#define I2C_ADDR_BMP280_PRI     0x76    // SDO = GND
#define I2C_ADDR_BMP280_ALT     0x77    // SDO = 3.3V
#define I2C_ADDR_PCA9685_PRI    0x40    // Tất cả jumper A0-A5 = Open
#define I2C_ADDR_PCA9685_ALLCALL 0x70   // All Call Address mặc định của PCA9685

// =============================================================================
// CẤU HÌNH MPU6050 (IMU)
// =============================================================================
// Gyro Scale: 0=±250dps (131.0 LSB/dps), 1=±500dps (65.5 LSB/dps), 2=±1000dps (32.8 LSB/dps), 3=±2000dps (16.4 LSB/dps)
#define MPU6050_GYRO_FS_SEL     1       // ±500 deg/s (chuẩn cho Drone bay cân bằng)
#define MPU6050_GYRO_SCALE      65.5f   // LSB / (deg/s)

// Accel Scale: 0=±2g (16384 LSB/g), 1=±4g (8192 LSB/g), 2=±8g (4096 LSB/g), 3=±16g (2048 LSB/g)
#define MPU6050_ACCEL_FS_SEL    1       // ±4g (chuẩn chống rung động cơ)
#define MPU6050_ACCEL_SCALE     8192.0f // LSB / g

// DLPF (Bộ lọc thông thấp số lọc nhiễu rung động cơ): 0=260Hz, 1=184Hz, 2=94Hz, 3=44Hz, 4=21Hz, 5=10Hz, 6=5Hz
#define MPU6050_DLPF_CFG        2       // 94Hz Accel / 98Hz Gyro (Tối ưu độ trễ thấp & lọc rung)
#define MPU6050_CALIB_SAMPLES   500     // Số mẫu lấy khi hiệu chuẩn Gyro ban đầu

// =============================================================================
// CẤU HÌNH TẦN SỐ VÒNG LẶP & TIMING
// =============================================================================
#define MAIN_LOOP_FREQ_HZ       250     // Tần số vòng lặp điều khiển chính (250Hz = 4ms)
#define SERIAL_BAUD_RATE        115200  // Tốc độ Serial giao tiếp máy tính

// =============================================================================
// CẤU HÌNH AN TOÀN (SAFETY LIMITS)
// =============================================================================
#define MAX_TEST_THROTTLE_PERCENT 30    // Giới hạn ga tối đa trong giai đoạn test bàn (30%)
#define FAILSAFE_TIMEOUT_MS     500     // Thời gian mất tín hiệu điều khiển trước khi ngắt động cơ
#define MAX_TILT_ANGLE_DEG      45.0f   // Góc nghiêng tối đa trước khi kích hoạt failsafe cắt motor

#endif // CONFIG_H
