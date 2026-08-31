#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PidController {
public:
    PidController();
    PidController(float kp, float ki, float kd, float maxI = 300.0f, float maxOut = 400.0f);

    // Khởi tạo hệ số và giới hạn
    void begin(float kp, float ki, float kd, float maxI = 300.0f, float maxOut = 400.0f);

    // Tính toán PID thông thường (Vòng ngoài - Angle Loop)
    float update(float setpoint, float measurement, float dt);

    // Tính toán PID vận tốc góc (Vòng trong - Rate Loop) với Derivative on Measurement và Low-Pass Filter
    float updateRate(float targetRate, float currentGyroRate, float dt);

    // Cập nhật hệ số PID trực tiếp (Dùng khi Tune từ GCS)
    void setGains(float kp, float ki, float kd);

    // Thiết lập giới hạn chống bão hòa tích phân (Anti-Windup) và giới hạn ngõ ra
    void setLimits(float maxI, float maxOut);

    // Thiết lập hệ số lọc thông thấp cho thành phần vi phân D (0.0 = tắt, 0.1 - 0.9 = lọc)
    void setDTermFilter(float alpha) { dFilterAlpha_ = alpha; }

    // Reset toàn bộ trạng thái bộ điều khiển
    void reset();

    // Reset riêng thành phần tích phân I-term (Khi ga = 0 hoặc Disarm)
    void resetIntegral() { integral_ = 0.0f; }

    // Các hàm đọc giá trị phục vụ truyền dữ liệu Telemetry hiển thị trên biểu đồ GCS
    float getKp() const { return kp_; }
    float getKi() const { return ki_; }
    float getKd() const { return kd_; }
    float getP() const { return pTerm_; }
    float getI() const { return iTerm_; }
    float getD() const { return dTerm_; }
    float getOutput() const { return output_; }

private:
    float kp_;
    float ki_;
    float kd_;

    float maxI_;            // Giới hạn Anti-Windup cho I-term
    float maxOutput_;       // Giới hạn ngõ ra tối đa

    float integral_;        // Tích phân sai số tích lũy
    float prevMeasurement_; // Giá trị đo lường chu kỳ trước (Dùng cho Derivative on Measurement)
    float prevError_;       // Sai số chu kỳ trước
    float filteredDTerm_;   // Giá trị D-term sau khi qua bộ lọc thông thấp (Low-Pass Filter)
    float dFilterAlpha_;    // Hệ số lọc D-term (Mặc định 0.7 tương đương cắt tần số rung ~30Hz)

    float pTerm_;
    float iTerm_;
    float dTerm_;
    float output_;
};

#endif // PID_CONTROLLER_H
