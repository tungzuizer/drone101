#include "Barometer.h"
#include <math.h>

// Thanh ghi BMP280
#define BMP_REG_CALIB_START     0x88    // Bắt đầu 24 bytes hiệu chuẩn
#define BMP_REG_CHIP_ID         0xD0    // 0x58 cho BMP280, 0x60 cho BME280
#define BMP_REG_RESET           0xE0    // Ghi 0xB6 để soft reset
#define BMP_REG_STATUS          0xF3
#define BMP_REG_CTRL_MEAS       0xF4    // Cấu hình Oversampling & Mode
#define BMP_REG_CONFIG          0xF5    // Cấu hình IIR Filter & Standby
#define BMP_REG_PRESS_MSB       0xF7    // Bắt đầu 6 bytes dữ liệu thô (Pressure + Temp)

Barometer::Barometer()
    : address_(I2C_ADDR_BMP280_PRI),
      isHealthy_(false),
      t_fine_(0),
      seaLevelPressure_(1013.25f), // 1013.25 hPa tiêu chuẩn
      groundPressure_(1013.25f) {
    memset(&data_, 0, sizeof(BaroData));
    memset(&calib_, 0, sizeof(Bmp280CalibParams));
}

bool Barometer::begin(uint8_t i2cAddress) {
    address_ = (i2cAddress != 0) ? i2cAddress : I2C_ADDR_BMP280_PRI;
    isHealthy_ = false;

    // 1. Kiểm tra Chip ID
    uint8_t chipId = 0;
    if (!readRegisters(BMP_REG_CHIP_ID, &chipId, 1)) {
        // Thử địa chỉ phụ nếu địa chỉ chính không phản hồi
        if (address_ == I2C_ADDR_BMP280_PRI) {
            address_ = I2C_ADDR_BMP280_ALT;
            if (!readRegisters(BMP_REG_CHIP_ID, &chipId, 1)) {
                Serial.println("[BARO ERROR] Không tìm thấy BMP280 ở cả 0x76 và 0x77!");
                return false;
            }
        } else {
            return false;
        }
    }

    if (chipId != 0x58 && chipId != 0x60 && chipId != 0x56 && chipId != 0x57) {
        Serial.printf("[BARO WARN] Chip ID lạ (0x%02X), tiếp tục thử khởi tạo...\n", chipId);
    }

    // 2. Soft Reset BMP280
    writeRegister(BMP_REG_RESET, 0xB6);
    delay(100);

    // 3. Đọc bảng tham số hiệu chuẩn từ nhà máy (24 bytes)
    if (!readCalibrationData()) {
        Serial.println("[BARO ERROR] Không thể đọc bảng tham số hiệu chuẩn BMP280!");
        return false;
    }

    // 4. Cấu hình IIR Filter & Standby time:
    // Standby = 0.5ms (000), Filter Coefficient = 16 (100) -> 0x10
    if (!writeRegister(BMP_REG_CONFIG, 0x10)) {
        return false;
    }

    // 5. Cấu hình chế độ hoạt động và Oversampling:
    // osrs_t = x2 (010), osrs_p = x16 (101), Mode = Normal (11) -> 0x57
    if (!writeRegister(BMP_REG_CTRL_MEAS, 0x57)) {
        return false;
    }

    isHealthy_ = true;
    Serial.printf("[BARO OK] Khởi tạo BMP280 thành công (Addr: 0x%02X, Chip ID: 0x%02X, IIR Filter=16)\n",
                  address_, chipId);

    // Lấy áp suất nền mặt đất ban đầu
    setGroundReference(30);
    return true;
}

bool Barometer::readCalibrationData() {
    uint8_t buf[24];
    if (!readRegisters(BMP_REG_CALIB_START, buf, 24)) {
        return false;
    }

    // Little-Endian (LSB trước, MSB sau)
    calib_.dig_T1 = (uint16_t)((buf[1] << 8) | buf[0]);
    calib_.dig_T2 = (int16_t)((buf[3] << 8) | buf[2]);
    calib_.dig_T3 = (int16_t)((buf[5] << 8) | buf[4]);

    calib_.dig_P1 = (uint16_t)((buf[7] << 8) | buf[6]);
    calib_.dig_P2 = (int16_t)((buf[9] << 8) | buf[8]);
    calib_.dig_P3 = (int16_t)((buf[11] << 8) | buf[10]);
    calib_.dig_P4 = (int16_t)((buf[13] << 8) | buf[12]);
    calib_.dig_P5 = (int16_t)((buf[15] << 8) | buf[14]);
    calib_.dig_P6 = (int16_t)((buf[17] << 8) | buf[16]);
    calib_.dig_P7 = (int16_t)((buf[19] << 8) | buf[18]);
    calib_.dig_P8 = (int16_t)((buf[21] << 8) | buf[20]);
    calib_.dig_P9 = (int16_t)((buf[23] << 8) | buf[22]);

    return true;
}

bool Barometer::update() {
    if (!isHealthy_) {
        return false;
    }

    // Đọc 6 bytes liên tiếp: Press MSB, LSB, XLSB (0xF7-0xF9) và Temp MSB, LSB, XLSB (0xFA-0xFC)
    uint8_t buf[6];
    if (!readRegisters(BMP_REG_PRESS_MSB, buf, 6)) {
        isHealthy_ = false;
        return false;
    }

    int32_t rawPressure = (int32_t)((((uint32_t)buf[0]) << 12) | (((uint32_t)buf[1]) << 4) | (((uint32_t)buf[2]) >> 4));
    int32_t rawTemp     = (int32_t)((((uint32_t)buf[3]) << 12) | (((uint32_t)buf[4]) << 4) | (((uint32_t)buf[5]) >> 4));

    // Bắt buộc tính nhiệt độ trước để cập nhật biến t_fine_
    data_.temperature = compensateTemperature(rawTemp);
    data_.pressure    = compensatePressure(rawPressure);
    data_.altitude    = calculateAltitude(data_.pressure, seaLevelPressure_);
    data_.relativeAltitude = calculateAltitude(data_.pressure, groundPressure_);
    data_.timestampUs = micros();

    return true;
}

float Barometer::compensateTemperature(int32_t rawTemp) {
    int32_t var1 = ((((rawTemp >> 3) - ((int32_t)calib_.dig_T1 << 1))) * ((int32_t)calib_.dig_T2)) >> 11;
    int32_t var2 = (((((rawTemp >> 4) - ((int32_t)calib_.dig_T1)) *
                      ((rawTemp >> 4) - ((int32_t)calib_.dig_T1))) >> 12) *
                    ((int32_t)calib_.dig_T3)) >> 14;

    t_fine_ = var1 + var2;
    int32_t T = (t_fine_ * 5 + 128) >> 8;
    return (float)T / 100.0f;
}

float Barometer::compensatePressure(int32_t rawPressure) {
    int64_t var1 = ((int64_t)t_fine_) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)calib_.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib_.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib_.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_.dig_P3) >> 8) + ((var1 * (int64_t)calib_.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_.dig_P1) >> 33;

    if (var1 == 0) {
        return 0.0f; // Tránh chia cho 0
    }

    int64_t p = 1048576 - rawPressure;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_.dig_P7) << 4);

    return (float)p / 25600.0f; // Trả về áp suất theo đơn vị hPa
}

float Barometer::calculateAltitude(float pressureHpa, float referenceHpa) {
    if (pressureHpa <= 0 || referenceHpa <= 0) return 0.0f;
    // Công thức khí áp Hypsometric: Alt = 44330 * (1 - (P/P0)^(1/5.255))
    return 44330.0f * (1.0f - powf(pressureHpa / referenceHpa, 0.1902949f));
}

void Barometer::setGroundReference(uint16_t sampleCount) {
    if (!isHealthy_) return;

    double sumP = 0.0;
    uint16_t valid = 0;

    for (uint16_t i = 0; i < sampleCount; i++) {
        if (update()) {
            sumP += data_.pressure;
            valid++;
        }
        delay(10);
    }

    if (valid > 0) {
        groundPressure_ = (float)(sumP / valid);
        Serial.printf("[BARO] Đã thiết lập áp suất mốc mặt đất: %.2f hPa (0.00 m)\n", groundPressure_);
    }
}

bool Barometer::writeRegister(uint8_t regAddr, uint8_t data) {
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

bool Barometer::readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length) {
    if (buffer == nullptr || length == 0) {
        return false;
    }
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    if (Wire.endTransmission(false) != 0) {
        Wire.beginTransmission(address_);
        Wire.write(regAddr);
        if (Wire.endTransmission(true) != 0) {
            return false;
        }
    }
    uint8_t count = Wire.requestFrom((int)address_, (int)length);
    if (count != length) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        buffer[i] = Wire.read();
    }
    return true;
}
