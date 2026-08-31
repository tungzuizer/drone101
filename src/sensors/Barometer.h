#ifndef BAROMETER_H
#define BAROMETER_H

#include <Arduino.h>
#include <Wire.h>
#include "../config.h"

struct BaroData {
    float pressure;             // Áp suất khí quyển (hPa)
    float temperature;          // Nhiệt độ khí quyển (°C)
    float altitude;             // Độ cao tuyệt đối so với mực nước biển chuẩn (m)
    float relativeAltitude;     // Độ cao tương đối so với mặt đất lúc cất cánh (m)
    uint32_t timestampUs;
};

// Hệ số hiệu chuẩn từ nhà máy Bosch Sensortec trong ROM của BMP280
struct Bmp280CalibParams {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
};

class Barometer {
public:
    Barometer();

    // Khởi tạo BMP280 (tự động thử 0x76 và 0x77), nạp bảng Calib và cấu hình IIR Filter
    bool begin(uint8_t i2cAddress = I2C_ADDR_BMP280_PRI);

    // Đọc áp suất, nhiệt độ và cập nhật độ cao
    bool update();

    // Đặt áp suất mặt đất lúc xuất phát làm mốc 0 mét (Takeoff Ground Zero)
    void setGroundReference(uint16_t sampleCount = 50);

    // Lấy dữ liệu
    const BaroData& getData() const { return data_; }
    float getPressure() const { return data_.pressure; }
    float getTemperature() const { return data_.temperature; }
    float getAltitude() const { return data_.altitude; }
    float getRelativeAltitude() const { return data_.relativeAltitude; }
    bool isHealthy() const { return isHealthy_; }
    uint8_t getAddress() const { return address_; }

    // Áp suất mặt biển tiêu chuẩn (mặc định 1013.25 hPa)
    void setSeaLevelPressure(float seaLevelHpa) { seaLevelPressure_ = seaLevelHpa; }

private:
    uint8_t address_;
    bool isHealthy_;

    BaroData data_;
    Bmp280CalibParams calib_;
    int32_t t_fine_; // Biến phụ trợ nhiệt độ dùng cho bù áp suất
    float seaLevelPressure_;
    float groundPressure_;

    bool readCalibrationData();
    float compensateTemperature(int32_t rawTemp);
    float compensatePressure(int32_t rawPressure);
    float calculateAltitude(float pressureHpa, float referenceHpa);

    bool writeRegister(uint8_t regAddr, uint8_t data);
    bool readRegisters(uint8_t regAddr, uint8_t* buffer, uint8_t length);
};

#endif // BAROMETER_H
