#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "../config.h"

// Cấu trúc chứa dữ liệu cảm biến sau khi quy đổi ra đơn vị vật lý chuẩn
struct ImuData {
    float ax, ay, az;       // Gia tốc 3 trục theo đơn vị g (1g ≈ 9.81 m/s²)
    float gx, gy, gz;       // Vận tốc góc 3 trục theo đơn vị độ/giây (°/s)
    float temp;             // Nhiệt độ chip (°C)
    uint32_t timestampUs;   // Thời gian đo (Microseconds)
};

// Cấu trúc chứa dữ liệu thô (Raw 16-bit)
struct ImuRawData {
    int16_t rawAx, rawAy, rawAz;
    int16_t rawGx, rawGy, rawGz;
    int16_t rawTemp;
};

class ImuSensor {
public:
    ImuSensor();

    // Khởi tạo MPU6050, cấu hình Full Scale Range và bộ lọc DLPF
    bool begin(uint8_t i2cAddress = I2C_ADDR_MPU6050_PRI);

    // Đọc đồng thời 14 bytes (Burst Read) trong một chu kỳ I2C duy nhất
    bool update();

    // Tự động lấy mẫu và tính toán độ lệch tĩnh (Bias/Offset) cho Gyroscope
    bool calibrateGyro(uint16_t sampleCount = MPU6050_CALIB_SAMPLES);

    // Cài đặt trực tiếp Offset
    void setGyroOffsets(float offsetX, float offsetY, float offsetZ);

    // Lấy dữ liệu cảm biến
    const ImuData& getData() const { return data_; }
    const ImuRawData& getRawData() const { return rawData_; }

    // Trạng thái sức khỏe cảm biến
    bool isHealthy() const { return isHealthy_; }
    uint8_t getAddress() const { return address_; }

    // Offset hiện tại
    float getGyroOffsetX() const { return gyroOffsetX_; }
    float getGyroOffsetY() const { return gyroOffsetY_; }
    float getGyroOffsetZ() const { return gyroOffsetZ_; }

private:
    uint8_t address_;
    bool isHealthy_;

    ImuData data_;
    ImuRawData rawData_;

    // Offset hiệu chuẩn Gyro
    float gyroOffsetX_;
    float gyroOffsetY_;
    float gyroOffsetZ_;

    // Hệ số tỷ lệ chia theo cấu hình thanh ghi
    float gyroScaleFactor_;
    float accelScaleFactor_;

    // Ghi và đọc thanh ghi I2C
    bool writeRegister(uint8_t regAddr, uint8_t data);
    bool readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length);
};

#endif // IMU_SENSOR_H
