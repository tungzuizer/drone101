#include "SerialControlInput.h"
#include <string.h>
#include <ctype.h>

SerialControlInput::SerialControlInput(Stream& serialPort)
    : stream_(serialPort),
      bufferIndex_(0) {
    memset(&pendingCmd_, 0, sizeof(GcsCommand));
    resetToSafeState();
}

bool SerialControlInput::begin() {
    resetToSafeState();
    bufferIndex_ = 0;
    lineBuffer_[0] = '\0';
    return true;
}

void SerialControlInput::resetToSafeState() {
    data_.throttle = 0.0f;
    data_.roll = 0.0f;
    data_.pitch = 0.0f;
    data_.yaw = 0.0f;
    data_.flightMode = MODE_ANGLE;
    data_.armSwitch = false;
    data_.aux1 = false;
    data_.aux2 = false;
    data_.isConnected = false;
    data_.lastPacketTimeMs = 0;
}

void SerialControlInput::update() {
    while (stream_.available() > 0) {
        char c = (char)stream_.read();

        // Xử lý ký tự kết thúc dòng '\n' hoặc '\r'
        if (c == '\n' || c == '\r') {
            if (bufferIndex_ > 0) {
                lineBuffer_[bufferIndex_] = '\0';
                processLine(lineBuffer_);
                bufferIndex_ = 0;
            }
        } else {
            if (bufferIndex_ < SERIAL_CMD_BUFFER_SIZE - 1) {
                lineBuffer_[bufferIndex_++] = c;
            }
        }
    }
}

void SerialControlInput::processLine(const char* line) {
    // Bỏ qua khoảng trắng đầu dòng
    while (*line && isspace(*line)) line++;
    if (*line == '\0' || *line == '#') return; // Dòng trống hoặc chú thích

    if (strncasecmp(line, "ARM", 3) == 0 && (line[3] == '\0' || isspace(line[3]))) {
        strncpy(pendingCmd_.command, "ARM_REQUEST", sizeof(pendingCmd_.command) - 1);
        pendingCmd_.isPending = true;
        data_.isConnected = true;
        data_.lastPacketTimeMs = millis();
        Serial.println("[GCS CMD] Nhận lệnh: ARM");
    } else if (strncasecmp(line, "DISARM", 6) == 0) {
        strncpy(pendingCmd_.command, "DISARM_REQUEST", sizeof(pendingCmd_.command) - 1);
        pendingCmd_.isPending = true;
        data_.throttle = 0.0f;
        data_.lastPacketTimeMs = millis();
        Serial.println("[GCS CMD] Nhận lệnh: DISARM");
    } else if (strncasecmp(line, "SET RC ", 7) == 0) {
        parseRcCommand(line + 7);
    } else if (strncasecmp(line, "SET MODE ", 9) == 0) {
        parseModeCommand(line + 9);
    } else if (strncasecmp(line, "SET PID ", 8) == 0) {
        parsePidCommand(line + 8);
    } else if (strncasecmp(line, "SET FILTER ", 11) == 0) {
        parseFilterCommand(line + 11);
    } else if (strncasecmp(line, "SET RATES ", 10) == 0) {
        parseRatesCommand(line + 10);
    } else if (strncasecmp(line, "SET AIRMODE ", 12) == 0) {
        parseAirmodeCommand(line + 12);
    } else if (strncasecmp(line, "SET TPA ", 8) == 0) {
        parseTpaCommand(line + 8);
    } else if (strncasecmp(line, "SET ESTIMATOR ", 14) == 0) {
        parseEstimatorCommand(line + 14);
    } else if (strncasecmp(line, "SET FAILSAFE ", 13) == 0) {
        parseFailsafeCommand(line + 13);
    } else if (strncasecmp(line, "GET VERSION", 11) == 0 || strncasecmp(line, "VERSION", 7) == 0) {
        strncpy(pendingCmd_.command, "GET_VERSION", sizeof(pendingCmd_.command) - 1);
        pendingCmd_.isPending = true;
    } else if (strncasecmp(line, "TEST ", 5) == 0) {
        parseMotorTestCommand(line + 5);
    } else if (strncasecmp(line, "CALIB ", 6) == 0) {
        parseCalibCommand(line + 6);
    } else if (strncasecmp(line, "SCAN", 4) == 0 || strncasecmp(line, "I2C_SCAN", 8) == 0) {
        strncpy(pendingCmd_.command, "SCAN_I2C", sizeof(pendingCmd_.command) - 1);
        pendingCmd_.isPending = true;
    } else if (strncasecmp(line, "REINIT", 6) == 0) {
        strncpy(pendingCmd_.command, "REINIT_SENSORS", sizeof(pendingCmd_.command) - 1);
        pendingCmd_.isPending = true;
    } else if (strncasecmp(line, "PING", 4) == 0 || strncasecmp(line, "HEARTBEAT", 9) == 0) {
        data_.isConnected = true;
        data_.lastPacketTimeMs = millis();
        // Không in PONG để tránh spam Serial khi Tuner gửi PING 200ms liên tục
    }
}

void SerialControlInput::parseRcCommand(const char* args) {
    float t = 0.0f, r = 0.0f, p = 0.0f, y = 0.0f;
    int parsed = sscanf(args, "%f %f %f %f", &t, &r, &p, &y);
    if (parsed >= 4) {
        // Giới hạn ga (0 - 100%)
        if (t < 0.0f) t = 0.0f;
        if (t > 100.0f) t = 100.0f;

        // Giới hạn góc nghiêng Roll/Pitch (-45° đến +45°)
        if (r < -45.0f) r = -45.0f;
        if (r > 45.0f) r = 45.0f;
        if (p < -45.0f) p = -45.0f;
        if (p > 45.0f) p = 45.0f;

        // Giới hạn vận tốc xoay Yaw (-180°/s đến +180°/s)
        if (y < -180.0f) y = -180.0f;
        if (y > 180.0f) y = 180.0f;

        data_.throttle = t;
        data_.roll = r;
        data_.pitch = p;
        data_.yaw = y;
        data_.isConnected = true;
        data_.lastPacketTimeMs = millis();
    }
}

void SerialControlInput::parseModeCommand(const char* args) {
    while (*args && isspace(*args)) args++;
    if (strncasecmp(args, "ANGLE", 5) == 0 || strcmp(args, "0") == 0) {
        data_.flightMode = MODE_ANGLE;
        Serial.println("[GCS CMD] Chế độ bay: ANGLE (Tự cân bằng)");
    } else if (strncasecmp(args, "ACRO", 4) == 0 || strcmp(args, "1") == 0) {
        data_.flightMode = MODE_ACRO;
        Serial.println("[GCS CMD] Chế độ bay: ACRO (Nhào lộn Rate)");
    } else if (strncasecmp(args, "ALT_HOLD", 8) == 0 || strcmp(args, "2") == 0) {
        data_.flightMode = MODE_ALT_HOLD;
        Serial.println("[GCS CMD] Chế độ bay: ALT_HOLD (Giữ độ cao)");
    } else if (strncasecmp(args, "POS_HOLD", 8) == 0 || strcmp(args, "3") == 0) {
        data_.flightMode = MODE_POS_HOLD;
        Serial.println("[GCS CMD] Chế độ bay: POS_HOLD (Giữ vị trí GPS)");
    }
    data_.lastPacketTimeMs = millis();
}

void SerialControlInput::parsePidCommand(const char* args) {
    // Định dạng: SET PID <AXIS> <KP> <KI> <KD>
    char axis[16] = {0};
    float kp = 0, ki = 0, kd = 0;
    if (sscanf(args, "%15s %f %f %f", axis, &kp, &ki, &kd) >= 4) {
        strncpy(pendingCmd_.command, "SET_PID", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, axis, sizeof(pendingCmd_.arg1) - 1);
        snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", kp);
        snprintf(pendingCmd_.arg3, sizeof(pendingCmd_.arg3), "%f", ki);
        snprintf(pendingCmd_.arg4, sizeof(pendingCmd_.arg4), "%f", kd);
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình PID %s: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", axis, kp, ki, kd);
    }
}

void SerialControlInput::parseMotorTestCommand(const char* args) {
    // Định dạng: TEST M<1-4> <PERCENT>
    char motor[16] = {0};
    float percent = 0;
    if (sscanf(args, "%15s %f", motor, &percent) >= 2) {
        strncpy(pendingCmd_.command, "TEST_MOTOR", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, motor, sizeof(pendingCmd_.arg1) - 1);
        snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", percent);
        pendingCmd_.isPending = true;
    }
}

void SerialControlInput::parseCalibCommand(const char* args) {
    // Định dạng: CALIB <GYRO|MAG|BARO>
    char target[16] = {0};
    if (sscanf(args, "%15s", target) >= 1) {
        strncpy(pendingCmd_.command, "CALIB", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, target, sizeof(pendingCmd_.arg1) - 1);
        pendingCmd_.isPending = true;
    }
}

void SerialControlInput::parseFilterCommand(const char* args) {
    // Định dạng: SET FILTER <DTERM|GYRO_LPF|NOTCH> <VAL>
    char filterType[16] = {0};
    float val = 0.0f;
    if (sscanf(args, "%15s %f", filterType, &val) >= 2) {
        strncpy(pendingCmd_.command, "SET_FILTER", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, filterType, sizeof(pendingCmd_.arg1) - 1);
        snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", val);
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình Filter %s = %.2f\n", filterType, val);
    }
}

void SerialControlInput::parseRatesCommand(const char* args) {
    // Định dạng: SET RATES <RC_RATE> <SUPER_RATE> <EXPO>
    float rc = 1.0f, sr = 0.7f, ex = 0.0f;
    if (sscanf(args, "%f %f %f", &rc, &sr, &ex) >= 3) {
        strncpy(pendingCmd_.command, "SET_RATES", sizeof(pendingCmd_.command) - 1);
        snprintf(pendingCmd_.arg1, sizeof(pendingCmd_.arg1), "%f", rc);
        snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", sr);
        snprintf(pendingCmd_.arg3, sizeof(pendingCmd_.arg3), "%f", ex);
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình Rates: RC=%.2f, Super=%.2f, Expo=%.2f\n", rc, sr, ex);
    }
}

void SerialControlInput::parseFailsafeCommand(const char* args) {
    // Định dạng: SET FAILSAFE <TIMEOUT|MAX_TILT|ACTION> <VAL>
    char param[16] = {0};
    char val[32] = {0};
    if (sscanf(args, "%15s %31s", param, val) >= 2) {
        strncpy(pendingCmd_.command, "SET_FAILSAFE", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, param, sizeof(pendingCmd_.arg1) - 1);
        strncpy(pendingCmd_.arg2, val, sizeof(pendingCmd_.arg2) - 1);
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình Failsafe %s = %s\n", param, val);
    }
}

void SerialControlInput::parseAirmodeCommand(const char* args) {
    // Định dạng: SET AIRMODE <ON|OFF|1|0>
    while (*args && isspace(*args)) args++;
    strncpy(pendingCmd_.command, "SET_AIRMODE", sizeof(pendingCmd_.command) - 1);
    if (strncasecmp(args, "ON", 2) == 0 || strcmp(args, "1") == 0) {
        strncpy(pendingCmd_.arg1, "1", sizeof(pendingCmd_.arg1) - 1);
        Serial.println("[GCS CMD] Kích hoạt AirMode: ON");
    } else {
        strncpy(pendingCmd_.arg1, "0", sizeof(pendingCmd_.arg1) - 1);
        Serial.println("[GCS CMD] Kích hoạt AirMode: OFF");
    }
    pendingCmd_.isPending = true;
}

void SerialControlInput::parseTpaCommand(const char* args) {
    // Định dạng: SET TPA <RATE> <BREAKPOINT>
    float rate = 0.0f, bp = 0.50f;
    if (sscanf(args, "%f %f", &rate, &bp) >= 1) {
        strncpy(pendingCmd_.command, "SET_TPA", sizeof(pendingCmd_.command) - 1);
        snprintf(pendingCmd_.arg1, sizeof(pendingCmd_.arg1), "%f", rate);
        snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", bp);
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình TPA: Rate=%.2f, Breakpoint=%.2f\n", rate, bp);
    }
}

void SerialControlInput::parseEstimatorCommand(const char* args) {
    // Định dạng: SET ESTIMATOR <MAHONY|MADGWICK> [PARAM]
    char algo[16] = {0};
    float param = 0.0f;
    int parsed = sscanf(args, "%15s %f", algo, &param);
    if (parsed >= 1) {
        strncpy(pendingCmd_.command, "SET_ESTIMATOR", sizeof(pendingCmd_.command) - 1);
        strncpy(pendingCmd_.arg1, algo, sizeof(pendingCmd_.arg1) - 1);
        if (parsed >= 2) {
            snprintf(pendingCmd_.arg2, sizeof(pendingCmd_.arg2), "%f", param);
        } else {
            pendingCmd_.arg2[0] = '\0';
        }
        pendingCmd_.isPending = true;
        Serial.printf("[GCS CMD] Cấu hình Attitude Estimator: %s (param=%.3f)\n", algo, param);
    }
}

GcsCommand SerialControlInput::getPendingCommand() {
    GcsCommand cmd = pendingCmd_;
    pendingCmd_.isPending = false;
    return cmd;
}

void SerialControlInput::processExternalLine(const char* line) {
    processLine(line);
}
