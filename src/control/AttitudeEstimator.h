#ifndef ATTITUDE_ESTIMATOR_H
#define ATTITUDE_ESTIMATOR_H

#include <Arduino.h>
#include "../sensors/ImuSensor.h"
#include "../sensors/Magnetometer.h"

// Chế độ thuật toán ước lượng tư thế
enum EstimatorAlgorithm {
    ESTIMATOR_MAHONY = 0,   // Mahony Complementary Filter (Nhẹ, ổn định, phản hồi nhanh)
    ESTIMATOR_MADGWICK = 1  // Madgwick Gradient Descent AHRS (Tối ưu hóa Quaternion vi sai)
};

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

    // Khởi tạo thuật toán ước lượng tư thế với hệ số Kp, Ki (Mahony) hoặc Beta (Madgwick)
    void begin(float kp = 2.0f, float ki = 0.005f, float beta = 0.08f);

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
    void setMadgwickBeta(float beta) { beta_ = beta; }
    void setAlgorithm(EstimatorAlgorithm algo) { algorithm_ = algo; }
    EstimatorAlgorithm getAlgorithm() const { return algorithm_; }

    // Bật/tắt loại bỏ sai số gia tốc tuyến tính (G-Force Rejection)
    void enableGForceRejection(bool enable) { gForceRejection_ = enable; }

    // Reset Quaternion về vị trí cân bằng
    void reset();

    // Fast Inverse Square Root (Quake III / Madgwick standard)
    static inline float invSqrt(float x) {
        if (x <= 0.0f) return 0.0f;
        union {
            float f;
            uint32_t i;
        } conv;
        conv.f = x;
        conv.i = 0x5f3759df - (conv.i >> 1);
        float y = conv.f;
        return y * (1.5f - 0.5f * x * y * y); // 1st iteration Newton-Raphson
    }

private:
    AttitudeData attitude_;
    EstimatorAlgorithm algorithm_;

    // Hệ số thuật toán Mahony Filter
    float kp_;
    float ki_;

    // Hệ số thuật toán Madgwick Filter (beta = sqrt(3/4) * GyroMeanErrorRad)
    float beta_;

    // Thành phần tích phân sai số (Mahony)
    float integralFBx_;
    float integralFBy_;
    float integralFBz_;

    // Tính năng loại bỏ gia tốc tuyến tính khi tăng ga/lượn gấp
    bool gForceRejection_;

    // Các hàm tính toán nội bộ
    void updateMahony6DOF(float gx, float gy, float gz, float ax, float ay, float az, float dt);
    void updateMahony9DOF(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt);
    void updateMadgwick6DOF(float gx, float gy, float gz, float ax, float ay, float az, float dt);
    void updateMadgwick9DOF(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt);

    // Chuyển đổi Quaternion sang góc Euler (Roll, Pitch, Yaw)
    void computeEulerAngles();
};

#endif // ATTITUDE_ESTIMATOR_H
