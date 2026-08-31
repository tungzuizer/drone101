#ifndef SERIAL_CONTROL_INPUT_H
#define SERIAL_CONTROL_INPUT_H

#include "ControlInputSource.h"
#include <Arduino.h>

#define SERIAL_CMD_BUFFER_SIZE 128

// Cấu trúc lệnh phụ trợ từ GCS (Ví dụ cấu hình PID, Calib, Test Motor)
struct GcsCommand {
    char command[32];
    char arg1[32];
    char arg2[32];
    char arg3[32];
    char arg4[32];
    bool isPending;
};

class SerialControlInput : public ControlInputSource {
public:
    SerialControlInput(Stream& serialPort = Serial);

    bool begin() override;
    void update() override;
    const ControlData& getControlData() const override { return data_; }

    // Kiểm tra có lệnh cấu hình đặc biệt từ GCS không (PID tune, Calib, Motor test)
    bool hasPendingCommand() const { return pendingCmd_.isPending; }
    GcsCommand getPendingCommand();

    // Reset lại dữ liệu điều khiển về trạng thái an toàn
    void resetToSafeState();

private:
    Stream& stream_;
    ControlData data_;
    GcsCommand pendingCmd_;

    char lineBuffer_[SERIAL_CMD_BUFFER_SIZE];
    uint8_t bufferIndex_;

    void processLine(const char* line);
    void parseRcCommand(const char* args);
    void parseModeCommand(const char* args);
    void parsePidCommand(const char* args);
    void parseMotorTestCommand(const char* args);
    void parseCalibCommand(const char* args);
};

#endif // SERIAL_CONTROL_INPUT_H
