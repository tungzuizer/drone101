#include "ImuSensor.h"

// Thanh ghi MPU6050
#define MPU_REG_SMPLRT_DIV      0x19
#define MPU_REG_CONFIG          0x1A
#define MPU_REG_GYRO_CONFIG     0x1B
#define MPU_REG_ACCEL_CONFIG    0x1C
#define MPU_REG_INT_PIN_CFG     0x37
#define MPU_REG_ACCEL_XOUT_H    0x3B
#define MPU_REG_USER_CTRL       0x6A
#define MPU_REG_PWR_MGMT_1      0x6B
#define MPU_REG_WHO_AM_I        0x75

ImuSensor::ImuSensor()
    : address_(I2C_ADDR_MPU6050_PRI),
      isHealthy_(false),
      orientation_(IMU_SENSOR_ORIENTATION),
      gyroOffsetX_(0.0f),
      gyroOffsetY_(0.0f),
      gyroOffsetZ_(0.0f),
      gyroScaleFactor_(MPU6050_GYRO_SCALE),
      accelScaleFactor_(MPU6050_ACCEL_SCALE) {
    memset(&data_, 0, sizeof(ImuData));
    memset(&rawData_, 0, sizeof(ImuRawData));
}

bool ImuSensor::begin(uint8_t i2cAddress) {
    address_ = i2cAddress;
    isHealthy_ = false;

    // 1. Kiểm tra sự hiện diện của MPU6050 qua thanh ghi WHO_AM_I (0x75)
    uint8_t whoAmI = 0;
    bool found = readRegisters(MPU_REG_WHO_AM_I, &whoAmI, 1);

    // Nếu không thấy tại địa chỉ chính (0x68), tự động thử địa chỉ phụ (0x69)
    if (!found && address_ == I2C_ADDR_MPU6050_PRI) {
        address_ = I2C_ADDR_MPU6050_ALT;
        found = readRegisters(MPU_REG_WHO_AM_I, &whoAmI, 1);
        if (found) {
            Serial.printf("[IMU INFO] Tìm thấy MPU6050 tại địa chỉ phụ AD0=3.3V (0x%02X)\n", address_);
        }
    }

    if (!found) {
        Serial.printf("[IMU ERROR] Không thể đọc thanh ghi WHO_AM_I tại cả 0x68 và 0x69 trên SDA=GPIO%d, SCL=GPIO%d!\n",
                      PIN_I2C_SDA, PIN_I2C_SCL);
        return false;
    }

    if (whoAmI != 0x68 && whoAmI != 0x70 && whoAmI != 0x71 && whoAmI != 0x72 && whoAmI != 0x73 && whoAmI != 0x98) {
        Serial.printf("[IMU WARN] WHO_AM_I nhận được là 0x%02X (Chuẩn MPU6050 là 0x68), tiếp tục khởi tạo...\n", whoAmI);
    }

    // 2. Reset MPU6050 để đưa về trạng thái sạch ban đầu
    if (!writeRegister(MPU_REG_PWR_MGMT_1, 0x80)) {
        Serial.println("[IMU ERROR] Không thể gửi lệnh Reset cho MPU6050!");
        return false;
    }
    delay(100);

    // 3. Đánh thức chip và chọn nguồn Clock tối ưu: PLL với trục Gyro X
    if (!writeRegister(MPU_REG_PWR_MGMT_1, 0x01)) {
        Serial.println("[IMU ERROR] Không thể đánh thức MPU6050 (Clock PLL)!");
        return false;
    }
    delay(15);

    // 4. Cấu hình Sample Rate Divider = 0 -> Tần số lấy mẫu nội bộ 1kHz
    if (!writeRegister(MPU_REG_SMPLRT_DIV, 0x00)) {
        return false;
    }

    // 5. Cấu hình DLPF (Digital Low Pass Filter) để lọc rung động từ 4 motor
    if (!writeRegister(MPU_REG_CONFIG, MPU6050_DLPF_CFG)) {
        return false;
    }

    // 6. Cấu hình Full Scale Range cho Gyroscope
    uint8_t gyroConfigVal = (MPU6050_GYRO_FS_SEL << 3);
    if (!writeRegister(MPU_REG_GYRO_CONFIG, gyroConfigVal)) {
        return false;
    }

    // 7. Cấu hình Full Scale Range cho Accelerometer
    uint8_t accelConfigVal = (MPU6050_ACCEL_FS_SEL << 3);
    if (!writeRegister(MPU_REG_ACCEL_CONFIG, accelConfigVal)) {
        return false;
    }

    // 8. Bật chế độ I2C Bypass (Cho phép Magnetometer kết nối trực tiếp)
    writeRegister(MPU_REG_USER_CTRL, 0x00);
    writeRegister(MPU_REG_INT_PIN_CFG, 0x02);

    // Cập nhật hệ số chia tỷ lệ tương ứng
    gyroScaleFactor_  = MPU6050_GYRO_SCALE;
    accelScaleFactor_ = MPU6050_ACCEL_SCALE;

    isHealthy_ = true;
    const char* orientStr = "DEFAULT (0°)";
    if (orientation_ == IMU_ALIGN_CW90) orientStr = "CW90 (X->Phải, Y->Sau)";
    else if (orientation_ == IMU_ALIGN_CW90_VERT) orientStr = "CW90_VERT (X->Lên, Y->Ngang, Dựng đứng)";
    else if (orientation_ == IMU_ALIGN_CW180) orientStr = "CW180 (X->Sau, Y->Phải)";
    else if (orientation_ == IMU_ALIGN_CCW90) orientStr = "CCW90 (X->Trái, Y->Trước)";

    Serial.printf("[IMU OK] Khởi tạo MPU6050 THÀNH CÔNG (Addr: 0x%02X, WHO_AM_I: 0x%02X, Align: %s, DLPF: %d)\n",
                  address_, whoAmI, orientStr, MPU6050_DLPF_CFG);

    return true;
}

bool ImuSensor::update() {
    if (!isHealthy_) {
        return false;
    }

    // Đọc liên tiếp 14 bytes (Burst Read) từ thanh ghi 0x3B:
    // 0x3B-0x3C: Accel X (H, L)
    // 0x3D-0x3E: Accel Y (H, L)
    // 0x3F-0x40: Accel Z (H, L)
    // 0x41-0x42: Temp (H, L)
    // 0x43-0x44: Gyro X (H, L)
    // 0x45-0x46: Gyro Y (H, L)
    // 0x47-0x48: Gyro Z (H, L)
    uint8_t buffer[14];
    if (!readRegisters(MPU_REG_ACCEL_XOUT_H, buffer, 14)) {
        isHealthy_ = false;
        return false;
    }

    // Ghép 2 bytes thành số nguyên có dấu 16-bit
    rawData_.rawAx   = (int16_t)((buffer[0] << 8) | buffer[1]);
    rawData_.rawAy   = (int16_t)((buffer[2] << 8) | buffer[3]);
    rawData_.rawAz   = (int16_t)((buffer[4] << 8) | buffer[5]);
    rawData_.rawTemp = (int16_t)((buffer[6] << 8) | buffer[7]);
    rawData_.rawGx   = (int16_t)((buffer[8] << 8) | buffer[9]);
    rawData_.rawGy   = (int16_t)((buffer[10] << 8) | buffer[11]);
    rawData_.rawGz   = (int16_t)((buffer[12] << 8) | buffer[13]);

    // Chuyển đổi sang đơn vị vật lý của cảm biến (Sensor Frame)
    float sAx = (float)rawData_.rawAx / accelScaleFactor_;
    float sAy = (float)rawData_.rawAy / accelScaleFactor_;
    float sAz = (float)rawData_.rawAz / accelScaleFactor_;

    // Công thức nhiệt độ MPU6050 từ Datasheet: Temp(°C) = (Raw / 340.0) + 36.53
    data_.temp = ((float)rawData_.rawTemp / 340.0f) + 36.53f;

    // Vận tốc góc sau khi trừ bias offset tĩnh trong Sensor Frame
    float sGx = ((float)rawData_.rawGx / gyroScaleFactor_) - gyroOffsetX_;
    float sGy = ((float)rawData_.rawGy / gyroScaleFactor_) - gyroOffsetY_;
    float sGz = ((float)rawData_.rawGz / gyroScaleFactor_) - gyroOffsetZ_;

    // Ánh xạ tọa độ Sensor sang Body Frame của Drone theo góc xoay orientation_
    switch (orientation_) {
        case IMU_ALIGN_CW90:
            // Xoay 90° thuận KĐH: Mũi tên X chỉ sang Phải (+Y_body), Mũi tên Y chỉ ra Sau (-X_body)
            data_.ax = -sAy;
            data_.ay =  sAx;
            data_.az =  sAz;
            data_.gx = -sGy;
            data_.gy =  sGx;
            data_.gz =  sGz;
            break;

        case IMU_ALIGN_CW90_VERT:
            // Dựng đứng + Xoay CW90: Trục X sensor hướng lên (+Z_body)
            // Sensor X (lên)   -> Body Z (lên, trọng lực)
            // Sensor Z (ngang) -> Body X (mũi drone, trục Roll)
            // Sensor Y (ngang) -> Body -Y (trục Pitch, đảo dấu theo quy ước tay phải)
            data_.ax =  sAz;
            data_.ay = -sAy;
            data_.az =  sAx;
            data_.gx =  sGz;
            data_.gy = -sGy;
            data_.gz =  sGx;
            break;

        case IMU_ALIGN_CCW90:
            // Xoay 90° ngược KĐH: Mũi tên X chỉ sang Trái (-Y_body), Mũi tên Y chỉ ra Trước (+X_body)
            data_.ax =  sAy;
            data_.ay = -sAx;
            data_.az =  sAz;
            data_.gx =  sGy;
            data_.gy = -sGx;
            data_.gz =  sGz;
            break;

        case IMU_ALIGN_CW180:
            // Xoay 180°: Mũi tên X chỉ ra Sau (-X_body), Mũi tên Y chỉ sang Phải (-Y_body)
            data_.ax = -sAx;
            data_.ay = -sAy;
            data_.az =  sAz;
            data_.gx = -sGx;
            data_.gy = -sGy;
            data_.gz =  sGz;
            break;

        case IMU_ALIGN_DEFAULT:
        default:
            // Mặc định: Mũi tên X chỉ ra Trước (+X_body), Mũi tên Y chỉ sang Phải (+Y_body)
            data_.ax = sAx;
            data_.ay = sAy;
            data_.az = sAz;
            data_.gx = sGx;
            data_.gy = sGy;
            data_.gz = sGz;
            break;
    }

    data_.timestampUs = micros();
    return true;
}

bool ImuSensor::calibrateGyro(uint16_t sampleCount) {
    if (!isHealthy_) {
        return false;
    }

    Serial.println("\n-------------------------------------------------------");
    Serial.printf(">>> BẮT ĐẦU HIỆU CHUẨN GYROSCOPE (%d MẪU) <<<\n", sampleCount);
    Serial.println(">>> YÊU CẦU: ĐẶT DRONE NẰM YÊN TUYỆT ĐỐI TRÊN MẶT PHẲNG <<<");
    Serial.println("-------------------------------------------------------");

    double sumGx = 0.0;
    double sumGy = 0.0;
    double sumGz = 0.0;

    // Tạm thời xóa offset cũ
    gyroOffsetX_ = 0.0f;
    gyroOffsetY_ = 0.0f;
    gyroOffsetZ_ = 0.0f;

    // Đợi 200ms để cảm biến ổn định
    delay(200);

    if (sampleCount < 10) sampleCount = 10;
    uint16_t progressStep = sampleCount / 10;

    uint16_t validSamples = 0;
    for (uint16_t i = 0; i < sampleCount; i++) {
        if (update()) {
            sumGx += (double)rawData_.rawGx / (double)gyroScaleFactor_;
            sumGy += (double)rawData_.rawGy / (double)gyroScaleFactor_;
            sumGz += (double)rawData_.rawGz / (double)gyroScaleFactor_;
            validSamples++;
        }
        delay(4); // Lấy mẫu ở tần số ~250Hz

        if (progressStep > 0 && (i % progressStep == 0)) {
            Serial.printf("  Tiến độ: %d%%\n", (i * 100) / sampleCount);
        }
    }

    if (validSamples < (sampleCount * 0.9f)) {
        Serial.println("[CALIB FAILED] Tỷ lệ mẫu hợp lệ quá thấp (dưới 90%)!");
        return false;
    }

    gyroOffsetX_ = (float)(sumGx / validSamples);
    gyroOffsetY_ = (float)(sumGy / validSamples);
    gyroOffsetZ_ = (float)(sumGz / validSamples);

    Serial.println("-------------------------------------------------------");
    Serial.println("[CALIB OK] Hiệu chuẩn hoàn tất!");
    Serial.printf("  Gyro Offset X: %+.4f °/s\n", gyroOffsetX_);
    Serial.printf("  Gyro Offset Y: %+.4f °/s\n", gyroOffsetY_);
    Serial.printf("  Gyro Offset Z: %+.4f °/s\n", gyroOffsetZ_);
    Serial.println("-------------------------------------------------------\n");

    return true;
}

void ImuSensor::setGyroOffsets(float offsetX, float offsetY, float offsetZ) {
    gyroOffsetX_ = offsetX;
    gyroOffsetY_ = offsetY;
    gyroOffsetZ_ = offsetZ;
}

bool ImuSensor::writeRegister(uint8_t regAddr, uint8_t data) {
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    Wire.write(data);
    return (Wire.endTransmission() == 0);
}

bool ImuSensor::readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length) {
    if (buffer == nullptr || length == 0) {
        return false;
    }
    Wire.beginTransmission(address_);
    Wire.write(regAddr);
    if (Wire.endTransmission(false) != 0) {
        // Thử lại với stop = true nếu repeated start không hỗ trợ
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
