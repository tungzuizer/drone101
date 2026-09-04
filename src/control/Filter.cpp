#include "Filter.h"

// =============================================================================
// PT1 FILTER IMPLEMENTATION
// =============================================================================
Pt1Filter::Pt1Filter() : state_(0.0f), k_(1.0f) {}

Pt1Filter::Pt1Filter(float cutoffFreqHz, float sampleRateHz) : state_(0.0f) {
    init(cutoffFreqHz, sampleRateHz);
}

void Pt1Filter::init(float cutoffFreqHz, float sampleRateHz) {
    setCutoff(cutoffFreqHz, sampleRateHz);
    reset(0.0f);
}

void Pt1Filter::setCutoff(float cutoffFreqHz, float sampleRateHz) {
    if (sampleRateHz <= 0.0f || cutoffFreqHz <= 0.0f) {
        k_ = 1.0f;
        return;
    }
    float dt = 1.0f / sampleRateHz;
    float rc = 1.0f / (2.0f * (float)M_PI * cutoffFreqHz);
    k_ = dt / (rc + dt);
    if (k_ > 1.0f) k_ = 1.0f;
    if (k_ < 0.0f) k_ = 0.0f;
}

float Pt1Filter::update(float input) {
    state_ += k_ * (input - state_);
    return state_;
}

void Pt1Filter::reset(float value) {
    state_ = value;
}

// =============================================================================
// BIQUAD FILTER IMPLEMENTATION (Direct Form II Transposed)
// =============================================================================
BiquadFilter::BiquadFilter()
    : type_(BIQUAD_LPF_2ND),
      b0_(1.0f), b1_(0.0f), b2_(0.0f),
      a1_(0.0f), a2_(0.0f),
      d1_(0.0f), d2_(0.0f),
      output_(0.0f) {
}

void BiquadFilter::initLpf(float cutoffFreqHz, float sampleRateHz, float q) {
    type_ = BIQUAD_LPF_2ND;
    if (sampleRateHz <= 0.0f || cutoffFreqHz <= 0.0f) {
        b0_ = 1.0f; b1_ = 0.0f; b2_ = 0.0f;
        a1_ = 0.0f; a2_ = 0.0f;
        reset(0.0f);
        return;
    }

    // Giới hạn tần số cắt nhỏ hơn tần số Nyquist (fs / 2)
    if (cutoffFreqHz > sampleRateHz * 0.49f) {
        cutoffFreqHz = sampleRateHz * 0.49f;
    }

    float omega = 2.0f * (float)M_PI * cutoffFreqHz / sampleRateHz;
    float sinOmega = sinf(omega);
    float cosOmega = cosf(omega);
    float alpha = sinOmega / (2.0f * q);

    float a0 = 1.0f + alpha;
    b0_ = ((1.0f - cosOmega) / 2.0f) / a0;
    b1_ = (1.0f - cosOmega) / a0;
    b2_ = ((1.0f - cosOmega) / 2.0f) / a0;
    a1_ = (-2.0f * cosOmega) / a0;
    a2_ = (1.0f - alpha) / a0;

    reset(0.0f);
}

void BiquadFilter::initNotch(float centerFreqHz, float sampleRateHz, float cutoffFreqHz) {
    type_ = BIQUAD_NOTCH;
    if (sampleRateHz <= 0.0f || centerFreqHz <= 0.0f) {
        b0_ = 1.0f; b1_ = 0.0f; b2_ = 0.0f;
        a1_ = 0.0f; a2_ = 0.0f;
        reset(0.0f);
        return;
    }

    if (centerFreqHz > sampleRateHz * 0.49f) {
        centerFreqHz = sampleRateHz * 0.49f;
    }

    float q = 3.0f; // Q factor mặc định cho notch filter
    if (cutoffFreqHz > 0.0f && cutoffFreqHz < centerFreqHz) {
        float bandwidth = (centerFreqHz - cutoffFreqHz) * 2.0f;
        if (bandwidth > 0.0f) {
            q = centerFreqHz / bandwidth;
        }
    }

    float omega = 2.0f * (float)M_PI * centerFreqHz / sampleRateHz;
    float sinOmega = sinf(omega);
    float cosOmega = cosf(omega);
    float alpha = sinOmega / (2.0f * q);

    float a0 = 1.0f + alpha;
    b0_ = 1.0f / a0;
    b1_ = (-2.0f * cosOmega) / a0;
    b2_ = 1.0f / a0;
    a1_ = (-2.0f * cosOmega) / a0;
    a2_ = (1.0f - alpha) / a0;

    reset(0.0f);
}

float BiquadFilter::update(float input) {
    output_ = b0_ * input + d1_;
    d1_ = b1_ * input - a1_ * output_ + d2_;
    d2_ = b2_ * input - a2_ * output_;
    return output_;
}

void BiquadFilter::reset(float value) {
    output_ = value;
    if (fabsf(b0_ + b1_ + b2_) > 0.0001f) {
        d1_ = value * (1.0f - b0_ + a1_);
        d2_ = value * (b2_ - a2_);
    } else {
        d1_ = 0.0f;
        d2_ = 0.0f;
    }
}
