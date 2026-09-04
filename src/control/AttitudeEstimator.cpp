#include "AttitudeEstimator.h"
#include <math.h>

#define DEG_TO_RAD_F (0.017453292519943295f) // PI / 180
#define RAD_TO_DEG_F (57.29577951308232f)    // 180 / PI

AttitudeEstimator::AttitudeEstimator()
    : algorithm_(ESTIMATOR_MAHONY),
      kp_(2.5f),
      ki_(0.005f),
      beta_(0.08f),
      integralFBx_(0.0f),
      integralFBy_(0.0f),
      integralFBz_(0.0f),
      gForceRejection_(true) {
    reset();
}

void AttitudeEstimator::begin(float kp, float ki, float beta) {
    kp_ = kp;
    ki_ = ki;
    beta_ = beta;
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
        dtSeconds = 0.004f; // 250Hz default fallback
    }

    attitude_.rateRoll  = imu.gx;
    attitude_.ratePitch = imu.gy;
    attitude_.rateYaw   = imu.gz;

    float gx = imu.gx * DEG_TO_RAD_F;
    float gy = imu.gy * DEG_TO_RAD_F;
    float gz = imu.gz * DEG_TO_RAD_F;

    if (algorithm_ == ESTIMATOR_MADGWICK) {
        updateMadgwick6DOF(gx, gy, gz, imu.ax, imu.ay, imu.az, dtSeconds);
    } else {
        updateMahony6DOF(gx, gy, gz, imu.ax, imu.ay, imu.az, dtSeconds);
    }

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

    if (algorithm_ == ESTIMATOR_MADGWICK) {
        updateMadgwick9DOF(gx, gy, gz, imu.ax, imu.ay, imu.az, mag.mx, mag.my, mag.mz, dtSeconds);
    } else {
        updateMahony9DOF(gx, gy, gz, imu.ax, imu.ay, imu.az, mag.mx, mag.my, mag.mz, dtSeconds);
    }

    computeEulerAngles();
}

// =============================================================================
// THUẬT TOÁN MAHONY 6-DOF VỚI G-FORCE REJECTION & FAST INVSQRT
// =============================================================================
void AttitudeEstimator::updateMahony6DOF(float gx, float gy, float gz, float ax, float ay, float az, float dt) {
    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    float accSq = ax * ax + ay * ay + az * az;
    if (accSq > 0.001f) {
        float recipAcc = invSqrt(accSq);
        ax *= recipAcc;
        ay *= recipAcc;
        az *= recipAcc;

        // Ước lượng hướng vector trọng lực từ Quaternion hiện tại: v = [2(q1q3 - q0q2), 2(q0q1 + q2q3), q0^2 - q1^2 - q2^2 + q3^2]
        float vx = 2.0f * (q1 * q3 - q0 * q2);
        float vy = 2.0f * (q0 * q1 + q2 * q3);
        float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        // Sai số tích có hướng (Cross Product) giữa gia tốc đo và gia tốc ước lượng
        float ex = (ay * vz - az * vy);
        float ey = (az * vx - ax * vz);
        float ez = (ax * vy - ay * vx);

        // G-Force Rejection: Giảm trọng số Accel khi Drone chịu gia tốc ly tâm/tăng ga đột ngột (khác 1.0g)
        float weight = 1.0f;
        if (gForceRejection_) {
            float gMag = sqrtf(accSq);
            float gDiff = fabsf(gMag - 1.0f);
            if (gDiff > 0.3f) {
                weight = 1.0f - (gDiff - 0.3f) * 2.0f;
                if (weight < 0.0f) weight = 0.0f;
            }
        }

        if (ki_ > 0.0f && weight > 0.1f) {
            integralFBx_ += ex * ki_ * dt * weight;
            integralFBy_ += ey * ki_ * dt * weight;
            integralFBz_ += ez * ki_ * dt * weight;
            gx += integralFBx_;
            gy += integralFBy_;
            gz += integralFBz_;
        }

        gx += kp_ * ex * weight;
        gy += kp_ * ey * weight;
        gz += kp_ * ez * weight;
    }

    // Tích phân Quaternion: dq/dt = 0.5 * q * w
    float dq0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float dq1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float dq2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float dq3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    q0 += dq0 * dt;
    q1 += dq1 * dt;
    q2 += dq2 * dt;
    q3 += dq3 * dt;

    float recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    attitude_.q0 = q0 * recipNorm;
    attitude_.q1 = q1 * recipNorm;
    attitude_.q2 = q2 * recipNorm;
    attitude_.q3 = q3 * recipNorm;
}

// =============================================================================
// THUẬT TOÁN MAHONY 9-DOF FUSION TỪ TRƯỜNG LA BÀN
// =============================================================================
void AttitudeEstimator::updateMahony9DOF(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt) {
    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    float ex = 0.0f, ey = 0.0f, ez = 0.0f;

    // 1. Gia tốc kế
    float accSq = ax * ax + ay * ay + az * az;
    if (accSq > 0.001f) {
        float recipAcc = invSqrt(accSq);
        ax *= recipAcc;
        ay *= recipAcc;
        az *= recipAcc;

        float vx = 2.0f * (q1 * q3 - q0 * q2);
        float vy = 2.0f * (q0 * q1 + q2 * q3);
        float vz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

        ex += (ay * vz - az * vy);
        ey += (az * vx - ax * vz);
        ez += (ax * vy - ay * vx);
    }

    // 2. Từ kế
    float magSq = mx * mx + my * my + mz * mz;
    if (magSq > 0.001f) {
        float recipMag = invSqrt(magSq);
        mx *= recipMag;
        my *= recipMag;
        mz *= recipMag;

        // Chuyển vector từ trường từ Body sang Earth: h = R * m
        float hx = 2.0f * mx * (0.5f - q2 * q2 - q3 * q3) + 2.0f * my * (q1 * q2 - q0 * q3) + 2.0f * mz * (q1 * q3 + q0 * q2);
        float hy = 2.0f * mx * (q1 * q2 + q0 * q3) + 2.0f * my * (0.5f - q1 * q1 - q3 * q3) + 2.0f * mz * (q2 * q3 - q0 * q1);
        float hz = 2.0f * mx * (q1 * q3 - q0 * q2) + 2.0f * my * (q2 * q3 + q0 * q1) + 2.0f * mz * (0.5f - q1 * q1 - q2 * q2);
        float bx = sqrtf(hx * hx + hy * hy);
        float bz = hz;

        // Chuyển lại vector tham chiếu chuẩn bx, bz từ Earth sang Body
        float wx = 2.0f * bx * (0.5f - q2 * q2 - q3 * q3) + 2.0f * bz * (q1 * q3 - q0 * q2);
        float wy = 2.0f * bx * (q1 * q2 - q0 * q3) + 2.0f * bz * (q2 * q3 + q0 * q1);
        float wz = 2.0f * bx * (q1 * q3 + q0 * q2) + 2.0f * bz * (0.5f - q1 * q1 - q2 * q2);

        // Sai số từ trường
        ex += (my * wz - mz * wy);
        ey += (mz * wx - mx * wz);
        ez += (mx * wy - my * wx);
    }

    if (ki_ > 0.0f) {
        integralFBx_ += ex * ki_ * dt;
        integralFBy_ += ey * ki_ * dt;
        integralFBz_ += ez * ki_ * dt;
        gx += integralFBx_;
        gy += integralFBy_;
        gz += integralFBz_;
    }

    gx += kp_ * ex;
    gy += kp_ * ey;
    gz += kp_ * ez;

    float dq0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float dq1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float dq2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float dq3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    q0 += dq0 * dt;
    q1 += dq1 * dt;
    q2 += dq2 * dt;
    q3 += dq3 * dt;

    float recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    attitude_.q0 = q0 * recipNorm;
    attitude_.q1 = q1 * recipNorm;
    attitude_.q2 = q2 * recipNorm;
    attitude_.q3 = q3 * recipNorm;
}

// =============================================================================
// THUẬT TOÁN MADGWICK GRADIENT DESCENT 6-DOF
// =============================================================================
void AttitudeEstimator::updateMadgwick6DOF(float gx, float gy, float gz, float ax, float ay, float az, float dt) {
    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    // Đạo hàm Quaternion từ Gyroscope (dq/dt = 0.5 * q * w)
    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    float accSq = ax * ax + ay * ay + az * az;
    if (accSq > 0.001f) {
        float recipAcc = invSqrt(accSq);
        ax *= recipAcc;
        ay *= recipAcc;
        az *= recipAcc;

        // Các biến phụ tối ưu tính toán Gradient Descent
        float _2q0 = 2.0f * q0;
        float _2q1 = 2.0f * q1;
        float _2q2 = 2.0f * q2;
        float _2q3 = 2.0f * q3;
        float _4q0 = 4.0f * q0;
        float _4q1 = 4.0f * q1;
        float _4q2 = 4.0f * q2;
        float _8q1 = 8.0f * q1;
        float _8q2 = 8.0f * q2;
        float q0q0 = q0 * q0;
        float q1q1 = q1 * q1;
        float q2q2 = q2 * q2;
        float q3q3 = q3 * q3;

        // Gradient của hàm mục tiêu f_g: J^T * f
        float s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        float s1 = _8q1 * q1q1 + _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q2q2 + _4q1 * az;
        float s2 = _4q2 * q0q0 - _2q0 * ax + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * q3q3 - _2q3 * ay - _4q2 + _4q2 * az;
        float s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;

        float recipGrad = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        s0 *= recipGrad;
        s1 *= recipGrad;
        s2 *= recipGrad;
        s3 *= recipGrad;

        // Áp dụng hệ số bù Madgwick beta
        qDot0 -= beta_ * s0;
        qDot1 -= beta_ * s1;
        qDot2 -= beta_ * s2;
        qDot3 -= beta_ * s3;
    }

    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    float recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    attitude_.q0 = q0 * recipNorm;
    attitude_.q1 = q1 * recipNorm;
    attitude_.q2 = q2 * recipNorm;
    attitude_.q3 = q3 * recipNorm;
}

// =============================================================================
// THUẬT TOÁN MADGWICK GRADIENT DESCENT 9-DOF (ACCEL + GYRO + MAG)
// =============================================================================
void AttitudeEstimator::updateMadgwick9DOF(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz, float dt) {
    float q0 = attitude_.q0;
    float q1 = attitude_.q1;
    float q2 = attitude_.q2;
    float q3 = attitude_.q3;

    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
    float qDot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy);
    float qDot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx);
    float qDot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx);

    float accSq = ax * ax + ay * ay + az * az;
    float magSq = mx * mx + my * my + mz * mz;

    if (accSq > 0.001f && magSq > 0.001f) {
        float recipAcc = invSqrt(accSq);
        ax *= recipAcc;
        ay *= recipAcc;
        az *= recipAcc;

        float recipMag = invSqrt(magSq);
        mx *= recipMag;
        my *= recipMag;
        mz *= recipMag;

        // Tính vector từ trường tham chiếu
        float hx = mx * (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) + 2.0f * my * (q1 * q2 - q0 * q3) + 2.0f * mz * (q1 * q3 + q0 * q2);
        float hy = 2.0f * mx * (q1 * q2 + q0 * q3) + my * (q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3) + 2.0f * mz * (q2 * q3 - q0 * q1);
        float _2bx = sqrtf(hx * hx + hy * hy);
        float _2bz = 2.0f * (2.0f * mx * (q1 * q3 - q0 * q2) + 2.0f * my * (q2 * q3 + q0 * q1) + mz * (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3));
        float _4bx = 2.0f * _2bx;
        float _4bz = 2.0f * _2bz;

        // Gradient kết hợp Accel và Mag
        float _2q0 = 2.0f * q0;
        float _2q1 = 2.0f * q1;
        float _2q2 = 2.0f * q2;
        float _2q3 = 2.0f * q3;
        float q0q0 = q0 * q0;
        float q1q1 = q1 * q1;
        float q2q2 = q2 * q2;
        float q3q3 = q3 * q3;

        float s0 = -_2q2 * (2.0f * (q1 * q3 - q0 * q2) - ax) + _2q1 * (2.0f * (q0 * q1 + q2 * q3) - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1 * q3 - q0 * q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1 * q2 - q0 * q3) + _2bz * (q0 * q1 + q2 * q3) - my) + _2bx * q2 * (_2bx * (q0 * q2 + q1 * q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        float s1 = _2q3 * (2.0f * (q1 * q3 - q0 * q2) - ax) + _2q0 * (2.0f * (q0 * q1 + q2 * q3) - ay) - 4.0f * q1 * (1.0f - 2.0f * (q1q1 + q2q2) - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1 * q3 - q0 * q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1 * q2 - q0 * q3) + _2bz * (q0 * q1 + q2 * q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0 * q2 + q1 * q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        float s2 = -_2q0 * (2.0f * (q1 * q3 - q0 * q2) - ax) + _2q3 * (2.0f * (q0 * q1 + q2 * q3) - ay) - 4.0f * q2 * (1.0f - 2.0f * (q1q1 + q2q2) - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1 * q3 - q0 * q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1 * q2 - q0 * q3) + _2bz * (q0 * q1 + q2 * q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0 * q2 + q1 * q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        float s3 = _2q1 * (2.0f * (q1 * q3 - q0 * q2) - ax) + _2q2 * (2.0f * (q0 * q1 + q2 * q3) - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1 * q3 - q0 * q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1 * q2 - q0 * q3) + _2bz * (q0 * q1 + q2 * q3) - my) + _2bx * q1 * (_2bx * (q0 * q2 + q1 * q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);

        float recipGrad = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        s0 *= recipGrad;
        s1 *= recipGrad;
        s2 *= recipGrad;
        s3 *= recipGrad;

        qDot0 -= beta_ * s0;
        qDot1 -= beta_ * s1;
        qDot2 -= beta_ * s2;
        qDot3 -= beta_ * s3;
    }

    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;

    float recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    attitude_.q0 = q0 * recipNorm;
    attitude_.q1 = q1 * recipNorm;
    attitude_.q2 = q2 * recipNorm;
    attitude_.q3 = q3 * recipNorm;
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

    // Pitch (Trục Y): asin(2(q0*q2 - q3*q1)) - Giới hạn ±90° để tránh singularity
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
