#include "MotorMixer.h"

MotorMixer::MotorMixer(MotorController& motorController)
    : motors_(motorController),
      airModeEnabled_(false) {
    outputs_.m1 = ESC_MIN_PULSE_US;
    outputs_.m2 = ESC_MIN_PULSE_US;
    outputs_.m3 = ESC_MIN_PULSE_US;
    outputs_.m4 = ESC_MIN_PULSE_US;

    rates_.rcRate = 1.0f;
    rates_.superRate = 0.70f;
    rates_.expo = 0.15f;
}

void MotorMixer::begin() {
    // 1. Cấu hình PID Vòng ngoài (Euler Angle Loop)
    // Kp = 4.5: Với sai số 10° nghiêng sẽ yêu cầu vận tốc bẻ góc 45°/s
    rollAnglePid_.begin(4.5f, 0.0f, 0.0f, 0.0f, 300.0f);
    pitchAnglePid_.begin(4.5f, 0.0f, 0.0f, 0.0f, 300.0f);

    // 2. Cấu hình PID Vòng trong (Rate Loop - Gyro 250Hz)
    // Hệ số khởi điểm an toàn cho động cơ A2212 1000KV + ESC 30A + Cánh 1045
    rollRatePid_.begin(1.20f, 0.04f, 0.035f, 150.0f, 400.0f);
    pitchRatePid_.begin(1.20f, 0.04f, 0.035f, 150.0f, 400.0f);
    yawRatePid_.begin(2.50f, 0.08f, 0.0f, 150.0f, 400.0f);

    // Cấu hình lọc thông thấp triệt tiêu rung động cơ cho vi phân D-term
    rollRatePid_.setDTermFilter(0.70f);
    pitchRatePid_.setDTermFilter(0.70f);
    yawRatePid_.setDTermFilter(0.70f);

    // Cấu hình TPA (Throttle PID Attenuation): Giảm 20% PID ở ga cao (>50% ga) để triệt tiêu rung
    setTpa(0.20f, 0.50f);

    // Cấu hình Feedforward bù quán tính cần gạt stick
    setFeedforward(0.012f, 0.0f);

    resetPids();
}

void MotorMixer::setRates(float rcRate, float superRate, float expo) {
    rates_.rcRate = rcRate;
    rates_.superRate = superRate;
    rates_.expo = expo;
}

void MotorMixer::setTpa(float rate, float breakpoint) {
    rollRatePid_.setTpa(rate, breakpoint);
    pitchRatePid_.setTpa(rate, breakpoint);
}

void MotorMixer::setFeedforward(float rollPitchKff, float yawKff) {
    rollRatePid_.setFeedforward(rollPitchKff);
    pitchRatePid_.setFeedforward(rollPitchKff);
    yawRatePid_.setFeedforward(yawKff);
}

float MotorMixer::calculateBetaflightRate(float stickDeflectionNorm, float rcRate, float superRate, float expo) {
    // stickDeflectionNorm trong khoảng [-1.0, +1.0]
    if (stickDeflectionNorm > 1.0f) stickDeflectionNorm = 1.0f;
    else if (stickDeflectionNorm < -1.0f) stickDeflectionNorm = -1.0f;

    float absStick = fabsf(stickDeflectionNorm);

    // Đường cong Expo
    float stickCubed = stickDeflectionNorm * stickDeflectionNorm * stickDeflectionNorm;
    float rcFactor = stickDeflectionNorm * (1.0f - expo) + stickCubed * expo;

    // Hệ số Super Rate
    float superFactor = 1.0f;
    float denominator = 1.0f - (absStick * superRate);
    if (denominator > 0.01f) {
        superFactor = 1.0f / denominator;
    }

    // Đổi sang Vận tốc góc danh định (deg/s) - Tối đa ~600-800°/s ở full stick
    return rcFactor * rcRate * superFactor * 200.0f;
}

void MotorMixer::resetPids() {
    rollAnglePid_.reset();
    pitchAnglePid_.reset();
    rollRatePid_.reset();
    pitchRatePid_.reset();
    yawRatePid_.reset();
}

void MotorMixer::update(const ControlData& control, const AttitudeData& attitude, float dtSeconds) {
    // 1. Kiểm tra an toàn: Nếu chưa ARM -> Dừng toàn bộ động cơ
    if (!motors_.isArmed()) {
        resetPids();
        motors_.setAllMotorsPwm(ESC_MIN_PULSE_US, ESC_MIN_PULSE_US, ESC_MIN_PULSE_US, ESC_MIN_PULSE_US);
        outputs_.m1 = ESC_MIN_PULSE_US;
        outputs_.m2 = ESC_MIN_PULSE_US;
        outputs_.m3 = ESC_MIN_PULSE_US;
        outputs_.m4 = ESC_MIN_PULSE_US;
        return;
    }

    // Nếu ARM nhưng ga = 0 và KHÔNG bật AirMode -> Quay Idle và reset I-term
    if (control.throttle <= 0.5f && !airModeEnabled_) {
        resetPids();
        motors_.setAllMotorsPwm(ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US, ESC_IDLE_PULSE_US);
        outputs_.m1 = ESC_IDLE_PULSE_US;
        outputs_.m2 = ESC_IDLE_PULSE_US;
        outputs_.m3 = ESC_IDLE_PULSE_US;
        outputs_.m4 = ESC_IDLE_PULSE_US;
        return;
    }

    float targetRateRoll = 0.0f;
    float targetRatePitch = 0.0f;
    float targetRateYaw = control.yaw; // Lệnh Yaw mặc định theo deg/s

    // 2. VÒNG NGOÀI (Outer Angle Loop)
    if (control.flightMode == MODE_ANGLE || control.flightMode == MODE_ALT_HOLD || control.flightMode == MODE_POS_HOLD) {
        // Chế độ tự cân bằng: Đưa sai số góc nghiêng qua PID Angle để tính vận tốc góc mong muốn
        targetRateRoll  = rollAnglePid_.update(control.roll, attitude.roll, dtSeconds);
        targetRatePitch = pitchAnglePid_.update(control.pitch, attitude.pitch, dtSeconds);
    } else {
        // Chế độ Acro: Áp dụng đường cong Betaflight Rates cho chuyển động nhào lộn mượt mà
        targetRateRoll  = calculateBetaflightRate(control.roll / 45.0f, rates_.rcRate, rates_.superRate, rates_.expo);
        targetRatePitch = calculateBetaflightRate(control.pitch / 45.0f, rates_.rcRate, rates_.superRate, rates_.expo);
    }

    // 3. VÒNG TRONG (Inner Rate Loop - 250Hz Gyro Feedback) tích hợp TPA & Feedforward
    float normalizedThrottle = control.throttle / 100.0f;
    float rollCmd  = rollRatePid_.updateRate(targetRateRoll, attitude.rateRoll, dtSeconds, normalizedThrottle);
    float pitchCmd = pitchRatePid_.updateRate(targetRatePitch, attitude.ratePitch, dtSeconds, normalizedThrottle);
    float yawCmd   = yawRatePid_.updateRate(targetRateYaw, attitude.rateYaw, dtSeconds, normalizedThrottle);

    // 4. TÍNH TOÁN XUNG GA CƠ BẢN (Base Throttle Pulse)
    uint8_t maxThrottlePercent = motors_.getMaxTestThrottle();
    float maxAllowedPulse = ESC_MIN_PULSE_US + (float)maxThrottlePercent * 10.0f; // 30% -> 1300µs, 100% -> 2000µs
    float minPulse = (float)ESC_IDLE_PULSE_US;

    float baseThrottle = minPulse + normalizedThrottle * (maxAllowedPulse - minPulse);

    // 5. MA TRẬN TRỘN TÍN HIỆU QUAD-X (Quad-X Motor Mixer)
    // M1: Trước Phải (CCW) = Ga - Roll - Pitch + Yaw
    // M2: Trước Trái (CW)  = Ga + Roll - Pitch - Yaw
    // M3: Sau Phải (CW)   = Ga - Roll + Pitch - Yaw
    // M4: Sau Trái (CCW)  = Ga + Roll + Pitch + Yaw
    float m1 = baseThrottle - rollCmd - pitchCmd + yawCmd;
    float m2 = baseThrottle + rollCmd - pitchCmd - yawCmd;
    float m3 = baseThrottle - rollCmd + pitchCmd - yawCmd;
    float m4 = baseThrottle + rollCmd + pitchCmd + yawCmd;

    // 6. XỬ LÝ CHỐNG BÃO HÒA MÔ-MEN XOẮN (Proportional Torque Saturation Scaling)
    applySaturationLimits(m1, m2, m3, m4, maxAllowedPulse, minPulse);

    outputs_.m1 = (uint16_t)m1;
    outputs_.m2 = (uint16_t)m2;
    outputs_.m3 = (uint16_t)m3;
    outputs_.m4 = (uint16_t)m4;

    // 7. Ghi xung ra Driver Motor
    motors_.setAllMotorsPwm(outputs_.m1, outputs_.m2, outputs_.m3, outputs_.m4);
}

void MotorMixer::applySaturationLimits(float& m1, float& m2, float& m3, float& m4, float maxPulse, float minPulse) {
    // Tìm giá trị xung cao nhất và thấp nhất trong 4 động cơ
    float maxVal = m1;
    if (m2 > maxVal) maxVal = m2;
    if (m3 > maxVal) maxVal = m3;
    if (m4 > maxVal) maxVal = m4;

    float minVal = m1;
    if (m2 < minVal) minVal = m2;
    if (m3 < minVal) minVal = m3;
    if (m4 < minVal) minVal = m4;

    // Nếu vượt quá trần xung tối đa: Hạ đều cả 4 động cơ xuống để bảo toàn mô-men quay roll/pitch
    if (maxVal > maxPulse) {
        float excess = maxVal - maxPulse;
        m1 -= excess;
        m2 -= excess;
        m3 -= excess;
        m4 -= excess;
    }

    // Nếu thấp hơn mức Idle (Đặc biệt khi hạ ga nhanh hoặc bật AirMode): Nâng đều cả 4 động cơ lên
    if (minVal < minPulse) {
        float deficit = minPulse - minVal;
        m1 += deficit;
        m2 += deficit;
        m3 += deficit;
        m4 += deficit;
    }

    // Kẹp chắc chắn trong miền an toàn [minPulse, maxPulse]
    if (m1 < minPulse) m1 = minPulse; else if (m1 > maxPulse) m1 = maxPulse;
    if (m2 < minPulse) m2 = minPulse; else if (m2 > maxPulse) m2 = maxPulse;
    if (m3 < minPulse) m3 = minPulse; else if (m3 > maxPulse) m3 = maxPulse;
    if (m4 < minPulse) m4 = minPulse; else if (m4 > maxPulse) m4 = maxPulse;
}

MotorOutputs MotorMixer::mix(float throttlePercent, float rollCommand, float pitchCommand, float yawCommand) {
    float maxAllowedPulse = ESC_MIN_PULSE_US + (float)motors_.getMaxTestThrottle() * 10.0f;
    float minPulse = (float)ESC_IDLE_PULSE_US;
    float baseThrottle = minPulse + (throttlePercent / 100.0f) * (maxAllowedPulse - minPulse);

    float m1 = baseThrottle - rollCommand - pitchCommand + yawCommand;
    float m2 = baseThrottle + rollCommand - pitchCommand - yawCommand;
    float m3 = baseThrottle - rollCommand + pitchCommand - yawCommand;
    float m4 = baseThrottle + rollCommand + pitchCommand + yawCommand;

    applySaturationLimits(m1, m2, m3, m4, maxAllowedPulse, minPulse);

    MotorOutputs mo;
    mo.m1 = (uint16_t)m1;
    mo.m2 = (uint16_t)m2;
    mo.m3 = (uint16_t)m3;
    mo.m4 = (uint16_t)m4;
    return mo;
}
