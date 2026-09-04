#ifndef FILTER_H
#define FILTER_H

#include <Arduino.h>
#include <math.h>

// =============================================================================
// BỘ LỌC THÔNG THẤP BẬC 1 (PT1 / 1st-ORDER LOW-PASS FILTER)
// =============================================================================
class Pt1Filter {
public:
    Pt1Filter();
    Pt1Filter(float cutoffFreqHz, float sampleRateHz);

    void init(float cutoffFreqHz, float sampleRateHz);
    void setCutoff(float cutoffFreqHz, float sampleRateHz);
    float update(float input);
    void reset(float value = 0.0f);
    float getValue() const { return state_; }

private:
    float state_;
    float k_; // alpha = dt / (RC + dt)
};

// =============================================================================
// BỘ LỌC SỐ BIQUAD BẬC 2 (2nd-ORDER BIQUAD FILTER: LPF & NOTCH)
// Chuẩn thuật toán âm thanh & điều khiển bay Betaflight / INAV
// =============================================================================
enum BiquadFilterType {
    BIQUAD_LPF_2ND,     // Butterworth 2nd-order Low Pass Filter (-12dB/octave)
    BIQUAD_NOTCH        // Notch Filter triệt tiêu rung động tần số cộng hưởng
};

class BiquadFilter {
public:
    BiquadFilter();

    // Khởi tạo Biquad Low Pass Filter bậc 2
    void initLpf(float cutoffFreqHz, float sampleRateHz, float q = 0.7071f);

    // Khởi tạo Biquad Notch Filter (Triệt tiêu rung tần số trung tâm)
    void initNotch(float centerFreqHz, float sampleRateHz, float cutoffFreqHz = 0.0f);

    // Xử lý mẫu tín hiệu (Direct Form II Transposed - Ổn định và tối ưu bộ nhớ)
    float update(float input);

    // Reset trạng thái bộ lọc
    void reset(float value = 0.0f);

    float getValue() const { return output_; }

private:
    BiquadFilterType type_;
    float b0_, b1_, b2_; // Hệ số tử số
    float a1_, a2_;     // Hệ số mẫu số (a0 = 1.0)
    float d1_, d2_;     // Bộ trễ trạng thái bộ lọc (Delay registers)
    float output_;
};

#endif // FILTER_H
