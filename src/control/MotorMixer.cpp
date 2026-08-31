#include "MotorMixer.h"

MotorMixer::MotorMixer(MotorController& motorController)
    : motors_(motorController) {
    outputs_.m1 = ESC_MIN_PULSE_US;
    outputs_.m2 = ESC_MIN_PULSE_US;
    outputs_.m3 = ESC_MIN_PULSE_US;
    outputs_.m4 = ESC_MIN_PULSE_US;
}

void MotorMixer::begin() {
    // 1. Cấu hình PID Vòng ngoài (Euler Angle Loop)
    // Kp = 4.5: Với sai số 10° nghiêng sẽ yêu cầu vận tốc bẻ góc 45°/s
    rollAnglePid_.begin(4.5f, 0.0f, 0.0f, 0.0f, 300.0f);
    pitchAnglePid_.begin(4.5f, 0.0f, 0.0f, 0.0f, 300.0f);

    // 2. Cấu hình PID Vòng trong (Rate Loop - Gyro 250Hz)
    // Hệ số khởi điểm an toàn cho động cơ A2212 1000KV + ESC 30A + Cánh 1045
    // Kd được tính theo miền thời gian thực (dt = 0.004s): Kd = 0.035 tương đương vi phân nhạy mượt mà
    rollRatePid_.begin(1.20f, 0.04f, 0.035f, 150.0f, 400.0f);
    pitchRatePid_.begin(1.20f, 0.04f, 0.035f, 150.0f, 400.0f);
    yawRatePid_.begin(2.50f, 0.08f, 0.0f, 150.0f, 400.0f);

    // Cấu hình lọc thông thấp triệt tiêu rung động cơ cho vi phân D
    rollRatePid_.setDTermFilter(0.70f);
    pitchRatePid_.setDTermFilter(0.70f);

    resetPids();
}

void MotorMixer::resetPids() {
    rollAnglePid_.reset();
    pitchAnglePid_.reset();
    rollRatePid_.reset();
    pitchRatePid_.reset();
    yawRatePid_.reset();
}

void MotorMixer::update(const ControlData& control, const AttitudeData& attitude, float dtSeconds) {
    // 1. Kiểm tra an toàn: Nếu chưa ARM hoặc Cần ga = 0% -> Dừng động cơ và ngắt tích phân I-term
    if (!motors_.isArmed() || control.throttle <= 0.5f) {
        resetPids();
        if (motors_.isArmed()) {
            // Khi ARM nhưng ga = 0: Quay chậm ở mức Idle để người dùng nhận biết
            motors_.setAllMotorsPwm(ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US);
            outputs_.m1 = ESC_IDLE_PULSE_US;
            outputs_.m2 = ESC_IDLE_PULSE_US;
            outputs_.m3 = ESC_IDLE_PULSE_US;
            outputs_.m4 = ESC_IDLE_PULSE_US;
        } else {
            motors_.setAllMotorsPwm(ESC_MIN_PULSE_US, ESC_MIN_PULSE_US, ESC_MIN_PULSE_US, ESC_MIN_PULSE_US);
            outputs_.m1 = ESC_MIN_PULSE_US;
            outputs_.m2 = ESC_MIN_PULSE_US;
            outputs_.m3 = ESC_MIN_PULSE_US;
            outputs_.m4 = ESC_MIN_PULSE_US;
        }
        return;
    }

    float targetRateRoll = 0.0f;
    float targetRatePitch = 0.0f;
    float targetRateYaw = control.yaw; // Lệnh xoay hướng mũi luôn ở dạng Vận tốc góc (deg/s)

    // 2. VÒNG NGOÀI (Outer Angle Loop)
    if (control.flightMode == MODE_ANGLE || control.flightMode == MODE_ALT_HOLD || control.flightMode == MODE_POS_HOLD) {
        // Chế độ tự cân bằng: Đưa sai số góc nghiêng qua PID Angle để tính vận tốc góc mong muốn
        targetRateRoll  = rollAnglePid_.update(control.roll, attitude.roll, dtSeconds);
        targetRatePitch = pitchAnglePid_.update(control.pitch, attitude.pitch, dtSeconds);
    } else {
        // Chế độ Acro / Rate: Cần điều khiển trực tiếp đặt vận tốc góc
        targetRateRoll  = control.roll * 4.0f;  // ±45° cần gạt tương đương ±180°/s
        targetRatePitch = control.pitch * 4.0f;
    }

    // 3. VÒNG TRONG (Inner Rate Loop - 250Hz Gyro Feedback)
    float rollCmd  = rollRatePid_.updateRate(targetRateRoll, attitude.rateRoll, dtSeconds);
    float pitchCmd = pitchRatePid_.updateRate(targetRatePitch, attitude.ratePitch, dtSeconds);
    float yawCmd   = yawRatePid_.updateRate(targetRateYaw, attitude.rateYaw, dtSeconds);

    // 4. TÍNH TOÁN XUNG GA CƠ BẢN (Base Throttle Pulse)
    uint8_t maxThrottlePercent = motors_.getMaxTestThrottle();
    float maxAllowedPulse = ESC_MIN_PULSE_US + (float)maxThrottlePercent * 10.0f; // 30% -> 1300µs

    float normalizedThrottle = control.throttle / 100.0f;
    float baseThrottle = ESC_IDLE_PULSE_US + normalizedThrottle * (maxAllowedPulse - ESC_IDLE_PULSE_US);

    // 5. MA TRẬN TRỘN TÍN HIỆU QUAD-X (Quad-X Motor Mixer)
    // M1: Trước Phải (CCW) = Ga - Roll + Pitch + Yaw
    // M2: Trước Trái (CW)  = Ga + Roll + Pitch - Yaw
    // M3: Sau Phải (CW)   = Ga - Roll - Pitch - Yaw
    // M4: Sau Trái (CCW)  = Ga + Roll - Pitch + Yaw
    float m1 = baseThrottle - rollCmd + pitchCmd + yawCmd;
    float m2 = baseThrottle + rollCmd + pitchCmd - yawCmd;
    float m3 = baseThrottle - rollCmd - pitchCmd - yawCmd;
    float m4 = baseThrottle + rollCmd - pitchCmd + yawCmd;

    // 6. XỬ LÝ CHỐNG BÃO HÒA ĐỘNG CƠ (Anti-Saturation)
    applySaturationLimits(m1, m2, m3, m4, maxAllowedPulse);

    outputs_.m1 = (uint16_t)m1;
    outputs_.m2 = (uint16_t)m2;
    outputs_.m3 = (uint16_t)m3;
    outputs_.m4 = (uint16_t)m4;

    // 7. Ghi xung ra PCA9685
    motors_.setAllMotorsPwm(outputs_.m1, outputs_.m2, outputs_.m3, outputs_.m4);
}

void MotorMixer::applySaturationLimits(float& m1, float& m2, float& m3, float& m4, float maxPulse) {
    float minPulse = (float)ESC_IDLE_PULSE_US;

    // Tìm giá trị xung cao nhất và thấp nhất trong 4 động cơ
    float maxVal = m1;
    if (m2 > maxVal) maxVal = m2;
    if (m3 > maxVal) maxVal = m3;
    if (m4 > maxVal) maxVal = m4;

    float minVal = m1;
    if (m2 < minVal) minVal = m2;
    if (m3 < minVal) minVal = m3;
    if (m4 < minVal) minVal = m4;

    // Nếu vượt quá trần xung tối đa: Hạ đều cả 4 động cơ xuống để giữ nguyên mô-men quay cân bằng
    if (maxVal > maxPulse) {
        float excess = maxVal - maxPulse;
        m1 -= excess;
        m2 -= excess;
        m3 -= excess;
        m4 -= excess;
    }

    // Nếu thấp hơn mức Idle: Nâng đều lên
    if (minVal < minPulse) {
        float deficit = minPulse - minVal;
        m1 += deficit;
        m2 += deficit;
        m3 += deficit;
        m4 += deficit;
    }

    // Kẹp chắc chắn trong miền [ESC_IDLE_PULSE_US, maxPulse]
    if (m1 < minPulse) m1 = minPulse; else if (m1 > maxPulse) m1 = maxPulse;
    if (m2 < minPulse) m2 = minPulse; else if (m2 > maxPulse) m2 = maxPulse;
    if (m3 < minPulse) m3 = minPulse; else if (m3 > maxPulse) m3 = maxPulse;
    if (m4 < minPulse) m4 = minPulse; else if (m4 > maxPulse) m4 = maxPulse;
}

MotorOutputs MotorMixer::mix(float throttlePercent, float rollCommand, float pitchCommand, float yawCommand) {
    float maxAllowedPulse = ESC_MIN_PULSE_US + (float)motors_.getMaxTestThrottle() * 10.0f;
    float baseThrottle = ESC_IDLE_PULSE_US + (throttlePercent / 100.0f) * (maxAllowedPulse - ESC_IDLE_PULSE_US);

    float m1 = baseThrottle - rollCommand + pitchCommand + yawCommand;
    float m2 = baseThrottle + rollCommand + pitchCommand - yawCommand;
    float m3 = baseThrottle - rollCommand - pitchCommand - yawCommand;
    float m4 = baseThrottle + rollCommand - pitchCommand + yawCommand;

    applySaturationLimits(m1, m2, m3, m4, maxAllowedPulse);

    MotorOutputs mo;
    mo.m1 = (uint16_t)m1;
    mo.m2 = (uint16_t)m2;
    mo.m3 = (uint16_t)m3;
    mo.m4 = (uint16_t)m4;
    return mo;
}
