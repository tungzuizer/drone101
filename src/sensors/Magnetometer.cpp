#include "Magnetometer.h"
#include <math.h>

// Thanh ghi HMC5883L (Honeywell 0x1E)
#define HMC_REG_CONFIG_A        0x00
#define HMC_REG_CONFIG_B        0x01
#define HMC_REG_MODE            0x02
#define HMC_REG_DATA_X_MSB      0x03
#define HMC_REG_ID_A            0x0A

// Thanh ghi QMC5883L (QST Clone 0x0D)
#define QMC_REG_DATA_X_LSB      0x00
#define QMC_REG_CONTROL_1       0x09
#define QMC_REG_SET_RESET       0x0B
#define QMC_REG_CHIP_ID         0x0D

Magnetometer::Magnetometer()
    : address_(0),
      chipType_(MAG_CHIP_UNKNOWN),
      isHealthy_(false),
      declinationDeg_(-1.45f), // Góc từ thiên mặc định tại Việt Nam (~ -1.45°)
      offsetX_(0.0f), offsetY_(0.0f), offsetZ_(0.0f),
      scaleX_(1.0f), scaleY_(1.0f), scaleZ_(1.0f) {
    memset(&data_, 0, sizeof(MagData));
}

bool Magnetometer::begin() {
    isHealthy_ = false;
    chipType_ = MAG_CHIP_UNKNOWN;

    // 1. Thử nhận diện chip QMC5883L / QMC5883P tại 0x0D (hoặc 0x0C, 0x2C)
    const uint8_t qmcAddrs[] = {0x0D, 0x0C, 0x2C};
    for (uint8_t addr : qmcAddrs) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            address_ = addr;
            chipType_ = MAG_CHIP_QMC5883L;
            if (initQMC5883L()) {
                isHealthy_ = true;
                Serial.printf("[MAG OK] Khởi tạo thành công chip QMC5883 (Địa chỉ: 0x%02X)\n", addr);
                return true;
            }
        }
    }

    // 2. Thử nhận diện chip HMC5883L / Clone tại 0x1E
    Wire.beginTransmission(I2C_ADDR_HMC5883L);
    if (Wire.endTransmission() == 0) {
        address_ = I2C_ADDR_HMC5883L;
        chipType_ = MAG_CHIP_HMC5883L;
        if (initHMC5883L()) {
            isHealthy_ = true;
            Serial.println("[MAG OK] Khởi tạo thành công chip HMC5883L (Địa chỉ: 0x1E)");
            return true;
        }
    }

    return false;
}

bool Magnetometer::initHMC5883L() {
    // Config A: 8 samples average, 75Hz data output rate, normal measurement -> 0x78
    if (!writeRegister(address_, HMC_REG_CONFIG_A, 0x78)) return false;
    // Config B: Gain ±1.3 Gauss (1090 LSB/Gauss) -> 0x20
    if (!writeRegister(address_, HMC_REG_CONFIG_B, 0x20)) return false;
    // Mode: Continuous measurement mode -> 0x00
    if (!writeRegister(address_, HMC_REG_MODE, 0x00)) return false;
    delay(20);
    return true;
}

bool Magnetometer::initQMC5883L() {
    // Set/Reset period: 0x01 theo khuyến nghị của datasheet
    if (!writeRegister(address_, QMC_REG_SET_RESET, 0x01)) return false;
    // Control 1: OSR=512 (0x00), RNG=±8G (0x10), ODR=200Hz (0x0C), MODE=Continuous (0x01) -> 0x1D
    if (!writeRegister(address_, QMC_REG_CONTROL_1, 0x1D)) return false;
    delay(20);
    return true;
}

bool Magnetometer::readHMC5883L(int16_t &rawX, int16_t &rawY, int16_t &rawZ) {
    uint8_t buf[6];
    // HMC5883L đọc 6 bytes bắt đầu từ 0x03 theo thứ tự: X_MSB, X_LSB, Z_MSB, Z_LSB, Y_MSB, Y_LSB
    if (!readRegisters(address_, HMC_REG_DATA_X_MSB, buf, 6)) {
        return false;
    }
    rawX = (int16_t)((buf[0] << 8) | buf[1]);
    rawZ = (int16_t)((buf[2] << 8) | buf[3]);
    rawY = (int16_t)((buf[4] << 8) | buf[5]);
    return true;
}

bool Magnetometer::readQMC5883L(int16_t &rawX, int16_t &rawY, int16_t &rawZ) {
    uint8_t buf[6];
    // QMC5883L đọc 6 bytes bắt đầu từ 0x00 theo thứ tự: X_LSB, X_MSB, Y_LSB, Y_MSB, Z_LSB, Z_MSB
    if (!readRegisters(address_, QMC_REG_DATA_X_LSB, buf, 6)) {
        return false;
    }
    rawX = (int16_t)((buf[1] << 8) | buf[0]);
    rawY = (int16_t)((buf[3] << 8) | buf[2]);
    rawZ = (int16_t)((buf[5] << 8) | buf[4]);
    return true;
}

bool Magnetometer::update() {
    if (!isHealthy_) {
        return false;
    }

    int16_t rawX = 0, rawY = 0, rawZ = 0;
    bool ok = false;

    if (chipType_ == MAG_CHIP_HMC5883L) {
        ok = readHMC5883L(rawX, rawY, rawZ);
        if (ok) {
            // Độ nhạy Gain ±1.3 Gauss = 1090 LSB/Gauss
            data_.mx = ((float)rawX / 1090.0f - offsetX_) * scaleX_;
            data_.my = ((float)rawY / 1090.0f - offsetY_) * scaleY_;
            data_.mz = ((float)rawZ / 1090.0f - offsetZ_) * scaleZ_;
        }
    } else if (chipType_ == MAG_CHIP_QMC5883L) {
        ok = readQMC5883L(rawX, rawY, rawZ);
        if (ok) {
            // Độ nhạy Range ±8 Gauss = 3000 LSB/Gauss
            data_.mx = ((float)rawX / 3000.0f - offsetX_) * scaleX_;
            data_.my = ((float)rawY / 3000.0f - offsetY_) * scaleY_;
            data_.mz = ((float)rawZ / 3000.0f - offsetZ_) * scaleZ_;
        }
    }

    if (!ok) {
        isHealthy_ = false;
        return false;
    }

    data_.timestampUs = micros();
    data_.heading = calculateHeading();
    return true;
}

float Magnetometer::calculateHeading() {
    // Tính góc heading phẳng (chưa bù nghiêng)
    float headingRad = atan2(-data_.my, data_.mx);

    // Bù góc từ thiên địa phương
    headingRad += (declinationDeg_ * (float)M_PI / 180.0f);

    // Chuẩn hóa về khoảng 0 đến 2*PI
    if (headingRad < 0) {
        headingRad += 2.0f * (float)M_PI;
    } else if (headingRad >= 2.0f * (float)M_PI) {
        headingRad -= 2.0f * (float)M_PI;
    }

    return headingRad * 180.0f / (float)M_PI;
}

float Magnetometer::calculateTiltCompensatedHeading(float rollRad, float pitchRad) {
    float cosRoll = cosf(rollRad);
    float sinRoll = sinf(rollRad);
    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);

    // Thuật toán chiếu từ trường 3D lên mặt phẳng nằm ngang (Tait-Bryan Z-Y-X projection)
    float compX = data_.mx * cosPitch + data_.my * sinRoll * sinPitch + data_.mz * cosRoll * sinPitch;
    float compY = data_.my * cosRoll - data_.mz * sinRoll;

    float headingRad = atan2f(-compY, compX);
    headingRad += (declinationDeg_ * (float)M_PI / 180.0f);

    if (headingRad < 0) {
        headingRad += 2.0f * (float)M_PI;
    } else if (headingRad >= 2.0f * (float)M_PI) {
        headingRad -= 2.0f * (float)M_PI;
    }

    data_.compensatedHeading = headingRad * 180.0f / (float)M_PI;
    return data_.compensatedHeading;
}

bool Magnetometer::calibrate(uint16_t durationSeconds) {
    if (!isHealthy_) return false;

    Serial.println("\n-------------------------------------------------------");
    Serial.printf(">>> BẮT ĐẦU HIỆU CHUẨN TỪ KẾ (%d GIÂY) <<<\n", durationSeconds);
    Serial.println(">>> YÊU CẦU: XOAY DRONE CHẬM RÃI MỌI HƯỚNG TRONG KHÔNG GIAN (HÌNH SỐ 8) <<<");
    Serial.println("-------------------------------------------------------");

    float minX = 9999.0f, maxX = -9999.0f;
    float minY = 9999.0f, maxY = -9999.0f;
    float minZ = 9999.0f, maxZ = -9999.0f;

    // Reset hiệu chuẩn tạm thời
    offsetX_ = 0; offsetY_ = 0; offsetZ_ = 0;
    scaleX_ = 1; scaleY_ = 1; scaleZ_ = 1;

    unsigned long startTime = millis();
    unsigned long durationMs = durationSeconds * 1000UL;
    unsigned long lastPrint = 0;

    while (millis() - startTime < durationMs) {
        if (update()) {
            if (data_.mx < minX) minX = data_.mx;
            if (data_.mx > maxX) maxX = data_.mx;
            if (data_.my < minY) minY = data_.my;
            if (data_.my > maxY) maxY = data_.my;
            if (data_.mz < minZ) minZ = data_.mz;
            if (data_.mz > maxZ) maxZ = data_.mz;
        }
        delay(10);

        if (millis() - lastPrint > 1000) {
            uint16_t elapsed = (millis() - startTime) / 1000;
            Serial.printf("  Thời gian: %d/%d giây | X:[%.2f, %.2f] Y:[%.2f, %.2f] Z:[%.2f, %.2f]\n",
                          elapsed, durationSeconds, minX, maxX, minY, maxY, minZ, maxZ);
            lastPrint = millis();
        }
    }

    // Tính Hard-Iron Offset (Tâm ellipse)
    offsetX_ = (maxX + minX) / 2.0f;
    offsetY_ = (maxY + minY) / 2.0f;
    offsetZ_ = (maxZ + minZ) / 2.0f;

    // Tính Soft-Iron Scale (Bán kính trung bình)
    float deltaX = (maxX - minX) / 2.0f;
    float deltaY = (maxY - minY) / 2.0f;
    float deltaZ = (maxZ - minZ) / 2.0f;
    float avgDelta = (deltaX + deltaY + deltaZ) / 3.0f;

    if (deltaX > 0.01f) scaleX_ = avgDelta / deltaX;
    if (deltaY > 0.01f) scaleY_ = avgDelta / deltaY;
    if (deltaZ > 0.01f) scaleZ_ = avgDelta / deltaZ;

    Serial.println("-------------------------------------------------------");
    Serial.println("[MAG CALIB OK] Hiệu chuẩn từ kế thành công!");
    Serial.printf("  Offset : X=%+.3f, Y=%+.3f, Z=%+.3f Gauss\n", offsetX_, offsetY_, offsetZ_);
    Serial.printf("  Scale  : X=%.3f, Y=%.3f, Z=%.3f\n", scaleX_, scaleY_, scaleZ_);
    Serial.println("-------------------------------------------------------\n");

    return true;
}

void Magnetometer::setCalibration(float offX, float offY, float offZ, float scaleX, float scaleY, float scaleZ) {
    offsetX_ = offX; offsetY_ = offY; offsetZ_ = offZ;
    scaleX_ = scaleX; scaleY_ = scaleY; scaleZ_ = scaleZ;
}

bool Magnetometer::writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    Wire.beginTransmission(devAddr);
    Wire.write(regAddr);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

bool Magnetometer::readRegisters(uint8_t devAddr, uint8_t regAddr, uint8_t* buffer, uint8_t length) {
    Wire.beginTransmission(devAddr);
    Wire.write(regAddr);
    if (Wire.endTransmission(false) != 0) {
        // Thử lại với stop condition nếu chip không hỗ trợ repeated-start
        Wire.beginTransmission(devAddr);
        Wire.write(regAddr);
        if (Wire.endTransmission(true) != 0) {
            return false;
        }
    }
    uint8_t count = Wire.requestFrom((int)devAddr, (int)length);
    if (count != length) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
