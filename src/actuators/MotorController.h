#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include "../config.h"

// Số lượng động cơ chính của Quadcopter
#define NUM_MOTORS 4

// Giới hạn độ rộng xung chuẩn của ESC máy bay (Microseconds)
#define ESC_MIN_PULSE_US    1000    // Xung tối thiểu (Motor dừng hoàn toàn)
#define ESC_IDLE_PULSE_US   1100    // Xung quay chậm khi Arm (Idle)
#define ESC_MAX_PULSE_US    2000    // Xung tối đa (100% công suất)

class MotorController {
public:
    MotorController();

    // Khởi tạo PCA9685, cấu hình tần số PWM 50Hz cho ESC
    bool begin(uint8_t i2cAddress = I2C_ADDR_PCA9685_PRI);

    // Kích hoạt động cơ (ARM) - Có kiểm tra an toàn (Cần ga phải = 0)
    bool arm();

    // Ngắt kích hoạt động cơ (DISARM) - Đưa toàn bộ 4 ESC về xung 1000µs
    void disarm();

    // Dừng khẩn cấp tức thì (Emergency Disarm)
    void emergencyStop();

    // Đặt độ rộng xung trực tiếp cho từng động cơ (1000 - 2000µs)
    // motorIndex: 0 = M1 (Trước Phải), 1 = M2 (Trước Trái), 2 = M3 (Sau Phải), 3 = M4 (Sau Trái)
    void setMotorPwm(uint8_t motorIndex, uint16_t pulseUs);

    // Đặt công suất động cơ theo phần trăm (0.0% - 100.0%)
    void setMotorPercent(uint8_t motorIndex, float percent);

    // Đặt độ rộng xung cho cả 4 động cơ cùng một lúc
    void setAllMotorsPwm(uint16_t m1Us, uint16_t m2Us, uint16_t m3Us, uint16_t m4Us);

    // Test riêng lẻ 1 động cơ từ phần mềm Tuner GCS (Có kiểm tra an toàn)
    void testMotor(uint8_t motorNum, float percent);

    // Trạng thái Arm
    bool isArmed() const { return isArmed_; }
    bool isHealthy() const { return isHealthy_; }

    // Đọc xung hiện tại của từng động cơ
    uint16_t getMotorPwm(uint8_t motorIndex) const {
        return (motorIndex < NUM_MOTORS) ? currentPulseUs_[motorIndex] : ESC_MIN_PULSE_US;
    }

    // Giới hạn công suất tối đa khi test bàn (Mặc định 30%)
    void setMaxTestThrottle(uint8_t maxPercent) { maxTestThrottlePercent_ = maxPercent; }
    uint8_t getMaxTestThrottle() const { return maxTestThrottlePercent_; }

private:
    uint8_t address_;
    bool isHealthy_;
    bool isArmed_;
    uint8_t maxTestThrottlePercent_;

    uint16_t currentPulseUs_[NUM_MOTORS];

    // Chuyển đổi Microseconds (1000 - 2000µs) sang 12-bit Counts của PCA9685 (0 - 4095)
    uint16_t usToCounts(uint16_t pulseUs);

    // Ghi PWM trực tiếp vào thanh ghi kênh của PCA9685
    bool setChannelPwm(uint8_t channel, uint16_t onCount, uint16_t offCount);

    bool writeRegister(uint8_t regAddr, uint8_t data);
    bool readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length);
};

#endif // MOTOR_CONTROLLER_H
