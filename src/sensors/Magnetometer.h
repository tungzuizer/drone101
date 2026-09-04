#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <Arduino.h>
#include <Wire.h>
#include "../config.h"

enum MagChipType {
    MAG_CHIP_UNKNOWN = 0,
    MAG_CHIP_HMC5883L,  // 0x1E Honeywell
    MAG_CHIP_QMC5883L   // 0x0D QST Clone
};

struct MagData {
    float mx, my, mz;           // Cảm ứng từ 3 trục theo Gauss
    float heading;              // Góc hướng la bàn (0 - 360 độ)
    float compensatedHeading;   // Góc hướng đã bù nghiêng qua Pitch/Roll (0 - 360 độ)
    uint32_t timestampUs;
};

class Magnetometer {
public:
    Magnetometer();

    // Tự động phát hiện và khởi tạo HMC5883L (0x1E) hoặc QMC5883L/P (0x0D, 0x0C, 0x2C...)
    bool begin(uint8_t targetAddr = 0);

    // Đọc dữ liệu từ kế và cập nhật từ trường 3 trục
    bool update();

    // Tính góc hướng Heading phẳng
    float calculateHeading();

    // Tính góc hướng có bù góc nghiêng Roll và Pitch (Radian)
    float calculateTiltCompensatedHeading(float rollRad, float pitchRad);

    // Hiệu chuẩn Hard-Iron & Soft-Iron (Xoay drone vòng tròn 360 độ)
    bool calibrate(uint16_t durationSeconds = 15);

    // Cài đặt trực tiếp Offset và Scale
    void setCalibration(float offX, float offY, float offZ, float scaleX, float scaleY, float scaleZ);

    // Lấy dữ liệu
    const MagData& getData() const { return data_; }
    MagChipType getChipType() const { return chipType_; }
    bool isHealthy() const { return isHealthy_; }
    uint8_t getAddress() const { return address_; }

    // Góc từ thiên (Magnetic Declination) tại Việt Nam (khoảng -1.5 độ)
    void setDeclinationAngle(float declinationDeg) { declinationDeg_ = declinationDeg; }

private:
    uint8_t address_;
    MagChipType chipType_;
    bool isHealthy_;

    MagData data_;
    float declinationDeg_; // Góc từ thiên

    // Tham số hiệu chuẩn Hard-Iron (Offset) & Soft-Iron (Scale)
    float offsetX_, offsetY_, offsetZ_;
    float scaleX_, scaleY_, scaleZ_;

    bool initHMC5883L();
    bool initQMC5883L();

    bool readHMC5883L(int16_t &rawX, int16_t &rawY, int16_t &rawZ);
    bool readQMC5883L(int16_t &rawX, int16_t &rawY, int16_t &rawZ);

    bool writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t data);
    bool readRegisters(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint8_t length);
};

#endif // MAGNETOMETER_H
