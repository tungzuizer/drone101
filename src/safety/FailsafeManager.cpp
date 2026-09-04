#include "FailsafeManager.h"
#include <math.h>

FailsafeManager::FailsafeManager(MotorController& motorController)
    : motors_(motorController),
      state_(FS_OK),
      reason_(FS_REASON_NONE),
      maxTiltDeg_(MAX_TILT_ANGLE_DEG),
      signalTimeoutMs_(FAILSAFE_TIMEOUT_MS),
      armTimestampMs_(0),
      landingStartMs_(0) {
}

void FailsafeManager::begin(float maxTiltDeg, uint32_t signalTimeoutMs) {
    maxTiltDeg_ = maxTiltDeg;
    signalTimeoutMs_ = signalTimeoutMs;
    reset();
}

void FailsafeManager::reset() {
    state_ = FS_OK;
    reason_ = FS_REASON_NONE;
    landingStartMs_ = 0;
}

bool FailsafeManager::canArm(const ControlData& control, const AttitudeData& attitude, bool imuHealthy, String& reason) {
    if (!imuHealthy) {
        reason = "Cảm biến IMU MPU6050 chưa sẵn sàng hoặc lỗi I2C!";
        return false;
    }

    if (!motors_.isHealthy()) {
        reason = "Driver PWM động cơ chưa sẵn sàng!";
        return false;
    }

    if (!control.isConnected) {
        reason = "Chưa kết nối tín hiệu điều khiển (Serial GCS / RC)!";
        return false;
    }

    // BẮT BUỘC CẦN GA PHẢI Ở MỨC 0% KHI ARM
    if (control.throttle > 1.0f) {
        char buf[80];
        snprintf(buf, sizeof(buf), "Cần ga phải ở 0%% khi ARM! (Hiện tại: %.1f%%)", control.throttle);
        reason = buf;
        return false;
    }

    // Kiểm tra góc đặt trên mặt đất không bị nghiêng quá 30°
    if (fabs(attitude.roll) > 30.0f || fabs(attitude.pitch) > 30.0f) {
        char buf[100];
        snprintf(buf, sizeof(buf), "Drone nghiêng quá mức! Roll=%.1f° Pitch=%.1f° (Max ±30°)", attitude.roll, attitude.pitch);
        reason = buf;
        return false;
    }

    if (state_ == FS_EMERGENCY) {
        reason = "Hệ thống đang ở trạng thái Dừng Khẩn Cấp, vui lòng reset Failsafe!";
        return false;
    }

    armTimestampMs_ = millis();
    reason = "An toàn, cho phép ARM.";
    return true;
}

FailsafeState FailsafeManager::check(const ControlInputSource& input,
                                    const AttitudeData& attitude,
                                    bool imuHealthy) {
    // Nếu động cơ chưa ARM, chỉ kiểm tra Failsafe cơ bản
    if (!motors_.isArmed()) {
        if (state_ != FS_EMERGENCY) {
            state_ = FS_OK;
            reason_ = FS_REASON_NONE;
        }
        return state_;
    }

    // 1. KIỂM TRA LẬT NGHIÊNG QUÁ MỨC TRONG KHI BAY (> 55°)
    if (fabs(attitude.roll) > maxTiltDeg_ || fabs(attitude.pitch) > maxTiltDeg_) {
        state_ = FS_EMERGENCY;
        reason_ = FS_REASON_EXCESSIVE_TILT;
        motors_.emergencyStop();
        Serial.printf("[FAILSAFE CRITICAL] Drone bị lật nghiêng (Roll: %.1f°, Pitch: %.1f° > %.1f°)! NGẮT GA KHẨN CẤP!\n",
                      attitude.roll, attitude.pitch, maxTiltDeg_);
        return state_;
    }

    // 2. KIỂM TRA HỎNG CẢM BIẾN IMU
    if (!imuHealthy) {
        state_ = FS_EMERGENCY;
        reason_ = FS_REASON_SENSOR_FAILURE;
        motors_.emergencyStop();
        Serial.println("[FAILSAFE CRITICAL] Mất tín hiệu IMU MPU6050! NGẮT GA KHẨN CẤP!");
        return state_;
    }

    // 3. KIỂM TRA MẤT SÓNG ĐIỀU KHIỂN (SIGNAL TIMEOUT)
    if (input.isSignalLost(signalTimeoutMs_)) {
        if (state_ != FS_LANDING && state_ != FS_EMERGENCY) {
            state_ = FS_LANDING;
            reason_ = FS_REASON_SIGNAL_LOST;
            landingStartMs_ = millis();
            Serial.println("[FAILSAFE WARN] Mất tín hiệu điều khiển! Chuyển sang chế độ TỰ HẠ CÁNH...");
        }

        // Sau 3 giây tự hạ cánh nếu vẫn không có sóng -> Cắt ga hoàn toàn
        if (millis() - landingStartMs_ > 3000UL) {
            state_ = FS_EMERGENCY;
            motors_.emergencyStop();
            Serial.println("[FAILSAFE CRITICAL] Hết thời gian hạ cánh khẩn cấp! DỪNG ĐỘNG CƠ!");
        }
        return state_;
    }

    // 4. KIỂM TRA ARM NHƯNG KHÔNG CẤT CÁNH (ARM IDLE TIMEOUT 15s)
    const ControlData& ctrl = input.getControlData();
    if (ctrl.throttle <= 0.5f) {
        if (millis() - armTimestampMs_ > 15000UL) {
            motors_.disarm();
            reason_ = FS_REASON_ARM_TIMEOUT;
            Serial.println("[FAILSAFE INFO] Động cơ không ga quá 15 giây, tự động DISARM an toàn.");
            return FS_OK;
        }
    } else {
        armTimestampMs_ = millis(); // Cập nhật thời điểm khi có ga
    }

    // Nếu mọi thứ bình thường
    if (state_ == FS_LANDING || state_ == FS_WARNING) {
        state_ = FS_OK;
        reason_ = FS_REASON_NONE;
    }
    return state_;
}

void FailsafeManager::triggerEmergency(FailsafeReason reason) {
    state_ = FS_EMERGENCY;
    reason_ = reason;
    motors_.emergencyStop();
}

const char* FailsafeManager::getReasonString() const {
    switch (reason_) {
        case FS_REASON_NONE:            return "Bình thường";
        case FS_REASON_SIGNAL_LOST:     return "Mất tín hiệu điều khiển (Signal Timeout)";
        case FS_REASON_EXCESSIVE_TILT:  return "Lật nghiêng quá mức (> 55°)";
        case FS_REASON_SENSOR_FAILURE:  return "Lỗi phần cứng cảm biến IMU/I2C";
        case FS_REASON_ARM_TIMEOUT:     return "Tự ngắt do chờ ga quá 15s";
        case FS_REASON_MANUAL_KILL:     return "Nút dừng khẩn cấp thủ công";
        default:                        return "Không xác định";
    }
}
