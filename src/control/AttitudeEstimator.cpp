#include "AttitudeEstimator.h"
#include <math.h>

#define DEG_TO_RAD_F (0.017453292519943295f) // PI / 180
#define RAD_TO_DEG_F (57.29577951308232f)    // 180 / PI

AttitudeEstimator::AttitudeEstimator()
    : kp_(2.5f),
      ki_(0.005f),
      integralFBx_(0.0f),
      integralFBy_(0.0f),
      integralFBz_(0.0f) {
    reset();
}

void AttitudeEstimator::begin(float kp, float ki) {
    kp_ = kp;
    ki_ = ki;
    reset();
}

void AttitudeEstimator::reset() {
    attitude_.q0 = 1.0f;
    attitude_.q1 = 0.0f;
    attitude_.q2 = 0.0f;
    attitude_.q3 = 0.0f;

    attitude_.roll = 0.0f;
    attitude_.pitch = 0.0f;
    attitude_.yaw = 0.0f;

    attitude_.rollRad = 0.0f;
    attitude_.pitchRad = 0.0f;
    attitude_.yawRad = 0.0f;

    attitude_.rateRoll = 0.0f;
    attitude_.ratePitch = 0.0f;
    attitude_.rateYaw = 0.0f;

    integralFBx_ = 0.0f;
    integralFBy_ = 0.0f;
    integralFBz_ = 0.0f;
}

void AttitudeEstimator::update6DOF(const ImuData& imu, float dtSeconds) {
    if (dtSeconds <= 0.0f || dtSeconds > 0.1f) {
        dtSeconds = 0.004f; // Mặc định 250Hz nếu dt không hợp lệ
    }

    // Lưu vận tốc góc (deg/s) cho vòng điều khiển PID Rate
    attitude_.rateRoll  = imu.gx;
    attitude_.ratePitch = imu.gy;
    attitude_.rateYaw   = imu.gz;

    // Chuyển đổi vận tốc góc từ Deg/s sang Rad/s
    float gx = imu.gx * DEG_TO_RAD_F;
    float gy = imu.gy * DEG_TO_RAD_F;
    float gz = imu.gz * DEG_TO_RAD_F;

    float ax = imu.ax;
    float ay = imu.ay;
    float az = imu.az;

    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    // Chuẩn hóa vector gia tốc trọng trường
    float accNorm = sqrtf(ax * ax + ay * ay + az * az);
    if (accNorm > 0.01f) {
        ax /= accNorm;
        ay /= accNorm;
        az /= accNorm;

        // Ước lượng hướng vector trọng lực từ Quaternion hiện tại
        // v = [2(q1*q3 - q0*q2), 2(q0*q1 + q2*q3), q0^2 - q1^2 - q2^2 + q3^2]
        float vx = 2.0f * (q1 * q3 - q0 * q2);
        float vy = 2.0f * (q0 * q1 + q2 * q3);
        float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        // Sai số đo được là tích có hướng (Cross Product) giữa gia tốc đo thực tế và gia tốc ước lượng
        float ex = (ay * vz - az * vy);
        float ey = (az * vx - ax * vz);
        float ez = (ax * vy - ay * vx);

        // Tích phân sai số (I-term) để triệt tiêu trôi Gyroscope
        if (ki_ > 0.0f) {
            integralFBx_ += ex * ki_ * dtSeconds;
            integralFBy_ += ey * ki_ * dtSeconds;
            integralFBz_ += ez * ki_ * dtSeconds;
            gx += integralFBx_;
            gy += integralFBy_;
            gz += integralFBz_;
        }

        // Bù sai số tỷ lệ (P-term)
        gx += kp_ * ex;
        gy += kp_ * ey;
        gz += kp_ * ez;
    }

    // Tích phân phương trình vi phân Quaternion: dq/dt = 0.5 * q * w
    float dq0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float dq1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float dq2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float dq3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    q0 += dq0 * dtSeconds;
    q1 += dq1 * dtSeconds;
    q2 += dq2 * dtSeconds;
    q3 += dq3 * dtSeconds;

    // Chuẩn hóa Quaternion về độ dài đơn vị (Unit Quaternion)
    float qNorm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (qNorm > 0.0001f) {
        attitude_.q0 = q0 / qNorm;
        attitude_.q1 = q1 / qNorm;
        attitude_.q2 = q2 / qNorm;
        attitude_.q3 = q3 / qNorm;
    }

    // Tính toán góc Euler
    computeEulerAngles();
}

void AttitudeEstimator::update9DOF(const ImuData& imu, const MagData& mag, float dtSeconds) {
    if (dtSeconds <= 0.0f || dtSeconds > 0.1f) {
        dtSeconds = 0.004f;
    }

    attitude_.rateRoll  = imu.gx;
    attitude_.ratePitch = imu.gy;
    attitude_.rateYaw   = imu.gz;

    float gx = imu.gx * DEG_TO_RAD_F;
    float gy = imu.gy * DEG_TO_RAD_F;
    float gz = imu.gz * DEG_TO_RAD_F;

    float ax = imu.ax;
    float ay = imu.ay;
    float az = imu.az;

    float mx = mag.mx;
    float my = mag.my;
    float mz = mag.mz;

    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    float ex = 0.0f, ey = 0.0f, ez = 0.0f;

    // 1. Thành phần sai số Gia tốc kế (Trọng lực)
    float accNorm = sqrtf(ax * ax + ay * ay + az * az);
    if (accNorm > 0.01f) {
        ax /= accNorm;
        ay /= accNorm;
        az /= accNorm;

        float vx = 2.0f * (q1 * q3 - q0 * q2);
        float vy = 2.0f * (q0 * q1 + q2 * q3);
        float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        ex += (ay * vz - az * vy);
        ey += (az * vx - ax * vz);
        ez += (ax * vy - ay * vx);
    }

    // 2. Thành phần sai số Từ kế (La bàn địa từ)
    float magNorm = sqrtf(mx * mx + my * my + mz * mz);
    if (magNorm > 0.01f) {
        mx /= magNorm;
        my /= magNorm;
        mz /= magNorm;

        // Vector từ trường tham chiếu: Chuyển vector từ trường từ hệ Body sang hệ Earth (h = R * m)
        float hx = 2.0f * mx * (0.5f - q2 * q2 - q3 * q3) + 2.0f * my * (q1 * q2 - q0 * q3) + 2.0f * mz * (q1 * q3 + q0 * q2);
        float hy = 2.0f * mx * (q1 * q2 + q0 * q3) + 2.0f * my * (0.5f - q1 * q1 - q3 * q3) + 2.0f * mz * (q2 * q3 - q0 * q1);
        float hz = 2.0f * mx * (q1 * q3 - q0 * q2) + 2.0f * my * (q2 * q3 + q0 * q1) + 2.0f * mz * (0.5f - q1 * q1 - q2 * q2);
        float bx = sqrtf(hx * hx + hy * hy);
        float bz = hz;

        // Ước lượng vector từ trường tham chiếu trong hệ Body (w = R^T * [bx, 0, bz]^T)
        float wx = 2.0f * bx * (0.5f - q2 * q2 - q3 * q3) + 2.0f * bz * (q1 * q3 - q0 * q2);
        float wy = 2.0f * bx * (q1 * q2 - q0 * q3) + 2.0f * bz * (q2 * q3 + q0 * q1);
        float wz = 2.0f * bx * (q1 * q3 + q0 * q2) + 2.0f * bz * (0.5f - q1 * q1 - q2 * q2);

        // Sai số từ trường
        ex += (my * wz - mz * wy);
        ey += (mz * wx - mx * wz);
        ez += (mx * wy - my * wx);
    }

    // Bù sai số tích phân và tỷ lệ
    if (ki_ > 0.0f) {
        integralFBx_ += ex * ki_ * dtSeconds;
        integralFBy_ += ey * ki_ * dtSeconds;
        integralFBz_ += ez * ki_ * dtSeconds;
        gx += integralFBx_;
        gy += integralFBy_;
        gz += integralFBz_;
    }

    gx += kp_ * ex;
    gy += kp_ * ey;
    gz += kp_ * ez;

    // Tích phân Quaternion
    float dq0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float dq1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float dq2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float dq3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    q0 += dq0 * dtSeconds;
    q1 += dq1 * dtSeconds;
    q2 += dq2 * dtSeconds;
    q3 += dq3 * dtSeconds;

    float qNorm = sqrtf(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    if (qNorm > 0.0001f) {
        attitude_.q0 = q0 / qNorm;
        attitude_.q1 = q1 / qNorm;
        attitude_.q2 = q2 / qNorm;
        attitude_.q3 = q3 / qNorm;
    }

    computeEulerAngles();
}

void AttitudeEstimator::computeEulerAngles() {
    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    // Roll (Trục X): atan2(2(q0*q1 + q2*q3), 1 - 2(q1^2 + q2^2))
    float sinr_cosp = 2.0f * (q0 * q1 + q2 * q3);
    float cosr_cosp = 1.0f - 2.0f * (q1 * q1 + q2 * q2);
    attitude_.rollRad = atan2f(sinr_cosp, cosr_cosp);
    attitude_.roll = attitude_.rollRad * RAD_TO_DEG_F;

    // Pitch (Trục Y): asin(2(q0*q2 - q3*q1)) - Giới hạn ±90° để tránh vượt miền giá trị asin
    float sinp = 2.0f * (q0 * q2 - q3 * q1);
    if (sinp >= 1.0f) {
        attitude_.pitchRad = (float)M_PI / 2.0f;
    } else if (sinp <= -1.0f) {
        attitude_.pitchRad = -(float)M_PI / 2.0f;
    } else {
        attitude_.pitchRad = asinf(sinp);
    }
    attitude_.pitch = attitude_.pitchRad * RAD_TO_DEG_F;

    // Yaw (Trục Z): atan2(2(q0*q3 + q1*q2), 1 - 2(q2^2 + q3^2))
    float siny_cosp = 2.0f * (q0 * q3 + q1 * q2);
    float cosy_cosp = 1.0f - 2.0f * (q2 * q2 + q3 * q3);
    attitude_.yawRad = atan2f(siny_cosp, cosy_cosp);
    attitude_.yaw = attitude_.yawRad * RAD_TO_DEG_F;

    // Chuẩn hóa góc Yaw về khoảng 0° - 360°
    if (attitude_.yaw < 0.0f) {
        attitude_.yaw += 360.0f;
    }
}
