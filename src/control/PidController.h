#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "Filter.h"

class PidController {
public:
    PidController();
    PidController(float kp, float ki, float kd, float maxI = 300.0f, float maxOut = 400.0f);

    // Khởi tạo hệ số và giới hạn
    void begin(float kp, float ki, float kd, float maxI = 300.0f, float maxOut = 400.0f);

    // Tính toán PID thông thường (Vòng ngoài - Angle Loop)
    float update(float setpoint, float measurement, float dt);

    // Tính toán PID vận tốc góc (Vòng trong - Rate Loop)
    // Tích hợp Derivative on Measurement, Feedforward, D-Term Low-Pass Filter, và TPA (Throttle PID Attenuation)
    float updateRate(float targetRate, float currentGyroRate, float dt, float throttleNormalized = 0.0f);

    // Cập nhật hệ số PID trực tiếp (Dùng khi Tune từ GCS)
    void setGains(float kp, float ki, float kd);

    // Cập nhật hệ số Feedforward (Kff: Bù trước quán tính từ cần lái)
    void setFeedforward(float kff) { kff_ = kff; }

    // Cấu hình Throttle PID Attenuation (TPA) giảm dao động rung khi ép ga cao
    // rate: Tỷ lệ giảm tối đa (0.0 = tắt, 0.30 = giảm 30% ở ga tối đa)
    // breakpoint: Điểm bắt đầu kích hoạt giảm PID (0.50 = 50% ga)
    void setTpa(float rate, float breakpoint = 0.50f) {
        tpaRate_ = rate;
        tpaBreakpoint_ = breakpoint;
    }

    // Thiết lập giới hạn chống bão hòa tích phân (Anti-Windup) và giới hạn ngõ ra
    void setLimits(float maxI, float maxOut);

    // Thiết lập hệ số lọc thông thấp PT1 cho thành phần vi phân D (0.0 = tắt, 0.1 - 0.9 = lọc)
    void setDTermFilter(float alpha) { dFilterAlpha_ = alpha; }

    // Thiết lập bộ lọc thông thấp Biquad 2nd-order cho D-term
    void setDTermBiquad(float cutoffFreqHz, float sampleRateHz) {
        useBiquad_ = true;
        dTermBiquad_.initLpf(cutoffFreqHz, sampleRateHz);
    }

    // Reset toàn bộ trạng thái bộ điều khiển
    void reset();

    // Reset riêng thành phần tích phân I-term (Khi ga = 0 hoặc Disarm)
    void resetIntegral() { integral_ = 0.0f; }

    // Các hàm đọc giá trị phục vụ truyền dữ liệu Telemetry hiển thị trên biểu đồ GCS
    float getKp() const { return kp_; }
    float getKi() const { return ki_; }
    float getKd() const { return kd_; }
    float getKff() const { return kff_; }
    float getP() const { return pTerm_; }
    float getI() const { return iTerm_; }
    float getD() const { return dTerm_; }
    float getFf() const { return ffTerm_; }
    float getOutput() const { return output_; }

private:
    float kp_;
    float ki_;
    float kd_;
    float kff_;             // Hệ số Feedforward (Kff)

    float maxI_;            // Giới hạn Anti-Windup cho I-term
    float maxOutput_;       // Giới hạn ngõ ra tối đa

    float tpaRate_;         // TPA attenuation factor (0.0 - 0.5)
    float tpaBreakpoint_;   // TPA activation threshold (0.0 - 1.0)

    float integral_;        // Tích phân sai số tích lũy
    float prevMeasurement_; // Giá trị đo lường chu kỳ trước (Dùng cho Derivative on Measurement)
    float prevTargetRate_;  // Lệnh vận tốc góc chu kỳ trước (Dùng cho Feedforward)
    float prevError_;       // Sai số chu kỳ trước
    float filteredDTerm_;   // Giá trị D-term sau khi qua bộ lọc
    float dFilterAlpha_;    // Hệ số lọc PT1 cho D-term

    bool isFirstUpdate_;
    bool useBiquad_;
    BiquadFilter dTermBiquad_;

    float pTerm_;
    float iTerm_;
    float dTerm_;
    float ffTerm_;
    float output_;
};

#endif // PID_CONTROLLER_H
