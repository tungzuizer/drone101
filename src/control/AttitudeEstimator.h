#ifndef ATTITUDE_ESTIMATOR_H
#define ATTITUDE_ESTIMATOR_H

#include <Arduino.h>
#include "../sensors/ImuSensor.h"
#include "../sensors/Magnetometer.h"

// Cấu trúc chứa dữ liệu tư thế không gian 3D của Drone
struct AttitudeData {
    float roll;             // Góc nghiêng ngang (°): Dương = Nghiêng sang phải
    float pitch;            // Góc chúc/ngóc dọc (°): Dương = Ngóc mũi lên trên
    float yaw;              // Góc xoay hướng mũi (°): 0 - 360° theo la bàn

    float rollRad;          // Radian
    float pitchRad;
    float yawRad;

    // Vận tốc góc thực tế (deg/s) cho vòng lặp PID Rate (Inner Loop)
    float rateRoll;
    float ratePitch;
    float rateYaw;

    // Quaternion đơn vị (q0 + q1*i + q2*j + q3*k)
    float q0, q1, q2, q3;
};

class AttitudeEstimator {
public:
    AttitudeEstimator();

    // Khởi tạo thuật toán ước lượng tư thế với hệ số Kp, Ki
    void begin(float kp = 2.0f, float ki = 0.005f);

    // Cập nhật tư thế 6-DOF (chỉ dùng Gia tốc kế & Con quay hồi chuyển từ MPU6050)
    void update6DOF(const ImuData& imu, float dtSeconds);

    // Cập nhật tư thế 9-DOF (kết hợp MPU6050 + Magnetometer để khoá góc Yaw tuyệt đối)
    void update9DOF(const ImuData& imu, const MagData& mag, float dtSeconds);

    // Lấy dữ liệu tư thế
    const AttitudeData& getAttitude() const { return attitude_; }
    float getRoll() const { return attitude_.roll; }
    float getPitch() const { return attitude_.pitch; }
    float getYaw() const { return attitude_.yaw; }

    float getRateRoll() const { return attitude_.rateRoll; }
    float getRatePitch() const { return attitude_.ratePitch; }
    float getRateYaw() const { return attitude_.rateYaw; }

    // Thiết lập hệ số bộ lọc
    void setFilterGains(float kp, float ki) { kp_ = kp; ki_ = ki; }

    // Reset Quaternion về vị trí cân bằng
    void reset();

private:
    AttitudeData attitude_;

    // Hệ số thuật toán Mahony Filter
    float kp_;
    float ki_;

    // Thành phần tích phân sai số
    float integralFBx_;
    float integralFBy_;
    float integralFBz_;

    // Chuyển đổi Quaternion sang góc Euler (Roll, Pitch, Yaw)
    void computeEulerAngles();
};

#endif // ATTITUDE_ESTIMATOR_H
