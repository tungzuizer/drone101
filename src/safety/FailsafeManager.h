#ifndef FAILSAFE_MANAGER_H
#define FAILSAFE_MANAGER_H

#include <Arduino.h>
#include "../config.h"
#include "../control/ControlInputSource.h"
#include "../control/AttitudeEstimator.h"
#include "../actuators/MotorController.h"

// Các mức độ kích hoạt Failsafe
enum FailsafeState {
    FS_OK = 0,          // Bình thường, an toàn
    FS_WARNING = 1,     // Cảnh báo mất sóng tạm thời
    FS_LANDING = 2,     // Tự động hạ cánh khẩn cấp (Giảm ga từ từ)
    FS_EMERGENCY = 3    // Cắt toàn bộ động cơ ngay lập tức (Kill Switch)
};

// Nguyên nhân kích hoạt Failsafe
enum FailsafeReason {
    FS_REASON_NONE = 0,
    FS_REASON_SIGNAL_LOST = 1,      // Mất kết nối Serial / RC quá thời gian quy định
    FS_REASON_EXCESSIVE_TILT = 2,   // Nghiêng lật vượt ngưỡng an toàn (> 55°)
    FS_REASON_SENSOR_FAILURE = 3,   // Hỏng cảm biến IMU / Mất kết nối I2C
    FS_REASON_ARM_TIMEOUT = 4,      // Arm nhưng không ga quá 15 giây (Tự Disarm)
    FS_REASON_MANUAL_KILL = 5       // Người dùng bấm nút dừng khẩn cấp
};

class FailsafeManager {
public:
    FailsafeManager(MotorController& motorController);

    // Khởi tạo các ngưỡng an toàn
    void begin(float maxTiltDeg = 55.0f, uint32_t signalTimeoutMs = 500);

    // Kiểm tra an toàn trước khi cho phép ARM
    // Trả về true nếu an toàn, false nếu bị chặn kèm lý do giải thích
    bool canArm(const ControlData& control, const AttitudeData& attitude, bool imuHealthy, String& reason);

    // Giám sát an toàn liên tục trong vòng lặp chính (250Hz)
    FailsafeState check(const ControlInputSource& input,
                        const AttitudeData& attitude,
                        bool imuHealthy);

    // Kích hoạt dừng khẩn cấp thủ công
    void triggerEmergency(FailsafeReason reason = FS_REASON_MANUAL_KILL);

    // Đọc trạng thái hiện tại
    FailsafeState getState() const { return state_; }
    FailsafeReason getReason() const { return reason_; }
    const char* getReasonString() const;

    // Reset Failsafe sau khi đã xử lý xong
    void reset();

private:
    MotorController& motors_;
    FailsafeState state_;
    FailsafeReason reason_;

    float maxTiltDeg_;
    uint32_t signalTimeoutMs_;
    uint32_t armTimestampMs_;
    uint32_t landingStartMs_;
};

#endif // FAILSAFE_MANAGER_H
