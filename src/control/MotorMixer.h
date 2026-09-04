#ifndef MOTOR_MIXER_H
#define MOTOR_MIXER_H

#include <Arduino.h>
#include "../actuators/MotorController.h"
#include "PidController.h"
#include "ControlInputSource.h"
#include "AttitudeEstimator.h"

// Cấu trúc chứa giá trị xung PWM cấp cho 4 động cơ (Microseconds)
struct MotorOutputs {
    uint16_t m1;    // Trước Phải (Front-Right, CCW)
    uint16_t m2;    // Trước Trái (Front-Left, CW)
    uint16_t m3;    // Sau Phải (Rear-Right, CW)
    uint16_t m4;    // Sau Trái (Rear-Left, CCW)
};

// Cấu trúc tham số đường cong điều khiển Rates (Chuẩn Betaflight)
struct FlightRates {
    float rcRate;       // Độ nhạy cơ bản tâm gậy (0.5 - 2.5)
    float superRate;    // Độ tăng tốc góc ở biên gậy (0.0 - 0.90)
    float expo;         // Độ mềm mại quanh tâm cần gạt (0.0 - 0.80)
};

class MotorMixer {
public:
    MotorMixer(MotorController& motorController);

    // Khởi tạo các bộ điều khiển PID (Vòng ngoài Angle + Vòng trong Rate)
    void begin();

    // Tính toán vòng lặp PID kép và Trộn tín hiệu động cơ Quad-X
    void update(const ControlData& control, const AttitudeData& attitude, float dtSeconds);

    // Trộn trực tiếp các tín hiệu điều khiển (Dùng khi tính toán độc lập)
    MotorOutputs mix(float throttlePercent, float rollCommand, float pitchCommand, float yawCommand);

    // Lấy xung động cơ hiện tại
    const MotorOutputs& getOutputs() const { return outputs_; }

    // Reset toàn bộ tích phân PID (Gọi khi Disarm hoặc Cần ga = 0)
    void resetPids();

    // Cấu hình AirMode (Duy trì cân bằng PID hoàn hảo ngay cả khi ga = 0)
    void setAirMode(bool enable) { airModeEnabled_ = enable; }
    bool isAirModeEnabled() const { return airModeEnabled_; }

    // Cấu hình Rates (RC Rate, Super Rate, Expo)
    void setRates(float rcRate, float superRate, float expo);
    const FlightRates& getRates() const { return rates_; }

    // Cấu hình TPA (Throttle PID Attenuation) cho các trục Rate
    void setTpa(float rate, float breakpoint = 0.50f);

    // Cấu hình Feedforward cho Roll/Pitch và Yaw
    void setFeedforward(float rollPitchKff, float yawKff);

    // Hàm tiện ích chuyển đổi độ lệch cần gạt sang Vận tốc góc theo mô hình Betaflight Rates
    static float calculateBetaflightRate(float stickDeflectionNorm, float rcRate, float superRate, float expo);

    // Truy cập các bộ điều khiển PID để Tune tham số từ GCS
    PidController& getRollAnglePid() { return rollAnglePid_; }
    PidController& getPitchAnglePid() { return pitchAnglePid_; }
    PidController& getRollRatePid() { return rollRatePid_; }
    PidController& getPitchRatePid() { return pitchRatePid_; }
    PidController& getYawRatePid() { return yawRatePid_; }

private:
    MotorController& motors_;
    MotorOutputs outputs_;
    FlightRates rates_;
    bool airModeEnabled_;

    // Bộ điều khiển PID 2 tầng (Dual-Loop PID)
    // Vòng ngoài: Góc Euler (Angle Loop) -> Xuất ra vận tốc góc mong muốn (deg/s)
    PidController rollAnglePid_;
    PidController pitchAnglePid_;

    // Vòng trong: Vận tốc góc Gyro (Rate Loop) -> Xuất ra xung bù động cơ (µs)
    PidController rollRatePid_;
    PidController pitchRatePid_;
    PidController yawRatePid_;

    // Phân bổ và chống bão hòa công suất động cơ theo tỷ lệ mô-men xoắn (Proportional Torque Saturation)
    void applySaturationLimits(float& m1, float& m2, float& m3, float& m4, float maxPulse, float minPulse);
};

#endif // MOTOR_MIXER_H
