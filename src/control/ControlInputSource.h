#ifndef CONTROL_INPUT_SOURCE_H
#define CONTROL_INPUT_SOURCE_H

#include <Arduino.h>

// Các chế độ bay của Drone
enum FlightMode {
    MODE_ANGLE = 0,     // Chế độ tự cân bằng góc (Self-Leveling Angle Mode) - Khuyến nghị cho người mới
    MODE_ACRO = 1,      // Chế độ nhào lộn theo vận tốc góc (Rate / Acrobatic Mode)
    MODE_ALT_HOLD = 2,  // Chế độ giữ độ cao tự động bằng cảm biến áp suất BMP280
    MODE_POS_HOLD = 3   // Chế độ giữ vị trí bằng GPS ATGM336H
};

// Dữ liệu điều khiển từ nguồn nhận (Serial, RC Receiver, ESP-NOW, v.v.)
struct ControlData {
    float throttle;     // Cần ga: 0.0% - 100.0%
    float roll;         // Nghiêng ngang: -45.0° đến +45.0° (hoặc deg/s trong Acro)
    float pitch;        // Ngóc/chúc dọc: -45.0° đến +45.0° (hoặc deg/s trong Acro)
    float yaw;          // Xoay hướng mũi: -180.0°/s đến +180.0°/s

    FlightMode flightMode;  // Chế độ bay hiện tại
    bool armSwitch;         // Công tắc Arm (Kích hoạt động cơ)
    bool aux1;              // Công tắc phụ 1
    bool aux2;              // Công tắc phụ 2

    bool isConnected;       // Đang có tín hiệu kết nối hay không
    uint32_t lastPacketTimeMs; // Thời điểm nhận gói tin cuối cùng (Dùng cho Failsafe timeout)
};

class ControlInputSource {
public:
    virtual ~ControlInputSource() = default;

    // Khởi tạo nguồn tín hiệu điều khiển
    virtual bool begin() = 0;

    // Cập nhật và giải mã dữ liệu điều khiển từ phần cứng/giao thức
    virtual void update() = 0;

    // Lấy dữ liệu điều khiển hiện tại
    virtual const ControlData& getControlData() const = 0;

    // Kiểm tra mất tín hiệu (Failsafe timeout tính bằng milli giây)
    virtual bool isSignalLost(uint32_t timeoutMs = 500) const {
        const ControlData& data = getControlData();
        if (!data.isConnected) return true;
        return (millis() - data.lastPacketTimeMs > timeoutMs);
    }
};

#endif // CONTROL_INPUT_SOURCE_H
