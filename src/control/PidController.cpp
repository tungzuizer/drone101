#include "PidController.h"

PidController::PidController()
    : kp_(0.0f),
      ki_(0.0f),
      kd_(0.0f),
      maxI_(300.0f),
      maxOutput_(400.0f),
      integral_(0.0f),
      prevMeasurement_(0.0f),
      prevError_(0.0f),
      filteredDTerm_(0.0f),
      dFilterAlpha_(0.7f),
      pTerm_(0.0f),
      iTerm_(0.0f),
      dTerm_(0.0f),
      output_(0.0f) {
}

PidController::PidController(float kp, float ki, float kd, float maxI, float maxOut)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      maxI_(maxI),
      maxOutput_(maxOut),
      integral_(0.0f),
      prevMeasurement_(0.0f),
      prevError_(0.0f),
      filteredDTerm_(0.0f),
      dFilterAlpha_(0.7f),
      pTerm_(0.0f),
      iTerm_(0.0f),
      dTerm_(0.0f),
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
    prevError_ = 0.0f;
    filteredDTerm_ = 0.0f;
    pTerm_ = 0.0f;
    iTerm_ = 0.0f;
    dTerm_ = 0.0f;
    output_ = 0.0f;
}

float PidController::update(float setpoint, float measurement, float dt) {
    if (dt <= 0.0f) return output_;

    // 1. Tính sai số (Error)
    float error = setpoint - measurement;

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

    // 5. Tổng hợp ngõ ra và giới hạn bão hòa
    output_ = pTerm_ + iTerm_ + dTerm_;
    if (output_ > maxOutput_) output_ = maxOutput_;
    else if (output_ < -maxOutput_) output_ = -maxOutput_;

    prevError_ = error;
    prevMeasurement_ = measurement;
    return output_;
}

float PidController::updateRate(float targetRate, float currentGyroRate, float dt) {
    if (dt <= 0.0f) return output_;

    // 1. Tính sai số vận tốc góc (Rate Error)
    float error = targetRate - currentGyroRate;

    // 2. Thành phần Tỷ lệ (P-term)
    pTerm_ = kp_ * error;

    // 3. Thành phần Tích phân (I-term) với Anti-Windup
    integral_ += error * ki_ * dt;
    if (integral_ > maxI_) integral_ = maxI_;
    else if (integral_ < -maxI_) integral_ = -maxI_;
    iTerm_ = integral_;

    // 4. Thành phần Vi phân trên giá trị đo lường (Derivative on Measurement)
    // Giúp loại bỏ hiện tượng "Derivative Kick" khi người lái đổi lệnh ga/góc đột ngột
    float deltaMeasurement = (currentGyroRate - prevMeasurement_) / dt;
    float rawD = -kd_ * deltaMeasurement;

    // 5. Bộ lọc thông thấp (1st-Order Low-Pass Filter) cho D-term để triệt tiêu rung động cơ
    filteredDTerm_ = (dFilterAlpha_ * filteredDTerm_) + ((1.0f - dFilterAlpha_) * rawD);
    dTerm_ = filteredDTerm_;

    // 6. Tổng hợp ngõ ra điều khiển
    output_ = pTerm_ + iTerm_ + dTerm_;
    if (output_ > maxOutput_) output_ = maxOutput_;
    else if (output_ < -maxOutput_) output_ = -maxOutput_;

    prevMeasurement_ = currentGyroRate;
    prevError_ = error;
    return output_;
}
