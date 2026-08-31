#include <Arduino.h>
#include <Wire.h>

// Sơ đồ chân I2C trên ESP32-S3 theo chuẩn thiết kế
#define PIN_I2C_SDA 8
#define PIN_I2C_SCL 9
#define I2C_FREQ    400000UL // 400kHz Fast Mode

// Danh mục địa chỉ I2C dự kiến
#define ADDR_PCA9685     0x40
#define ADDR_MPU6050_PRI 0x68
#define ADDR_MPU6050_SEC 0x69
#define ADDR_HMC5883L    0x1E
#define ADDR_QMC5883L    0x0D
#define ADDR_BMP280_PRI  0x76
#define ADDR_BMP280_SEC  0x77

void scanI2CBus() {
    Serial.println("\n-----------------------------------------------------------");
    Serial.printf("[SCAN] Bắt đầu quét Bus I2C (SDA=GPIO%d, SCL=GPIO%d @ 400kHz)...\n", PIN_I2C_SDA, PIN_I2C_SCL);
    Serial.println("-----------------------------------------------------------");

    uint8_t devicesFound = 0;
    bool foundMpu = false;
    bool foundHmc = false;
    bool foundQmc = false;
    bool foundBmp = false;
    bool foundPca = false;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf(" -> Tìm thấy thiết bị tại địa chỉ: 0x%02X (%3d) | ", address, address);
            devicesFound++;

            switch (address) {
                case ADDR_PCA9685:
                    Serial.println("PCA9685 (16-ch PWM Driver cho 4 ESC) [OK]");
                    foundPca = true;
                    break;
                case ADDR_MPU6050_PRI:
                    Serial.println("MPU6050 IMU (Địa chỉ chính AD0=GND) [OK]");
                    foundMpu = true;
                    break;
                case ADDR_MPU6050_SEC:
                    Serial.println("MPU6050 IMU (Địa chỉ phụ AD0=VCC) [OK]");
                    foundMpu = true;
                    break;
                case ADDR_HMC5883L:
                    Serial.println("HMC5883L / QMC5883L (La bàn từ trường) [OK]");
                    foundHmc = true;
                    break;
                case ADDR_QMC5883L:
                    Serial.println("QMC5883L (Chip la bàn từ trường Clone chuẩn DA5883) [OK]");
                    foundQmc = true;
                    break;
                case ADDR_BMP280_PRI:
                    Serial.println("BMP280 Barometer (Địa chỉ SDO=GND: 0x76) [OK]");
                    foundBmp = true;
                    break;
                case ADDR_BMP280_SEC:
                    Serial.println("BMP280 Barometer (Địa chỉ SDO=VCC: 0x77) [OK]");
                    foundBmp = true;
                    break;
                default:
                    Serial.println("Thiết bị không xác định");
                    break;
            }
        } else if (error == 4) {
            Serial.printf(" [LỖI BUS] Lỗi đường truyền tại địa chỉ: 0x%02X\n", address);
        }
    }

    Serial.println("-----------------------------------------------------------");
    Serial.printf("[KẾT QUẢ] Tìm thấy tổng cộng %d thiết bị I2C.\n", devicesFound);

    // Kiểm tra chi tiết và phân biệt chip La bàn (HMC5883L vs QMC5883L)
    Serial.println("\n[CHI TIẾT CHIP NHẬN DIỆN]:");

    // 1. Kiểm tra MPU6050
    if (foundMpu) {
        Wire.beginTransmission(ADDR_MPU6050_PRI);
        Wire.write(0x75); // WHO_AM_I register
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)ADDR_MPU6050_PRI, (uint8_t)1);
        if (Wire.available()) {
            uint8_t whoami = Wire.read();
            Serial.printf("  ✔ MPU6050 WHO_AM_I = 0x%02X (Chuẩn là 0x68)\n", whoami);
        }
    } else {
        Serial.println("  ❌ MPU6050: KHÔNG TÌM THẤY! Kiểm tra lại dây SDA/SCL & nguồn 3.3V.");
    }

    // 2. Phân biệt HMC5883L thật vs QMC5883L clone
    if (foundHmc) {
        // Đọc 3 thanh ghi Identification của HMC5883L (Reg 10, 11, 12: 'H', '4', '3')
        Wire.beginTransmission(ADDR_HMC5883L);
        Wire.write(10);
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)ADDR_HMC5883L, (uint8_t)3);
        char id[4] = {0};
        if (Wire.available() >= 3) {
            id[0] = Wire.read();
            id[1] = Wire.read();
            id[2] = Wire.read();
        }
        if (strcmp(id, "H43") == 0) {
            Serial.println("  ✔ La bàn: CHÍNH HÃNG Honeywell HMC5883L (ID: H43, Addr: 0x1E)");
        } else {
            Serial.printf("  ⚠ La bàn: Chip tại 0x1E nhưng ID là '%s' (Biến thể HMC)\n", id);
        }
    } else if (foundQmc) {
        Wire.beginTransmission(ADDR_QMC5883L);
        Wire.write(0x0D); // Chip ID register của QMC5883L
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)ADDR_QMC5883L, (uint8_t)1);
        uint8_t qmcId = Wire.available() ? Wire.read() : 0;
        Serial.printf("  ✔ La bàn: CHIP CLONE QMC5883L / DA5883 (Addr: 0x0D, Chip ID: 0x%02X)\n", qmcId);
    } else {
        Serial.println("  ❌ La bàn (HMC/QMC5883L): KHÔNG TÌM THẤY!");
    }

    // 3. Kiểm tra BMP280
    uint8_t bmpAddr = 0;
    if (foundBmp) {
        Wire.beginTransmission(ADDR_BMP280_PRI);
        if (Wire.endTransmission() == 0) {
            bmpAddr = ADDR_BMP280_PRI;
        } else {
            bmpAddr = ADDR_BMP280_SEC;
        }
    }
    if (bmpAddr != 0) {
        Wire.beginTransmission(bmpAddr);
        Wire.write(0xD0); // Chip ID register của BMP280 (0x58)
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)bmpAddr, (uint8_t)1);
        uint8_t bmpId = Wire.available() ? Wire.read() : 0;
        Serial.printf("  ✔ BMP280 Barometer: Addr 0x%02X, Chip ID = 0x%02X (Chuẩn: 0x58)\n", bmpAddr, bmpId);
    } else {
        Serial.println("  ❌ BMP280: KHÔNG TÌM THẤY!");
    }

    // 4. Kiểm tra PCA9685
    if (foundPca) {
        Wire.beginTransmission(ADDR_PCA9685);
        Wire.write(0x00); // MODE1
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)ADDR_PCA9685, (uint8_t)1);
        uint8_t mode1 = Wire.available() ? Wire.read() : 0;
        Serial.printf("  ✔ PCA9685 PWM Driver: Addr 0x40, MODE1 = 0x%02X\n", mode1);
    } else {
        Serial.println("  ❌ PCA9685: KHÔNG TÌM THẤY!");
    }
    Serial.println("===========================================================\n");
}

void setup() {
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && (millis() - start < 2000)) {
        delay(10);
    }

    Serial.println("\n\n=======================================================");
    Serial.println("    ESP32-S3 I2C SCANNER & CHIP IDENTIFIER TOOL        ");
    Serial.println("=======================================================");

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ);
    Wire.setTimeOut(20);

    // Chạy quét I2C lần đầu khi khởi động
    scanI2CBus();
}

void loop() {
    // Tự động quét lại mỗi 4 giây hoặc khi nhận phím bất kỳ từ Serial
    if (Serial.available()) {
        while (Serial.available()) Serial.read();
        scanI2CBus();
    }

    static uint32_t lastScan = 0;
    if (millis() - lastScan > 4000) {
        lastScan = millis();
        scanI2CBus();
    }
}
