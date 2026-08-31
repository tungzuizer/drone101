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
        data_.armSwitch = true;
        data_.isConnected = true;
        data_.lastPacketTimeMs = millis();
        Serial.println("[GCS CMD] Nhận lệnh: ARM");
    } else if (strncasecmp(line, "DISARM", 6) == 0) {
        data_.armSwitch = false;
        data_.throttle = 0.0f;
        data_.lastPacketTimeMs = millis();
        Serial.println("[GCS CMD] Nhận lệnh: DISARM");
    } else if (strncasecmp(line, "SET RC ", 7) == 0) {
        parseRcCommand(line + 7);
    } else if (strncasecmp(line, "SET MODE ", 9) == 0) {
        parseModeCommand(line + 9);
    } else if (strncasecmp(line, "SET PID ", 8) == 0) {
        parsePidCommand(line + 8);
    } else if (strncasecmp(line, "TEST ", 5) == 0) {
        parseMotorTestCommand(line + 5);
    } else if (strncasecmp(line, "CALIB ", 6) == 0) {
        parseCalibCommand(line + 6);
    } else if (strncasecmp(line, "PING", 4) == 0 || strncasecmp(line, "HEARTBEAT", 9) == 0) {
        data_.isConnected = true;
        data_.lastPacketTimeMs = millis();
        Serial.println("PONG");
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

GcsCommand SerialControlInput::getPendingCommand() {
    GcsCommand cmd = pendingCmd_;
    pendingCmd_.isPending = false;
    return cmd;
}
