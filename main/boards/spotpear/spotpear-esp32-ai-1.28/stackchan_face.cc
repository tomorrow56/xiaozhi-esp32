// SPDX-License-Identifier: MIT
// Face proportions and expression approach adapted from M5Stack StackChan's
// default avatar (Copyright (c) 2026 M5Stack Technology CO LTD).
// See STACKCHAN-LICENSE.txt. No M5Stack hardware/BSP dependency is used.
#include "stackchan_face.h"

#if CONFIG_SPOTPEAR_STACKCHAN_FACE
#include <esp_timer.h>
#include <cstring>

namespace {
uint32_t NowMs() { return static_cast<uint32_t>(esp_timer_get_time() / 1000); }

lv_obj_t* MakeFeature(lv_obj_t* parent) {
    auto obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}
}  // namespace

StackChanLipSync& GetStackChanLipSync() {
    static StackChanLipSync state;
    return state;
}

int StackChanAudioCodec::Write(const int16_t* data, int samples) {
    // Preserve the existing I2S format/volume conversion. Update the envelope
    // per 20ms PCM slice instead of jumping once per decoded Opus frame.
    const int frame = std::max(1, output_sample_rate_ / 50);
    int written = 0;
    while (written < samples) {
        const int count = std::min(frame, samples - written);
        const uint32_t hold = 60 + (count * 1000 / output_sample_rate_);
        GetStackChanLipSync().Publish(data + written, count, output_volume_, NowMs(), hold);
        const int result = NoAudioCodec::Write(data + written, count);
        if (result <= 0) {
            GetStackChanLipSync().Reset();
            break;
        }
        written += result;
    }
    return written;
}

void StackChanAudioCodec::EnableOutput(bool enable) {
    NoAudioCodec::EnableOutput(enable);
    if (!enable) {
        GetStackChanLipSync().Reset();
    }
}

StackChanDisplay::~StackChanDisplay() {
    DisplayLockGuard lock(this);
    if (timer_) {
        lv_timer_delete(timer_);
    }
    if (face_) {
        lv_obj_delete(face_);
    }
}

void StackChanDisplay::SetupUI() {
    if (IsSetupUICalled()) {
        return;
    }
    SpiLcdDisplay::SetupUI();
    DisplayLockGuard lock(this);
    if (!lock) {
        return;
    }
    // Retain the stock status, provisioning, notifications, subtitles and preview.
    // Only replace the emoji region; all original widgets stay alive.
    if (emoji_box_) {
        lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
    }
    face_ = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(face_);
    lv_obj_set_size(face_, 180, 150);
    lv_obj_align(face_, LV_ALIGN_CENTER, 0, -1);
    lv_obj_remove_flag(face_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_to_index(face_, 1);  // Above the backdrop, below status/preview/text.
    eyes_[0] = MakeFeature(face_);
    eyes_[1] = MakeFeature(face_);
    mouth_ = MakeFeature(face_);
    timer_ = lv_timer_create(
        [](lv_timer_t* timer) {
            static_cast<StackChanDisplay*>(lv_timer_get_user_data(timer))->Render();
        },
        40, this);
    Render();
    ESP_LOGI("StackChan", "Vector face ready; playback PCM lip sync at 25 FPS");
}

bool StackChanDisplay::SetEmotion(const char* emotion) {
    if (!emotion || !*emotion || !face_) {
        return false;
    }
    int value = 0;
    if (!std::strcmp(emotion, "happy") || !std::strcmp(emotion, "laughing") ||
        !std::strcmp(emotion, "excited") || !std::strcmp(emotion, "loving")) {
        value = 1;
    } else if (!std::strcmp(emotion, "sad") || !std::strcmp(emotion, "crying")) {
        value = 2;
    } else if (!std::strcmp(emotion, "angry")) {
        value = 3;
    } else if (!std::strcmp(emotion, "sleepy") || !std::strcmp(emotion, "sleeping")) {
        value = 4;
    } else if (!std::strcmp(emotion, "surprised") || !std::strcmp(emotion, "shocked")) {
        value = 5;
    } else if (!std::strcmp(emotion, "thinking") || !std::strcmp(emotion, "confused")) {
        value = 6;
    }
    expression_.store(value, std::memory_order_relaxed);
    return true;
}

void StackChanDisplay::Render() {
    // Called exclusively under the LVGL task/lock; never from the audio task.
    // The base preview timeout restores the emoji widget. Keep it replaced,
    // and hide our face while a preview image occupies the same region.
    lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
    if (preview_image_ && !lv_obj_has_flag(preview_image_, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(face_, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_obj_remove_flag(face_, LV_OBJ_FLAG_HIDDEN);
    const uint32_t now = NowMs();
    const int emotion = expression_.load(std::memory_order_relaxed);
    const bool blink = now % 4300 < 140;
    const auto color = lv_obj_get_style_text_color(lv_screen_active(), LV_PART_MAIN);
    const int gaze = emotion == 6 ? 4 : static_cast<int>(std::sin(now / 1600.0) * 2);
    for (int i = 0; i < 2; ++i) {
        // Keep happy/speaking eyes round; only blink and sleepy close the eyelids.
        const int h = blink ? 3 : emotion == 4 ? 5 : emotion == 5 ? 27 : 20;
        lv_obj_set_size(eyes_[i], 20, h);
        lv_obj_align(eyes_[i], LV_ALIGN_CENTER, (i ? 49 : -49) + gaze, -16);
        lv_obj_set_style_radius(eyes_[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(eyes_[i], color, 0);
        const int rotation = emotion == 3 ? (i ? -250 : 250) : emotion == 2 ? (i ? 200 : -200) : 0;
        lv_obj_set_style_transform_rotation(eyes_[i], rotation, 0);
    }
    const float target = GetStackChanLipSync().Level(now);
    const float smoothing = target > mouth_level_ ? 0.8f : 0.45f;
    mouth_level_ += (target - mouth_level_) * smoothing;
    if (target == 0 && mouth_level_ < 0.015f) {
        mouth_level_ = 0;
    }
    const int open = static_cast<int>(mouth_level_ * 100);
    // Scaled from StackChan's 90x6 -> 60x50 mouth proportions for a round LCD.
    const int w = emotion == 5 && open == 0 ? 22 : 64 - open * 21 / 100;
    const int h = emotion == 5 && open == 0 ? 24 : 4 + open * 32 / 100;
    lv_obj_set_size(mouth_, w, h);
    lv_obj_align(mouth_, LV_ALIGN_CENTER, gaze, 30);
    lv_obj_set_style_radius(mouth_, open > 0 || emotion == 5 ? 11 : 2, 0);
    lv_obj_set_style_bg_color(mouth_, color, 0);
}
#endif
