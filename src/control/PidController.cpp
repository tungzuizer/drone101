#include "PidController.h"

PidController::PidController()
    : kp_(0.0f),
      ki_(0.0f),
      kd_(0.0f),
      kff_(0.0f),
      maxI_(300.0f),
      maxOutput_(400.0f),
      tpaRate_(0.0f),
      tpaBreakpoint_(0.50f),
      integral_(0.0f),
      prevMeasurement_(0.0f),
      prevTargetRate_(0.0f),
      prevError_(0.0f),
      filteredDTerm_(0.0f),
      dFilterAlpha_(0.7f),
      useBiquad_(false),
      pTerm_(0.0f),
      iTerm_(0.0f),
      dTerm_(0.0f),
      ffTerm_(0.0f),
      output_(0.0f) {
}

PidController::PidController(float kp, float ki, float kd, float maxI, float maxOut)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      kff_(0.0f),
      maxI_(maxI),
      maxOutput_(maxOut),
      tpaRate_(0.0f),
      tpaBreakpoint_(0.50f),
      integral_(0.0f),
      prevMeasurement_(0.0f),
      prevTargetRate_(0.0f),
      prevError_(0.0f),
      filteredDTerm_(0.0f),
      dFilterAlpha_(0.7f),
      useBiquad_(false),
      pTerm_(0.0f),
      iTerm_(0.0f),
      dTerm_(0.0f),
      ffTerm_(0.0f),
      output_(0.0f) {
}

void PidController::begin(float kp, float ki, float kd, float maxI, float maxOut) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
    maxI_ = maxI;
    maxOutput_ = maxOut;
    reset();
}

void PidController::setGains(float kp, float ki, float kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

void PidController::setLimits(float maxI, float maxOut) {
    maxI_ = maxI;
    maxOutput_ = maxOut;
}

void PidController::reset() {
    integral_ = 0.0f;
    prevMeasurement_ = 0.0f;
    prevTargetRate_ = 0.0f;
    prevError_ = 0.0f;
    filteredDTerm_ = 0.0f;
    pTerm_ = 0.0f;
    iTerm_ = 0.0f;
    dTerm_ = 0.0f;
    ffTerm_ = 0.0f;
    output_ = 0.0f;
    isFirstUpdate_ = true;
    if (useBiquad_) {
        dTermBiquad_.reset(0.0f);
    }
}

float PidController::update(float setpoint, float measurement, float dt) {
    if (dt <= 0.0f) return output_;

    // 1. Tính sai số (Error)
    float error = setpoint - measurement;
    if (isFirstUpdate_) {
        prevError_ = error;
        prevMeasurement_ = measurement;
        isFirstUpdate_ = false;
    }

    // 2. Thành phần Tỷ lệ (P-term)
    pTerm_ = kp_ * error;

    // 3. Thành phần Tích phân (I-term) với Anti-Windup
    integral_ += error * ki_ * dt;
    if (integral_ > maxI_) integral_ = maxI_;
    else if (integral_ < -maxI_) integral_ = -maxI_;
    iTerm_ = integral_;

    // 4. Thành phần Vi phân (D-term)
    float derivative = (error - prevError_) / dt;
    dTerm_ = kd_ * derivative;
    ffTerm_ = 0.0f;

    // 5. Tổng hợp ngõ ra và giới hạn bão hòa
    output_ = pTerm_ + iTerm_ + dTerm_;
    if (output_ > maxOutput_) output_ = maxOutput_;
    else if (output_ < -maxOutput_) output_ = -maxOutput_;

    prevError_ = error;
    prevMeasurement_ = measurement;
    return output_;
}

float PidController::updateRate(float targetRate, float currentGyroRate, float dt, float throttleNormalized) {
    if (dt <= 0.0f) return output_;

    // 1. Tính hệ số suy giảm TPA (Throttle PID Attenuation) khi ép ga cao
    float tpaFactor = 1.0f;
    if (tpaRate_ > 0.0f && throttleNormalized > tpaBreakpoint_ && tpaBreakpoint_ < 1.0f) {
        float tpaRange = 1.0f - tpaBreakpoint_;
        float tpaProgress = (throttleNormalized - tpaBreakpoint_) / tpaRange;
        if (tpaProgress > 1.0f) tpaProgress = 1.0f;
        tpaFactor = 1.0f - (tpaRate_ * tpaProgress);
        if (tpaFactor < 0.2f) tpaFactor = 0.2f;
    }

    // 2. Tính sai số vận tốc góc (Rate Error)
    float error = targetRate - currentGyroRate;

    // Khởi tạo trạng thái chu kỳ đầu tiên tránh giật derivative/feedforward kick khi vừa Arm
    if (isFirstUpdate_) {
        prevMeasurement_ = currentGyroRate;
        prevTargetRate_ = targetRate;
        prevError_ = error;
        isFirstUpdate_ = false;
    }

    // 3. Thành phần Tỷ lệ (P-term) có áp dụng TPA
    pTerm_ = kp_ * tpaFactor * error;

    // 4. Thành phần Tích phân (I-term) với Anti-Windup
    integral_ += error * ki_ * dt;
    if (integral_ > maxI_) integral_ = maxI_;
    else if (integral_ < -maxI_) integral_ = -maxI_;
    iTerm_ = integral_;

    // 5. Thành phần Vi phân trên giá trị đo lường (Derivative on Measurement)
    // Loại bỏ hiện tượng "Derivative Kick" khi giật cần lái đột ngột
    float deltaMeasurement = (currentGyroRate - prevMeasurement_) / dt;
    float rawD = -kd_ * tpaFactor * deltaMeasurement;

    // 6. Lọc nhiễu D-term (Biquad 2nd-order hoặc PT1 Low-Pass Filter)
    if (useBiquad_) {
        filteredDTerm_ = dTermBiquad_.update(rawD);
    } else {
        filteredDTerm_ = (dFilterAlpha_ * filteredDTerm_) + ((1.0f - dFilterAlpha_) * rawD);
    }
    dTerm_ = filteredDTerm_;

    // 7. Thành phần Bù trước (Feedforward - FF)
    // Tăng tốc độ đáp ứng tức thời của drone theo chuyển động cần gạt stick
    if (kff_ > 0.0f) {
        float deltaTarget = (targetRate - prevTargetRate_) / dt;
        ffTerm_ = kff_ * deltaTarget;
    } else {
        ffTerm_ = 0.0f;
    }

    // 8. Tổng hợp ngõ ra điều khiển và kẹp bão hòa
    output_ = pTerm_ + iTerm_ + dTerm_ + ffTerm_;
    if (output_ > maxOutput_) output_ = maxOutput_;
    else if (output_ < -maxOutput_) output_ = -maxOutput_;

    prevMeasurement_ = currentGyroRate;
    prevTargetRate_ = targetRate;
    prevError_ = error;
    return output_;
}
