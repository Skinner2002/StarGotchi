// Speaker output: ES8311 codec over I2S, simple square-wave tones played
// from a background task so the UI never blocks.
#include <Arduino.h>
#include <ESP_I2S.h>
#include <Wire.h>

#include "es8311/es8311.h"
#include "hal.h"
#include "pins.h"

namespace sg {
namespace hal {

namespace {

constexpr uint32_t kRate = 16000;
constexpr int16_t kAmp = 3000;

struct Tone { uint16_t hz, ms; };

I2SClass i2s;
QueueHandle_t queue = nullptr;
volatile bool soundOn = true;
es8311_handle_t codec = nullptr;
uint8_t volumePct = 70;

bool codecInit() {
  es8311_handle_t h = es8311_create(0, ES8311_ADDRRES_0);
  if (!h) return false;
  codec = h;
  const es8311_clock_config_t clk = {
      .mclk_inverted = false, .sclk_inverted = false, .mclk_from_mclk_pin = true,
      .mclk_frequency = kRate * 256, .sample_frequency = kRate};
  if (es8311_init(h, &clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) != ESP_OK) return false;
  es8311_sample_frequency_config(h, clk.mclk_frequency, clk.sample_frequency);
  es8311_microphone_config(h, false);
  es8311_voice_volume_set(h, volumePct, nullptr);
  return true;
}

void playTone(const Tone &t) {
  static int16_t buf[256 * 2];
  uint32_t total = kRate * t.ms / 1000;
  const uint32_t half = t.hz ? kRate / (2 * t.hz) : 0;
  uint32_t phase = 0;
  while (total > 0) {
    const uint32_t n = total < 256 ? total : 256;
    for (uint32_t i = 0; i < n; i++) {
      int16_t s = 0;
      if (half) {
        s = (phase / half) % 2 ? kAmp : -kAmp;
        // short fade in/out against clicks
        const uint32_t pos = phase, left = total - i;
        if (pos < 80) s = s * (int32_t)pos / 80;
        if (left < 80) s = s * (int32_t)left / 80;
      }
      buf[2 * i] = buf[2 * i + 1] = s;
      phase++;
    }
    i2s.write((uint8_t *)buf, n * 4);
    total -= n;
  }
}

void audioTask(void *) {
  Tone t;
  for (;;) {
    if (xQueueReceive(queue, &t, portMAX_DELAY) == pdTRUE) {
      if (soundOn) playTone(t);
    }
  }
}

}  // namespace

bool audioBegin() {
  pinMode(PIN_PA_CTRL, OUTPUT);
  digitalWrite(PIN_PA_CTRL, HIGH);
  i2s.setPins(PIN_I2S_BCLK, PIN_I2S_WS, PIN_I2S_DOUT, PIN_I2S_DIN, PIN_I2S_MCLK);
  if (!i2s.begin(I2S_MODE_STD, kRate, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO,
                 I2S_STD_SLOT_BOTH))
    return false;
  if (!codecInit()) return false;
  queue = xQueueCreate(16, sizeof(Tone));
  if (!queue) return false;
  xTaskCreatePinnedToCore(audioTask, "sg_audio", 4096, nullptr, 2, nullptr, 0);
  return true;
}

void beep(uint16_t hz, uint16_t ms) {
  if (!queue || !soundOn) return;
  Tone t{hz, ms};
  xQueueSend(queue, &t, 0);
}

void setVolume(uint8_t percent) {
  if (percent > 100) percent = 100;
  soundOn = percent > 0;
  volumePct = percent;
  // Codec volume is roughly logarithmic; keep low settings audible.
  if (codec && percent > 0) es8311_voice_volume_set(codec, 40 + percent * 55 / 100, nullptr);
}

}  // namespace hal
}  // namespace sg
