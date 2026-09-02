#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =============================================================================
// CẤU HÌNH PHẦN CỨNG & CHÂN GPIO ESP32-S3 (R16N8)
// =============================================================================

// --- I2C BUS (CẢM BIẾN & DRIVER MỞ RỘNG) ---
// Dùng chung cho: MPU6050, HMC5883L / QMC5883L, BMP280, PCA9685
#define PIN_I2C_SDA         8
#define PIN_I2C_SCL         9
#define I2C_FREQUENCY       100000  // 100kHz (Chuẩn I2C Standard Mode chống nhiễu dây nối test bàn)

// --- CHỌN PHƯƠNG THỨC ĐIỀU KHIỂN ĐỘNG CƠ (MOTOR DRIVER MODE) ---
// true: Dùng qua I2C PCA9685 16-kênh
// false: Dùng phát xung phần cứng trực tiếp từ ESP32-S3 (GPIO 4, 5, 6, 7) - KHÔNG CẦN PCA9685
#define USE_PCA9685_FOR_MOTORS  false   // Mặc định false để điều khiển trực tiếp trên 4 chân GPIO 4,5,6,7 nếu chưa cắm PCA9685

// --- CHÂN ĐIỀU KHIỂN ESC ĐỘNG CƠ TRỰC TIẾP (HARDWARE MCPWM / LEDC 50-400Hz) ---
// 4 kênh ngõ ra PWM trực tiếp từ ESP32-S3 (khi USE_PCA9685_FOR_MOTORS = false)
#define PIN_MOTOR_1         4       // ESC 1: Động cơ M1 (Trước Phải - CCW)
#define PIN_MOTOR_2         5       // ESC 2: Động cơ M2 (Trước Trái - CW)
#define PIN_MOTOR_3         6       // ESC 3: Động cơ M3 (Sau Phải - CW)
#define PIN_MOTOR_4         7       // ESC 4: Động cơ M4 (Sau Trái - CCW)

// --- GIÁM SÁT ĐIỆN ÁP PIN & DÒNG ĐIỆN (ADC) ---
#define PIN_VBAT_SENSE          1       // ADC1_CH0 (GPIO 1): Giám sát pin LiPo 3S (11.1V-12.6V)
#define PIN_CURRENT_SENSE       2       // ADC1_CH1 (GPIO 2): Cảm biến dòng điện PDB / Shunt Sensor
#define VBAT_DIVIDER_R1_KOHM    10.0f   // Điện trở phân áp trên R1 = 10kΩ
#define VBAT_DIVIDER_R2_KOHM    2.2f    // Điện trở phân áp dưới R2 = 2.2kΩ
#define VBAT_CALIBRATION_SCALE  ((VBAT_DIVIDER_R1_KOHM + VBAT_DIVIDER_R2_KOHM) / VBAT_DIVIDER_R2_KOHM) // Hệ số nhân tỷ lệ (5.545f)

// --- CÒI BÁO ĐỘNG & ĐÈN BÁO TRẠNG THÁI (BUZZER & STATUS LEDS) ---
#define PIN_BUZZER          10      // Còi chíp Active Buzzer 5V (Báo Arm/Disarm/Pin yếu/Tìm drone)
#define PIN_ARM_LED         3       // Đèn LED đỏ báo trạng thái Arm động cơ
#define PIN_STATUS_LED      48      // WS2812 RGB LED nội hoặc LED xanh báo trạng thái cân bằng & GPS Fix

// --- GPS UART (UART1) ---
// Module ATGM336H NMEA 0183
#define PIN_GPS_RX          17      // U1RXD: Nối với TX của ATGM336H
#define PIN_GPS_TX          18      // U1TXD: Nối với RX của ATGM336H
#define GPS_BAUDRATE        9600

// --- BỘ THU SÓNG TAY CẦM RC RECEIVER (UART2 - SBUS / CRSF / ELRS / IBUS) ---
#define PIN_RC_RX           43      // U2RXD: Nhận tín hiệu điều khiển từ bộ thu RC ELRS / Crossfire / SBUS
#define PIN_RC_TX           44      // U2TXD: Telemetry hồi tiếp dữ liệu về tay cầm điều khiển

// --- DRIVER MOTOR DC PHỤ (L9110S - Tuỳ chọn cơ cấu phụ khi dùng PCA9685 cho ESC chính) ---
// Lưu ý: Chỉ dùng GPIO 4-7 cho L9110S nếu 4 ESC chính được điều khiển qua module I2C PCA9685
#define PIN_L9110S_IA       4
#define PIN_L9110S_IB       5
#define PIN_L9110S_IC       6
#define PIN_L9110S_ID       7

// --- CÁC CHÂN CẤM SỬ DỤNG TRÊN ESP32-S3 R16N8 ---
// GPIO 33-37: Dùng cho Octal SPI PSRAM / Flash
// GPIO 19-20: Dùng cho USB D- / D+ (Native USB CDC Telemetry)

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
// CẤU HÌNH MPU6050 (IMU) & HƯỚNG LẮP ĐẶT (SENSOR ORIENTATION)
// =============================================================================
// Các góc xoay lắp đặt MPU6050 trên khung Drone:
// IMU_ALIGN_DEFAULT : Mũi tên X hướng Trước (Nose), Y hướng Trái
// IMU_ALIGN_CW90    : Xoay 90° thuận KĐH (Mũi tên X hướng Phải, Y hướng Sau)
// IMU_ALIGN_CW180   : Xoay 180° (Mũi tên X hướng Sau, Y hướng Phải)
// IMU_ALIGN_CCW90   : Xoay 90° ngược KĐH (Mũi tên X hướng Trái, Y hướng Trước)
enum ImuOrientation {
    IMU_ALIGN_DEFAULT = 0,
    IMU_ALIGN_CW90    = 1,
    IMU_ALIGN_CW180   = 2,
    IMU_ALIGN_CCW90   = 3
};

// Cấu hình hướng xoay MPU6050 thực tế trên drone (Mặc định CW90 cho lắp vuông góc sang phải)
#define IMU_SENSOR_ORIENTATION  IMU_ALIGN_CW90

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

// =============================================================================
// CẤU HÌNH WI-FI SOFT-AP & ĐIỀU KHIỂN BẰNG ĐIỆN THOẠI (SMARTPHONE WEB COCKPIT)
// =============================================================================
#define ENABLE_WIFI_COCKPIT     false               // Đặt false để tắt Wi-Fi khi test bàn / giảm tải nguồn USB
#define WIFI_AP_SSID            "ESP32-DRONE-FC"
#define WIFI_AP_PASS            "12345678"          // Tối thiểu 8 ký tự WPA2
#define WIFI_AP_CHANNEL         6                   // Kênh truyền sóng Wi-Fi (1, 6, 11)
#define WIFI_AP_MAX_CLIENTS     4                   // Giới hạn số thiết bị kết nối đồng thời
#define WIFI_HTTP_PORT          80                  // Cổng Web Server
#define WIFI_WS_PORT            81                  // Cổng WebSocket
#define WIFI_TELEMETRY_RATE_HZ  25                  // Tần số truyền Telemetry về điện thoại (25Hz)
#define WIFI_FAILSAFE_TIMEOUT_MS 500                // Thời gian mất gói tin Wi-Fi kích hoạt Failsafe (500ms)

// Các chế độ giới hạn ga an toàn trên giao diện điện thoại
#define PHONE_THROTTLE_LIMIT_BEGINNER 30.0f         // Mức tập bay trong nhà: Max 30% ga
#define PHONE_THROTTLE_LIMIT_SPORT    60.0f         // Mức bay ngoài trời: Max 60% ga
#define PHONE_THROTTLE_LIMIT_PRO     100.0f         // Mức tối đa: 100% ga

#endif // CONFIG_H
