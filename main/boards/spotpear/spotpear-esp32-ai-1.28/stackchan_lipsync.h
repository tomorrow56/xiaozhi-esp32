#pragma once

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

// Playback-only envelope. The audio task publishes; the LVGL task only reads.
// All time values are milliseconds; unsigned subtraction handles tick wrap.
class StackChanLipSync {
public:
    void Publish(const int16_t* data, int count, int volume, uint32_t now, uint32_t hold_ms = 100) {
        float level = 0;
        if (data && count > 0 && volume > 0) {
            double energy = 0;
            for (int i = 0; i < count; ++i) {
                const double sample = static_cast<double>(data[i]) / 32768.0;
                energy += sample * sample;
            }
            const float gain = std::clamp(volume, 0, 100) / 100.0f;
            const float rms = std::sqrt(energy / count) * gain * gain;
            level = std::sqrt(std::clamp((rms - 0.0025f) / 0.08f, 0.0f, 1.0f));
        }
        level_.store(level, std::memory_order_relaxed);
        hold_ms_.store(hold_ms, std::memory_order_relaxed);
        updated_.store(now, std::memory_order_release);
    }

    float Level(uint32_t now) const {
        const uint32_t updated = updated_.load(std::memory_order_acquire);
        if (now - updated > hold_ms_.load(std::memory_order_relaxed)) {
            return 0;
        }
        return level_.load(std::memory_order_relaxed);
    }

    void Reset() { level_.store(0, std::memory_order_relaxed); }

private:
    std::atomic<float> level_{0};
    std::atomic<uint32_t> updated_{0};
    std::atomic<uint32_t> hold_ms_{0};
};
