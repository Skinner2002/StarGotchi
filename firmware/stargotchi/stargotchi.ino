// StarGotchi - a space-adventure virtual pet for the
// Waveshare ESP32-S3-Touch-AMOLED-1.75.
#include <lvgl.h>

#include "src/hal/display.h"
#include "src/hal/hal.h"

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
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "StarGotchi");
  lv_obj_set_style_text_font(title, &lv_font_unscii_16, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x85B7EB), 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, -12);

  lv_obj_t *ver = lv_label_create(scr);
  lv_label_set_text(ver, "boot ok v" SG_VERSION);
  lv_obj_set_style_text_font(ver, &lv_font_unscii_8, 0);
  lv_obj_set_style_text_color(ver, lv_color_hex(0xB4B2A9), 0);
  lv_obj_align(ver, LV_ALIGN_CENTER, 0, 14);
}

void loop() {
  lv_timer_handler();
  delay(5);
}
