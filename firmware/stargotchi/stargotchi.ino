// StarGotchi - a space-adventure virtual pet for the
// Waveshare ESP32-S3-Touch-AMOLED-1.75.
#include <lvgl.h>

#include "src/hal/display.h"
#include "src/hal/hal.h"
#include "src/ui/ui.h"

#ifndef SG_VERSION
#define SG_VERSION "dev"
#endif

void setup() {
  Serial.begin(115200);
  if (!sg::hal::displayBegin()) {
    Serial.println("display init failed");
    return;
  }
  sg::hal::i2cBegin();
  sg::hal::touchBegin();
  sg::hal::rtcBegin();
  sg::hal::imuBegin();
  sg::hal::powerBegin();
  sg::hal::audioBegin();
  sg::hal::beep(880, 120);
  static sg::Pet pet = sg::newPet(sg::hal::rtcNow());
  sg::ui::build([](sg::ui::Action, uint32_t) {}, {200, true});
  sg::ui::refresh(pet, sg::hal::rtcNow(), sg::hal::batteryPercent(), sg::hal::isCharging());
}

void loop() {
  lv_timer_handler();
  delay(5);
}
