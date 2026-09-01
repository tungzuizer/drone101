#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// Các module cảm biến
#include "sensors/ImuSensor.h"
#include "sensors/Magnetometer.h"
#include "sensors/Barometer.h"
#include "sensors/GpsReader.h"

// Các module chấp hành, ước lượng và điều khiển
#include "actuators/MotorController.h"
#include "control/AttitudeEstimator.h"
#include "control/SerialControlInput.h"
#include "control/WifiControlInput.h"
#include "control/MotorMixer.h"
#include "safety/FailsafeManager.h"

// =============================================================================
// KHỞI TẠO CÁC ĐỐI TƯỢNG HỆ THỐNG
// =============================================================================
ImuSensor           imu;
Magnetometer        mag;
Barometer           baro;
GpsReader           gps(Serial1);
MotorController     motors;
AttitudeEstimator   attitude;
SerialControlInput  rcInput(Serial);
WifiControlInput    wifiInput(WIFI_HTTP_PORT, WIFI_WS_PORT);
MotorMixer          mixer(motors);
FailsafeManager     failsafe(motors);

// =============================================================================
// BIẾN QUẢN LÝ THỜI GIAN VÒNG LẶP CHÍNH (NON-BLOCKING TIMING)
// =============================================================================
uint32_t lastLoopTimeUs     = 0;
uint32_t lastMedLoopTimeUs  = 0;
uint32_t lastSlowLoopTimeUs = 0;
uint32_t lastHeartbeatMs    = 0;

// Chu kỳ vòng lặp
const uint32_t FAST_LOOP_US = 1000000UL / MAIN_LOOP_FREQ_HZ;  // 250Hz -> 4000µs
const uint32_t MED_LOOP_US  = 20000UL;                         // 50Hz  -> 20000µs (Mag, Baro)
const uint32_t SLOW_LOOP_US = 100000UL;                        // 10Hz  -> 100000µs (Telemetry GCS)

// =============================================================================
// FREERTOS TASK CORE 0: XỬ LÝ MẠNG WI-FI & WEBSOCKET CHO ĐIỆN THOẠI
// Chạy độc lập trên Core 0, không ảnh hưởng vòng lặp bay 250Hz trên Core 1
// =============================================================================
void commsTask(void* pvParameters) {
    Serial.println("[FREERTOS] Comms Task đã khởi động trên Core 0.");
    while (true) {
        wifiInput.processNetwork();
        vTaskDelay(pdMS_TO_TICKS(5)); // Delay 5ms giải phóng CPU Core 0
    }
}

// =============================================================================
// QUÉT & CHẨN ĐOÁN BUS I2C TỰ ĐỘNG
// =============================================================================
void scanAndReportI2c(bool autoReinit = true) {
    Serial.println("\n-------------------------------------------------------");
    Serial.printf("[I2C SCAN] Đang quét toàn bộ Bus I2C (SDA=GPIO%d, SCL=GPIO%d)...\n", PIN_I2C_SDA, PIN_I2C_SCL);
    uint8_t count = 0;
    bool foundMpu = false, foundBmp = false, foundMag = false, foundPca = false;
    uint8_t mpuAddr = 0, bmpAddr = 0, magAddr = 0, pcaAddr = 0;

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            count++;
            Serial.printf(" -> Tìm thấy thiết bị tại: 0x%02X | ", addr);
            if (addr == 0x68 || addr == 0x69) {
                foundMpu = true;
                mpuAddr = addr;
                Serial.printf("MPU6050 IMU (Addr: 0x%02X) [OK]\n", addr);
            } else if (addr == 0x76 || addr == 0x77) {
                foundBmp = true;
                bmpAddr = addr;
                Serial.printf("BMP280 Barometer (Addr: 0x%02X) [OK]\n", addr);
            } else if (addr == 0x1E || addr == 0x0D) {
                foundMag = true;
                magAddr = addr;
                Serial.printf("La bàn từ trường (Addr: 0x%02X) [OK]\n", addr);
            } else if (addr == 0x40) {
                foundPca = true;
                pcaAddr = addr;
                Serial.printf("PCA9685 PWM Driver (Addr: 0x%02X) [OK]\n", addr);
            } else {
                Serial.println("Thiết bị I2C khác");
            }
        }
    }
    Serial.printf("[I2C SCAN] Tổng cộng phát hiện: %d thiết bị I2C.\n", count);

    // Gửi gói tin máy đọc cho Web Tuner: $DEV,mpuOk,bmpOk,magOk,pcaOk,mpuAddr,bmpAddr,magAddr,pcaAddr
    Serial.printf("$DEV,%d,%d,%d,%d,0x%02X,0x%02X,0x%02X,0x%02X\n",
                  foundMpu ? 1 : 0, foundBmp ? 1 : 0, foundMag ? 1 : 0, foundPca ? 1 : 0,
                  mpuAddr, bmpAddr, magAddr, pcaAddr);

    if (autoReinit && !motors.isArmed()) {
        if (foundMpu && !imu.isHealthy()) {
            Serial.println("[AUTO-REINIT] Đang khởi tạo lại MPU6050...");
            if (imu.begin(mpuAddr)) {
                imu.calibrateGyro(300);
            }
        }
        if (foundBmp && !baro.isHealthy()) {
            Serial.println("[AUTO-REINIT] Đang khởi tạo lại BMP280...");
            baro.begin(bmpAddr);
        }
        if (foundMag && !mag.isHealthy()) {
            Serial.println("[AUTO-REINIT] Đang khởi tạo lại Magnetometer...");
            mag.begin();
        }
        if (foundPca && !motors.isHealthy()) {
            Serial.println("[AUTO-REINIT] Đang khởi tạo lại PCA9685...");
            motors.begin(pcaAddr);
        }
    }
    Serial.println("-------------------------------------------------------\n");
}

// =============================================================================
// XỬ LÝ LỆNH TỪ GCS TUNER (PID TUNING, CALIBRATION, MOTOR TEST)
// =============================================================================
void handleGcsCommands() {
    if (!rcInput.hasPendingCommand()) return;

    GcsCommand cmd = rcInput.getPendingCommand();

    if (strcmp(cmd.command, "SET_PID") == 0) {
        float kp = atof(cmd.arg2);
        float ki = atof(cmd.arg3);
        float kd = atof(cmd.arg4);

        if (strcasecmp(cmd.arg1, "ROLL_RATE") == 0 || strcasecmp(cmd.arg1, "ROLL") == 0) {
            mixer.getRollRatePid().setGains(kp, ki, kd);
            Serial.printf("[PID OK] Roll Rate PID cập nhật: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
        } else if (strcasecmp(cmd.arg1, "PITCH_RATE") == 0 || strcasecmp(cmd.arg1, "PITCH") == 0) {
            mixer.getPitchRatePid().setGains(kp, ki, kd);
            Serial.printf("[PID OK] Pitch Rate PID cập nhật: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
        } else if (strcasecmp(cmd.arg1, "YAW_RATE") == 0 || strcasecmp(cmd.arg1, "YAW") == 0) {
            mixer.getYawRatePid().setGains(kp, ki, kd);
            Serial.printf("[PID OK] Yaw Rate PID cập nhật: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
        } else if (strcasecmp(cmd.arg1, "ROLL_ANGLE") == 0) {
            mixer.getRollAnglePid().setGains(kp, ki, kd);
            Serial.printf("[PID OK] Roll Angle PID cập nhật: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
        } else if (strcasecmp(cmd.arg1, "PITCH_ANGLE") == 0) {
            mixer.getPitchAnglePid().setGains(kp, ki, kd);
            Serial.printf("[PID OK] Pitch Angle PID cập nhật: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", kp, ki, kd);
        }
    } else if (strcmp(cmd.command, "TEST_MOTOR") == 0) {
        if (motors.isArmed()) {
            Serial.println("[TEST DENIED] Không thể test motor khi hệ thống đang ARM!");
            return;
        }
        // Lệnh test động cơ riêng lẻ trên bàn thử (Ví dụ: TEST M1 15)
        uint8_t motorNum = 1;
        if (cmd.arg1[0] == 'M' || cmd.arg1[0] == 'm') {
            motorNum = atoi(cmd.arg1 + 1);
        } else {
            motorNum = atoi(cmd.arg1);
        }
        float percent = atof(cmd.arg2);
        motors.testMotor(motorNum, percent);
    } else if (strcmp(cmd.command, "CALIB") == 0) {
        if (strcasecmp(cmd.arg1, "GYRO") == 0) {
            if (!motors.isArmed()) {
                Serial.println("[CALIB] Đang hiệu chuẩn Gyroscope (Giữ yên Drone)...");
                imu.calibrateGyro(500);
            } else {
                Serial.println("[CALIB DENIED] Không thể hiệu chuẩn khi đang ARM!");
            }
        } else if (strcasecmp(cmd.arg1, "MAG") == 0) {
            if (!motors.isArmed()) {
                mag.calibrate(15);
            } else {
                Serial.println("[CALIB DENIED] Không thể hiệu chuẩn khi đang ARM!");
            }
        } else if (strcasecmp(cmd.arg1, "BARO") == 0) {
            if (!motors.isArmed()) {
                Serial.println("[CALIB] Đang hiệu chuẩn độ cao gốc Barometer (BMP280)...");
                baro.setGroundReference(50);
            } else {
                Serial.println("[CALIB DENIED] Không thể hiệu chuẩn khi đang ARM!");
            }
        }
    } else if (strcmp(cmd.command, "SCAN_I2C") == 0) {
        scanAndReportI2c(true);
    } else if (strcmp(cmd.command, "REINIT_SENSORS") == 0) {
        if (!motors.isArmed()) {
            Serial.println("[GCS] Đang tái khởi tạo toàn bộ cảm biến...");
            imu.begin();
            mag.begin();
            baro.begin();
            motors.begin();
            scanAndReportI2c(false);
        } else {
            Serial.println("[CALIB DENIED] Không thể Reinit khi đang ARM!");
        }
    }
}

// =============================================================================
// TRUYỀN DỮ LIỆU TELEMETRY CHO PHẦN MỀM GCS TUNER (10Hz)
// =============================================================================
void sendTelemetryToGcs() {
    const AttitudeData& att = attitude.getAttitude();
    const MotorOutputs& out = mixer.getOutputs();
    const ControlData& ctrl = rcInput.getControlData();
    const BaroData& baroData = baro.getData();
    const ImuData& imuData = imu.getData();

    // Định dạng gói tin: $TEL,roll,pitch,yaw,rateRoll,ratePitch,rateYaw,throttle,m1,m2,m3,m4,alt,armed,fsState,imuOk,bmpOk,magOk,pcaOk,ax,ay,az
    Serial.printf("$TEL,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%d,%d,%d,%d,%.2f,%d,%d,%d,%d,%d,%d,%.2f,%.2f,%.2f\n",
                  att.roll, att.pitch, att.yaw,
                  att.rateRoll, att.ratePitch, att.rateYaw,
                  ctrl.throttle,
                  out.m1, out.m2, out.m3, out.m4,
                  baroData.relativeAltitude,
                  motors.isArmed() ? 1 : 0,
                  (int)failsafe.getState(),
                  imu.isHealthy() ? 1 : 0,
                  baro.isHealthy() ? 1 : 0,
                  mag.isHealthy() ? 1 : 0,
                  motors.isHealthy() ? 1 : 0,
                  imuData.ax, imuData.ay, imuData.az);
}

// =============================================================================
// SETUP - KHỞI TẠO TOÀN BỘ HỆ THỐNG
// =============================================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    // Chờ kết nối Serial USB CDC trên ESP32-S3
    unsigned long startWait = millis();
    while (!Serial && (millis() - startWait < 2000)) {
        delay(10);
    }

    Serial.println("\n\n=======================================================");
    Serial.println("  ESP32-S3 HIGH-RELIABILITY FLIGHT CONTROLLER FIRMWARE  ");
    Serial.println("  Tần số điều khiển: 250Hz | Cấu hình Quadcopter: Quad-X");
    Serial.println("=======================================================");

    // 1. Khởi tạo Bus I2C với SDA=GPIO8, SCL=GPIO9 ở 400kHz
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQUENCY);
    Wire.setTimeOut(5); // 5ms timeout tránh nghẽn bus trong vòng lặp điều khiển 250Hz

    // Quét nhanh toàn bộ thiết bị I2C để báo cáo và xác định địa chỉ chính xác
    scanAndReportI2c(false);

    // 2. Khởi tạo IMU MPU6050
    Serial.println("[INIT] Đang khởi tạo MPU6050...");
    if (imu.begin(I2C_ADDR_MPU6050_PRI)) {
        imu.calibrateGyro(500); // Lấy mẫu tĩnh hiệu chuẩn Gyro Bias
    } else {
        Serial.println("[WARN] MPU6050 chưa sẵn sàng! Firmware sẽ tự động thử kết nối lại khi phát hiện cảm biến.");
    }

    // 3. Khởi tạo Magnetometer (HMC5883L / QMC5883L tự thích ứng)
    Serial.println("[INIT] Đang khởi tạo Magnetometer...");
    mag.begin();

    // 4. Khởi tạo Barometer BMP280
    Serial.println("[INIT] Đang khởi tạo BMP280...");
    baro.begin(I2C_ADDR_BMP280_PRI);

    // 5. Khởi tạo GPS ATGM336H qua UART1 (RX=GPIO17, TX=GPIO18)
    Serial.println("[INIT] Đang khởi tạo GPS ATGM336H...");
    gps.begin(PIN_GPS_RX, PIN_GPS_TX, GPS_BAUDRATE);

    // 6. Khởi tạo Driver PWM PCA9685 cho 4 ESC
    Serial.println("[INIT] Đang khởi tạo PCA9685 PWM Driver...");
    motors.begin(I2C_ADDR_PCA9685_PRI);

    // 7. Khởi tạo Nguồn nhận tín hiệu điều khiển (Serial / GCS & Wi-Fi Phone Cockpit)
    rcInput.begin();
    wifiInput.begin();

    // Khởi tạo FreeRTOS Task chạy tác vụ Mạng / Wi-Fi trên Core 0
    xTaskCreatePinnedToCore(
        commsTask,          // Hàm thực thi task
        "CommsTask",        // Tên task
        8192,               // Kích thước Stack (8KB)
        NULL,               // Tham số truyền vào
        1,                  // Mức ưu tiên thấp (Background Comms)
        NULL,               // Handle task
        0                   // Chạy ghim cố định trên Core 0 (để Core 1 chuyên trách bay 250Hz)
    );

    // 8. Khởi tạo Bộ ước lượng tư thế không gian (Mahony AHRS Filter)
    attitude.begin(2.5f, 0.005f);

    // 9. Khởi tạo Motor Mixer Quad-X và Hệ thống PID kép
    mixer.begin();

    // 10. Khởi tạo Giám sát an toàn Failsafe
    failsafe.begin(MAX_TILT_ANGLE_DEG, FAILSAFE_TIMEOUT_MS);

    Serial.println("=======================================================");
    Serial.println("[READY] HỆ THỐNG ĐÃ SẴN SÀNG! ĐANG CHẠY MAIN LOOP 250Hz");
    Serial.println("=======================================================\n");

    lastLoopTimeUs = micros();
    lastMedLoopTimeUs = micros();
    lastSlowLoopTimeUs = micros();
}

// =============================================================================
// MAIN LOOP - ĐIỀU KHIỂN BAY 250HZ NON-BLOCKING
// =============================================================================
void loop() {
    uint32_t currentUs = micros();

    // -------------------------------------------------------------------------
    // 1. FAST LOOP (250Hz - 4000µs): Đọc IMU, Ước lượng tư thế, PID & Motor Mixer
    // -------------------------------------------------------------------------
    if (currentUs - lastLoopTimeUs >= FAST_LOOP_US) {
        float dtSeconds = (float)(currentUs - lastLoopTimeUs) * 1e-6f;
        lastLoopTimeUs = currentUs;

        // Giới hạn dt an toàn chống đột biến vi phân trong PID
        if (dtSeconds < 0.001f) dtSeconds = 0.001f;
        else if (dtSeconds > 0.020f) dtSeconds = 0.020f;

        // 1.1. Cập nhật tín hiệu điều khiển từ các nguồn (Serial GCS & Wi-Fi Phone Cockpit)
        rcInput.update();
        wifiInput.update();

        // Trọng tài nguồn điều khiển (Input Arbitration):
        // Nếu hệ thống đang ARM, giữ cố định nguồn điều khiển đã ARM để Failsafe xử lý nếu mất kết nối
        static ControlInputSource* armingSource = nullptr;
        ControlInputSource* activeInput = &rcInput;

        if (motors.isArmed() && armingSource != nullptr) {
            activeInput = armingSource;
        } else if (wifiInput.isPhoneConnected()) {
            activeInput = &wifiInput;
        }
        const ControlData& ctrl = activeInput->getControlData();

        // 1.2. Xử lý yêu cầu ARM / DISARM từ người dùng (Edge & Rate-limited Logging)
        static uint32_t lastArmRejectLogMs = 0;
        if (ctrl.armSwitch && !motors.isArmed()) {
            String denyReason;
            if (failsafe.canArm(ctrl, attitude.getAttitude(), imu.isHealthy(), denyReason)) {
                motors.arm();
                armingSource = activeInput;
                Serial.printf("[ARM OK] Hệ thống đã ARM bởi nguồn: %s\n",
                              (activeInput == &wifiInput) ? "Wi-Fi Smartphone" : "Serial/RC");
            } else if (millis() - lastArmRejectLogMs >= 1000) {
                lastArmRejectLogMs = millis();
                Serial.printf("[ARM REJECTED] %s\n", denyReason.c_str());
            }
        } else if (!ctrl.armSwitch && motors.isArmed() && activeInput == armingSource) {
            motors.disarm();
            mixer.resetPids();
            armingSource = nullptr;
            Serial.println("[DISARM] Đã ngắt động cơ theo lệnh người lái.");
        }

        // 1.3. Đọc dữ liệu IMU (Gia tốc kế & Con quay hồi chuyển)
        bool imuOk = imu.update();

        // 1.4. Ước lượng tư thế không gian 3D (Mahony AHRS)
        if (imuOk) {
            if (mag.isHealthy()) {
                attitude.update9DOF(imu.getData(), mag.getData(), dtSeconds);
            } else {
                attitude.update6DOF(imu.getData(), dtSeconds);
            }
        }

        // 1.5. Giám sát an toàn Failsafe
        FailsafeState fsState = failsafe.check(*activeInput, attitude.getAttitude(), imu.isHealthy());

        // 1.6. Tính toán vòng lặp PID kép và Trộn tín hiệu động cơ Quad-X
        if (fsState == FS_OK || fsState == FS_WARNING) {
            mixer.update(ctrl, attitude.getAttitude(), dtSeconds);
        } else if (fsState == FS_LANDING) {
            // Khi mất sóng: Tự cân bằng nằm ngang (Roll=0, Pitch=0, Yaw=0) và hạ ga từ từ
            ControlData landingCtrl = ctrl;
            landingCtrl.flightMode = MODE_ANGLE;
            landingCtrl.roll = 0.0f;
            landingCtrl.pitch = 0.0f;
            landingCtrl.yaw = 0.0f;
            landingCtrl.throttle = 15.0f; // Giữ ga hạ cánh an toàn
            mixer.update(landingCtrl, attitude.getAttitude(), dtSeconds);
        } else if (fsState == FS_EMERGENCY) {
            // Dừng khẩn cấp: Đảm bảo mixer không phát xung và ngắt PID
            mixer.resetPids();
        }

        // 1.7. Đẩy Telemetry thời gian thực vào Mailbox cho Core 0 phát về điện thoại (Lock-Free)
        const AttitudeData& currentAtt = attitude.getAttitude();
        const MotorOutputs& currentOut = mixer.getOutputs();
        const BaroData& currentBaro = baro.getData();

        TelemetryPayload telem;
        telem.roll = currentAtt.roll;
        telem.pitch = currentAtt.pitch;
        telem.yaw = currentAtt.yaw;
        telem.altitude = currentBaro.relativeAltitude;
        telem.batteryVoltage = 12.0f; // Mặc định hiển thị hoặc đọc từ ADC pin
        telem.m1 = currentOut.m1;
        telem.m2 = currentOut.m2;
        telem.m3 = currentOut.m3;
        telem.m4 = currentOut.m4;
        telem.isArmed = motors.isArmed();
        telem.failsafeState = static_cast<uint8_t>(fsState);
        telem.flightLoopTimeUs = micros() - currentUs;

        wifiInput.updateTelemetry(telem);
    }

    // -------------------------------------------------------------------------
    // 2. MEDIUM LOOP (50Hz - 20ms): Đọc Magnetometer, Barometer & Xử lý lệnh GCS
    // -------------------------------------------------------------------------
    if (currentUs - lastMedLoopTimeUs >= MED_LOOP_US) {
        lastMedLoopTimeUs = currentUs;

        // Đọc Từ kế
        if (mag.isHealthy()) {
            mag.update();
        }

        // Đọc Áp kế BMP280
        if (baro.isHealthy()) {
            baro.update();
        }

        // Xử lý các lệnh cấu hình từ GCS
        handleGcsCommands();
    }

    // -------------------------------------------------------------------------
    // 3. SLOW LOOP (10Hz - 100ms): Đọc GPS & Gửi Telemetry cho GCS Tuner
    // -------------------------------------------------------------------------
    if (currentUs - lastSlowLoopTimeUs >= SLOW_LOOP_US) {
        lastSlowLoopTimeUs = currentUs;

        // Cập nhật GPS
        gps.update();

        // Gửi gói tin trạng thái Telemetry
        sendTelemetryToGcs();
    }

    // -------------------------------------------------------------------------
    // 4. HEARTBEAT (1Hz): Nhịp tim hệ thống & Tự động kết nối lại MPU nếu mất
    // -------------------------------------------------------------------------
    if (millis() - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = millis();

        // Tự động thử kết nối lại MPU6050 nếu lúc bật nguồn chưa cắm và hiện chưa ARM
        if (!imu.isHealthy() && !motors.isArmed()) {
            Wire.beginTransmission(I2C_ADDR_MPU6050_PRI);
            if (Wire.endTransmission() == 0 || (Wire.beginTransmission(I2C_ADDR_MPU6050_ALT), Wire.endTransmission() == 0)) {
                Serial.println("[AUTO-DETECT] Phát hiện MPU6050 trên Bus I2C! Đang khởi tạo...");
                if (imu.begin()) {
                    imu.calibrateGyro(300);
                }
            }
        }
    }
}
