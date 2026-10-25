#pragma once

#include "codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "stackchan_lipsync.h"

#if CONFIG_SPOTPEAR_STACKCHAN_FACE
StackChanLipSync& GetStackChanLipSync();

class StackChanAudioCodec : public NoAudioCodecSimplex {
public:
    using NoAudioCodecSimplex::NoAudioCodecSimplex;
    void EnableOutput(bool enable) override;

protected:
    int Write(const int16_t* data, int samples) override;
};

class StackChanDisplay : public SpiLcdDisplay {
public:
    using SpiLcdDisplay::SpiLcdDisplay;
    ~StackChanDisplay() override;
    void SetupUI() override;
    bool SetEmotion(const char* emotion) override;

private:
    void Render();
    lv_obj_t* face_ = nullptr;
    lv_obj_t* eyes_[2] = {nullptr, nullptr};
    lv_obj_t* mouth_ = nullptr;
    lv_timer_t* timer_ = nullptr;
    std::atomic<int> expression_{0};
    float mouth_level_ = 0;
};
#endif
