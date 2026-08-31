#include "GpsReader.h"
#include <string.h>
#include <stdlib.h>

GpsReader::GpsReader(HardwareSerial& uart)
    : uart_(uart),
      isHealthy_(false),
      sentenceIndex_(0),
      lastByteTimeMs_(0) {
    memset(&data_, 0, sizeof(GpsData));
}

bool GpsReader::begin(int8_t rxPin, int8_t txPin, uint32_t baudRate) {
    // Khởi tạo UART1 cho ATGM336H
    uart_.begin(baudRate, SERIAL_8N1, rxPin, txPin);
    sentenceIndex_ = 0;
    sentenceBuffer_[0] = '\0';
    isHealthy_ = true;
    lastByteTimeMs_ = millis();
    Serial.printf("[GPS OK] Khởi tạo UART1 cho ATGM336H (RX=%d, TX=%d, Baud: %d)\n", rxPin, txPin, baudRate);
    return true;
}

bool GpsReader::update() {
    bool newFix = false;

    while (uart_.available() > 0) {
        char c = (char)uart_.read();
        lastByteTimeMs_ = millis();

        if (c == '$') {
            // Bắt đầu câu NMEA mới
            sentenceIndex_ = 0;
            sentenceBuffer_[sentenceIndex_++] = c;
        } else if (c == '\n' || c == '\r') {
            if (sentenceIndex_ > 5) {
                sentenceBuffer_[sentenceIndex_] = '\0';
                if (verifyChecksum(sentenceBuffer_)) {
                    parseNmeaSentence(sentenceBuffer_);
                    newFix = true;
                }
                sentenceIndex_ = 0;
            }
        } else {
            if (sentenceIndex_ < sizeof(sentenceBuffer_) - 1) {
                sentenceBuffer_[sentenceIndex_++] = c;
            }
        }
    }

    // Nếu quá 3 giây không nhận được byte nào từ GPS -> Đánh dấu mất kết nối
    if (millis() - lastByteTimeMs_ > 3000UL) {
        isHealthy_ = false;
        data_.fixValid = false;
    } else {
        isHealthy_ = true;
    }

    return newFix;
}

bool GpsReader::verifyChecksum(const char* sentence) {
    const char* star = strchr(sentence, '*');
    if (!star) return false;

    // Tính checksum XOR từ sau ký tự '$' đến trước '*'
    uint8_t calculated = 0;
    for (const char* p = sentence + 1; p < star; p++) {
        calculated ^= (uint8_t)(*p);
    }

    // Đọc 2 ký tự Hex sau '*'
    uint8_t expected = (uint8_t)strtol(star + 1, NULL, 16);
    return (calculated == expected);
}

void GpsReader::parseNmeaSentence(const char* sentence) {
    if (strncmp(sentence, "$GNGGA", 6) == 0 || strncmp(sentence, "$GPGGA", 6) == 0) {
        parseGga(sentence);
    } else if (strncmp(sentence, "$GNRMC", 6) == 0 || strncmp(sentence, "$GPRMC", 6) == 0) {
        parseRmc(sentence);
    }
}

void GpsReader::parseGga(const char* sentence) {
    // Phân tách các trường dấu phẩy
    char copy[128];
    strncpy(copy, sentence, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* tokens[15];
    int tokenCount = 0;
    char* p = copy;

    while (tokenCount < 15) {
        tokens[tokenCount++] = p;
        char* comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }

    if (tokenCount >= 10) {
        // Trường 6: Trạng thái Fix (0 = Invalid, 1 = GPS Fix, 2 = DGPS Fix)
        int fixQuality = atoi(tokens[6]);
        data_.fixValid = (fixQuality > 0);

        if (data_.fixValid && strlen(tokens[2]) > 0 && strlen(tokens[4]) > 0) {
            data_.latitude  = convertNmeaToDecimal(tokens[2], tokens[3][0]);
            data_.longitude = convertNmeaToDecimal(tokens[4], tokens[5][0]);
        }

        // Trường 7: Số lượng vệ tinh
        data_.satellites = (uint8_t)atoi(tokens[7]);

        // Trường 8: HDOP
        data_.hdop = (float)atof(tokens[8]);

        // Trường 9: Độ cao so với mực nước biển (Altitude MSL)
        data_.altitude = (float)atof(tokens[9]);

        data_.timestampUs = micros();
    }
}

void GpsReader::parseRmc(const char* sentence) {
    char copy[128];
    strncpy(copy, sentence, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char* tokens[15];
    int tokenCount = 0;
    char* p = copy;

    while (tokenCount < 15) {
        tokens[tokenCount++] = p;
        char* comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }

    if (tokenCount >= 9) {
        // Trường 2: Trạng thái 'A' = Active/Valid, 'V' = Void
        if (tokens[2][0] == 'A') {
            data_.fixValid = true;
            if (strlen(tokens[3]) > 0 && strlen(tokens[5]) > 0) {
                data_.latitude  = convertNmeaToDecimal(tokens[3], tokens[4][0]);
                data_.longitude = convertNmeaToDecimal(tokens[5], tokens[6][0]);
            }

            // Trường 7: Tốc độ theo knots (1 knot = 1.852 km/h)
            float speedKnots = (float)atof(tokens[7]);
            data_.speedKmh = speedKnots * 1.852f;

            // Trường 8: Hướng di chuyển (Track course)
            data_.courseDeg = (float)atof(tokens[8]);

            data_.timestampUs = micros();
        }
    }
}

double GpsReader::convertNmeaToDecimal(const char* nmeaCoord, char hemisphere) {
    double raw = atof(nmeaCoord);
    // Định dạng: ddmm.mmmm (Vĩ độ) hoặc dddmm.mmmm (Kinh độ)
    int degrees = (int)(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    double decimal = (double)degrees + (minutes / 60.0);

    if (hemisphere == 'S' || hemisphere == 'W') {
        decimal = -decimal;
    }
    return decimal;
}
