#ifndef GPS_READER_H
#define GPS_READER_H

#include <Arduino.h>
#include "../config.h"

// Dữ liệu định vị toàn cầu GPS / Beidou từ ATGM336H
struct GpsData {
    double latitude;        // Vĩ độ (Độ thập phân - Decimal Degrees, ví dụ: 21.028511)
    double longitude;       // Kinh độ (Độ thập phân, ví dụ: 105.804817)
    float altitude;         // Độ cao so với mực nước biển (m)
    float speedKmh;         // Tốc độ di chuyển mặt đất (km/h)
    float courseDeg;        // Hướng di chuyển (0 - 360°)
    float hdop;             // Độ chính xác vị trí mặt phẳng (HDOP < 2.0 là rất tốt)
    uint8_t satellites;     // Số lượng vệ tinh bắt được
    bool fixValid;          // Đã có Fix 3D hợp lệ hay chưa
    uint32_t timestampUs;
};

class GpsReader {
public:
    GpsReader(HardwareSerial& uart = Serial1);

    // Khởi tạo UART1 đọc GPS ở 9600 baud với chân RX=GPIO17, TX=GPIO18
    bool begin(int8_t rxPin = PIN_GPS_RX, int8_t txPin = PIN_GPS_TX, uint32_t baudRate = GPS_BAUDRATE);

    // Đọc luồng dữ liệu NMEA non-blocking trong vòng lặp chính
    bool update();

    // Lấy dữ liệu GPS hiện tại
    const GpsData& getData() const { return data_; }
    double getLatitude() const { return data_.latitude; }
    double getLongitude() const { return data_.longitude; }
    float getAltitude() const { return data_.altitude; }
    uint8_t getSatellites() const { return data_.satellites; }
    bool hasFix() const { return data_.fixValid; }
    bool isHealthy() const { return isHealthy_; }

private:
    HardwareSerial& uart_;
    GpsData data_;
    bool isHealthy_;

    char sentenceBuffer_[128];
    uint8_t sentenceIndex_;
    uint32_t lastByteTimeMs_;

    void parseNmeaSentence(const char* sentence);
    void parseGga(const char* sentence);
    void parseRmc(const char* sentence);
    double convertNmeaToDecimal(const char* nmeaCoord, char hemisphere);
    bool verifyChecksum(const char* sentence);
};

#endif // GPS_READER_H
