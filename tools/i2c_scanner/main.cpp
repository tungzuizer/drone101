#include <Arduino.h>
#include <Wire.h>

// Các cặp chân I2C thông dụng trên ESP32-S3
struct I2cPinPair {
    const char* name;
    uint8_t sda;
    uint8_t scl;
};

const I2cPinPair PIN_PAIRS[] = {
    {"Cặp chân mặc định dự án (GPIO8/GPIO9)", 8, 9},
    {"Cặp chân phụ A (GPIO1/GPIO2)",          1, 2},
    {"Cặp chân phụ B (GPIO41/GPIO42)",        41, 42},
    {"Cặp chân phụ C (GPIO11/GPIO12)",        11, 12},
    {"Cặp chân phụ D (GPIO21/GPIO47)",        21, 47}
};
const uint8_t NUM_PAIRS = sizeof(PIN_PAIRS) / sizeof(PIN_PAIRS[0]);

void checkPinVoltages(uint8_t sdaPin, uint8_t sclPin) {
    Wire.end();
    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);
    delay(10);
    int sda = digitalRead(sdaPin);
    int scl = digitalRead(sclPin);

    Serial.printf("  -> Trạng thái điện áp: SDA (GPIO%d)=%s | SCL (GPIO%d)=%s\n",
                  sdaPin, sda ? "3.3V (HIGH) [OK]" : "0V (LOW) [LỖI: Chập Mass hoặc lỏng dây!]",
                  sclPin, scl ? "3.3V (HIGH) [OK]" : "0V (LOW) [LỖI: Chập Mass hoặc lỏng dây!]");
}

bool scanPair(uint8_t sdaPin, uint8_t sclPin, uint32_t freq) {
    Wire.end();
    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);
    delay(10);

    if (digitalRead(sdaPin) == LOW || digitalRead(sclPin) == LOW) {
        // Thử phát xung giải phóng bus
        pinMode(sclPin, OUTPUT);
        for (int i = 0; i < 9; i++) {
            digitalWrite(sclPin, LOW); delayMicroseconds(10);
            digitalWrite(sclPin, HIGH); delayMicroseconds(10);
        }
        pinMode(sdaPin, INPUT_PULLUP);
        pinMode(sclPin, INPUT_PULLUP);
        delay(10);
    }

    if (digitalRead(sdaPin) == LOW || digitalRead(sclPin) == LOW) {
        Serial.printf("  [BỎ QUA] Bus SDA=%d, SCL=%d bị kẹt 0V (GND).\n", sdaPin, sclPin);
        return false;
    }

    Wire.begin(sdaPin, sclPin, freq);
    Wire.setTimeOut(20);

    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            found++;
            Serial.printf("  ✔ [TÌM THẤY] Địa chỉ: 0x%02X | ", addr);
            switch (addr) {
                case 0x68: Serial.println("MPU6050 IMU (AD0=GND)"); break;
                case 0x69: Serial.println("MPU6050 IMU (AD0=3.3V)"); break;
                case 0x76: Serial.println("BMP280 Barometer (SDO=GND)"); break;
                case 0x77: Serial.println("BMP280 Barometer (SDO=3.3V)"); break;
                case 0x0D: Serial.println("QMC5883L / DA5883 Compass"); break;
                case 0x1E: Serial.println("HMC5883L Compass"); break;
                case 0x40: Serial.println("PCA9685 PWM Driver"); break;
                case 0x70: Serial.println("PCA9685 All-Call"); break;
                default:   Serial.println("Thiết bị I2C khác"); break;
            }
        }
    }
    if (found == 0) {
        Serial.println("  (Không phát hiện thiết bị nào trên cặp chân này)");
    }
    return (found > 0);
}

void runFullDiagnostics() {
    Serial.println("\n=======================================================");
    Serial.println("     ESP32-S3 TOÀN DIỆN CHẨN ĐOÁN PHẦN CỨNG I2C        ");
    Serial.println("=======================================================");

    for (uint8_t i = 0; i < NUM_PAIRS; i++) {
        Serial.printf("\n[%d/%d] Kiểm tra: %s (SDA=%d, SCL=%d):\n",
                      i + 1, NUM_PAIRS, PIN_PAIRS[i].name, PIN_PAIRS[i].sda, PIN_PAIRS[i].scl);
        checkPinVoltages(PIN_PAIRS[i].sda, PIN_PAIRS[i].scl);
        scanPair(PIN_PAIRS[i].sda, PIN_PAIRS[i].scl, 100000);
    }
    Serial.println("\n=======================================================");
    Serial.println("Hướng dẫn kiểm tra nhanh:");
    Serial.println("1. Rút hết tất cả cảm biến, chỉ cắm duy nhất 1 module MPU6050.");
    Serial.println("2. Đấu nối chuẩn 4 dây MPU6050:");
    Serial.println("   VCC -> 3.3V (hoặc 5V nếu module có IC nguồn 662K)");
    Serial.println("   GND -> GND");
    Serial.println("   SDA -> GPIO 8");
    Serial.println("   SCL -> GPIO 9");
    Serial.println("=======================================================\n");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    runFullDiagnostics();
}

void loop() {
    if (Serial.available()) {
        while (Serial.available()) Serial.read();
        runFullDiagnostics();
    }
    delay(4000);
    runFullDiagnostics();
}

