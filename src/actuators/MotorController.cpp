#include "MotorController.h"

// Địa chỉ thanh ghi PCA9685
#define PCA9685_REG_MODE1           0x00
#define PCA9685_REG_MODE2           0x01
#define PCA9685_REG_SUBADR1         0x02
#define PCA9685_REG_SUBADR2         0x03
#define PCA9685_REG_SUBADR3         0x04
#define PCA9685_REG_ALLCALLADR      0x05
#define PCA9685_REG_LED0_ON_L       0x06
#define PCA9685_REG_LED0_ON_H       0x07
#define PCA9685_REG_LED0_OFF_L      0x08
#define PCA9685_REG_LED0_OFF_H      0x09
#define PCA9685_REG_ALL_LED_ON_L    0xFA
#define PCA9685_REG_ALL_LED_ON_H    0xFB
#define PCA9685_REG_ALL_LED_OFF_L   0xFC
#define PCA9685_REG_ALL_LED_OFF_H   0xFD
#define PCA9685_REG_PRESCALE        0xFE

// Bit điều khiển MODE1
#define PCA9685_MODE1_RESTART       0x80
#define PCA9685_MODE1_SLEEP         0x10
#define PCA9685_MODE1_AI            0x20    // Auto-Increment thanh ghi
#define PCA9685_MODE2_OUTDRV        0x04    // Totem pole output (Mạnh hơn Open-drain)

// Tần số dao động thạch anh nội của PCA9685 (25MHz)
#define PCA9685_OSC_CLOCK_FREQ      25000000.0f
#define PCA9685_PWM_FREQ_HZ         50.0f   // 50Hz tiêu chuẩn cho ESC RC (Chu kỳ 20,000 µs)

MotorController::MotorController()
    : address_(I2C_ADDR_PCA9685_PRI),
      isHealthy_(false),
      isArmed_(false),
      maxTestThrottlePercent_(MAX_TEST_THROTTLE_PERCENT) {
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        currentPulseUs_[i] = ESC_MIN_PULSE_US;
    }
}

bool MotorController::begin(uint8_t i2cAddress) {
    address_ = i2cAddress;
    isHealthy_ = false;
    isArmed_ = false;

    // 1. Kiểm tra kết nối I2C tới PCA9685
    Wire.beginTransmission(address_);
    if (Wire.endTransmission() != 0) {
        Serial.printf("[MOTOR ERROR] Không tìm thấy PCA9685 tại địa chỉ 0x%02X!\n", address_);
        return false;
    }

    // 2. Reset chip: ghi MODE1 = 0x00 (Normal mode)
    writeRegister(PCA9685_REG_MODE1, 0x00);
    delay(10);

    // 3. Cấu hình tần số PWM 50Hz cho ESC máy bay
    // Công thức: prescale = round(25MHz / (4096 * 50Hz)) - 1 = round(122.07) - 1 = 121 (0x79)
    float prescaleVal = (PCA9685_OSC_CLOCK_FREQ / (4096.0f * PCA9685_PWM_FREQ_HZ)) - 1.0f;
    uint8_t prescale = (uint8_t)floor(prescaleVal + 0.5f);

    uint8_t oldMode = 0;
    readRegisters(PCA9685_REG_MODE1, &oldMode, 1);
    uint8_t sleepMode = (oldMode & 0x7F) | PCA9685_MODE1_SLEEP; // Chuyển sang Sleep để đổi Prescale

    writeRegister(PCA9685_REG_MODE1, sleepMode);
    writeRegister(PCA9685_REG_PRESCALE, prescale);
    writeRegister(PCA9685_REG_MODE1, oldMode);
    delay(5);

    // 4. Kích hoạt Auto-Increment và Cấu hình Totem-Pole Output
    writeRegister(PCA9685_REG_MODE1, oldMode | PCA9685_MODE1_RESTART | PCA9685_MODE1_AI);
    writeRegister(PCA9685_REG_MODE2, PCA9685_MODE2_OUTDRV);

    // 5. Khởi tạo toàn bộ 4 kênh ESC về mức an toàn tối thiểu (1000 µs)
    uint16_t minCounts = usToCounts(ESC_MIN_PULSE_US);
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        setChannelPwm(i, 0, minCounts);
        currentPulseUs_[i] = ESC_MIN_PULSE_US;
    }

    isHealthy_ = true;
    Serial.printf("[MOTOR OK] Khởi tạo PCA9685 thành công (Addr: 0x%02X, Tần số: 50Hz, Prescale: %d)\n",
                  address_, prescale);
    return true;
}

bool MotorController::arm() {
    if (!isHealthy_) {
        Serial.println("[ARM DENIED] PCA9685 không khỏe mạnh!");
        return false;
    }

    isArmed_ = true;
    // Khi ARM, đưa cả 4 động cơ về mức Idle (1100 µs) để cánh quạt quay chậm báo hiệu
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        setMotorPwm(i, ESC_IDLE_PULSE_US);
    }
    Serial.println("[ARMED] ĐỘNG CƠ ĐÃ KÍCH HOẠT (ARM)! CẢNH BÁO NGUY HIỂM!");
    return true;
}

void MotorController::disarm() {
    isArmed_ = false;
    // Đưa toàn bộ 4 động cơ về 1000 µs (Dừng hoàn toàn)
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        setMotorPwm(i, ESC_MIN_PULSE_US);
    }
    Serial.println("[DISARMED] Động cơ đã ngắt kích hoạt an toàn (STOP).");
}

void MotorController::emergencyStop() {
    isArmed_ = false;
    uint16_t minCounts = usToCounts(ESC_MIN_PULSE_US);
    // Ghi tức thì vào thanh ghi All LED để cắt PWM ngay lập tức
    for (uint8_t i = 0; i < 16; i++) {
        setChannelPwm(i, 0, minCounts);
    }
    for (uint8_t i = 0; i < NUM_MOTORS; i++) {
        currentPulseUs_[i] = ESC_MIN_PULSE_US;
    }
    Serial.println("[EMERGENCY STOP] DỪNG KHẨN CẤP TOÀN BỘ ĐỘNG CƠ!");
}

void MotorController::setMotorPwm(uint8_t motorIndex, uint16_t pulseUs) {
    if (!isHealthy_ || motorIndex >= NUM_MOTORS) return;

    // Giới hạn xung an toàn từ 1000µs đến 2000µs
    if (pulseUs < ESC_MIN_PULSE_US) pulseUs = ESC_MIN_PULSE_US;
    if (pulseUs > ESC_MAX_PULSE_US) pulseUs = ESC_MAX_PULSE_US;

    // Nếu chưa ARM, bắt buộc chỉ phát xung 1000µs
    if (!isArmed_) {
        pulseUs = ESC_MIN_PULSE_US;
    }

    currentPulseUs_[motorIndex] = pulseUs;
    uint16_t offCount = usToCounts(pulseUs);
    setChannelPwm(motorIndex, 0, offCount);
}

void MotorController::setMotorPercent(uint8_t motorIndex, float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    // Ánh xạ % sang Microseconds: 0% = 1000µs, 100% = 2000µs
    uint16_t pulse = ESC_MIN_PULSE_US + (uint16_t)(percent * 10.0f);
    setMotorPwm(motorIndex, pulse);
}

void MotorController::setAllMotorsPwm(uint16_t m1Us, uint16_t m2Us, uint16_t m3Us, uint16_t m4Us) {
    setMotorPwm(0, m1Us);
    setMotorPwm(1, m2Us);
    setMotorPwm(2, m3Us);
    setMotorPwm(3, m4Us);
}

void MotorController::testMotor(uint8_t motorNum, float percent) {
    if (!isHealthy_) {
        Serial.println("[TEST ERROR] PCA9685 không khả dụng!");
        return;
    }

    if (isArmed_) {
        Serial.println("[TEST DENIED] Không thể test từng động cơ khi hệ thống đang ARM!");
        return;
    }

    // Giới hạn an toàn thử nghiệm trên bàn test (Tối đa 30% để tránh lật/bay ngoài ý muốn)
    if (percent > maxTestThrottlePercent_) {
        Serial.printf("[SAFETY ALERT] Ga test %.1f%% vượt quá giới hạn an toàn (%d%%)!\n",
                      percent, maxTestThrottlePercent_);
        percent = (float)maxTestThrottlePercent_;
    }

    if (percent < 0.0f) percent = 0.0f;

    // Chuyển đổi sang chỉ số mảng (0 - 3)
    uint8_t idx = motorNum - 1;
    if (idx >= NUM_MOTORS) return;

    if (percent == 0.0f) {
        currentPulseUs_[idx] = ESC_MIN_PULSE_US;
        setChannelPwm(idx, 0, usToCounts(ESC_MIN_PULSE_US));
        Serial.printf("[TEST MOTOR] M%d -> STOP (1000µs)\n", motorNum);
    } else {
        uint16_t pulse = ESC_MIN_PULSE_US + (uint16_t)(percent * 10.0f);
        currentPulseUs_[idx] = pulse;
        setChannelPwm(idx, 0, usToCounts(pulse));
        Serial.printf("[TEST MOTOR] M%d -> %.1f%% (%dµs)\n", motorNum, percent, pulse);
    }
}

uint16_t MotorController::usToCounts(uint16_t pulseUs) {
    // Chu kỳ 50Hz = 20,000 µs chia làm 4096 nấc
    // Count = (pulseUs * 4096) / 20000 = (pulseUs * 512) / 2500
    uint32_t counts = ((uint32_t)pulseUs * 4096UL) / 20000UL;
    if (counts > 4095) counts = 4095;
    return (uint16_t)counts;
}

bool MotorController::setChannelPwm(uint8_t channel, uint16_t onCount, uint16_t offCount) {
    if (channel > 15) return false;

    uint8_t regBase = PCA9685_REG_LED0_ON_L + (channel * 4);
    Wire.beginTransmission(address_);
    Wire.write(regBase);
    Wire.write((uint8_t)(onCount & 0xFF));
    Wire.write((uint8_t)((onCount >> 8) & 0x0F));
    Wire.write((uint8_t)(offCount & 0xFF));
    Wire.write((uint8_t)((offCount >> 8) & 0x0F));
    return (Wire.endTransmission() == 0);
}

bool MotorController::writeRegister(uint8_t regAddr, uint8_t data) {
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

bool MotorController::readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    uint8_t count = Wire.requestFrom(address_, length);
    if (count != length) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
