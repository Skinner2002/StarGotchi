#include <Arduino.h>
#include <TouchDrvCSTXXX.hpp>
#include <Wire.h>
#include <lvgl.h>

#include "hal.h"
#include "pins.h"

namespace sg {
namespace hal {

namespace {

TouchDrvCST92xx touch;
lv_indev_drv_t indevDrv;
bool (*gateFn)(uint32_t) = nullptr;
volatile bool intFlag = false;
bool pressed = false;    // debounced state reported to LVGL
bool swallowing = false;
uint32_t lastPointMs = 0;
int16_t lastX = 0, lastY = 0;

// A press only ends after this long without any touch report, so short gaps
// between controller reports don't look like release + new press.
constexpr uint32_t kReleaseMs = 80;

void IRAM_ATTR onTouchInt() { intFlag = true; }

void readCb(lv_indev_drv_t *, lv_indev_data_t *data) {
  const uint32_t now = millis();
  // Only talk to the controller when it signalled new data or a press is
  // ongoing (keeps the I2C bus quiet while idle).
  if (intFlag || pressed) {
    intFlag = false;
    int16_t x[5], y[5];
    if (touch.getPoint(x, y, 1) > 0) {
      if (!pressed) swallowing = gateFn && gateFn(now);
      pressed = true;
      lastPointMs = now;
      lastX = x[0];
      lastY = y[0];
    }
  }
  if (pressed && now - lastPointMs > kReleaseMs) {
    pressed = false;
    swallowing = false;
  }
  data->point.x = lastX;
  data->point.y = lastY;
  data->state = (pressed && !swallowing) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

}  // namespace

bool touchBegin() {
  pinMode(PIN_TP_RESET, OUTPUT);
  digitalWrite(PIN_TP_RESET, LOW);
  delay(30);
  digitalWrite(PIN_TP_RESET, HIGH);
  delay(50);
  touch.setPins(PIN_TP_RESET, PIN_TP_INT);
  if (!touch.begin(Wire, TOUCH_I2C_ADDR, PIN_I2C_SDA, PIN_I2C_SCL)) return false;
  touch.setMaxCoordinates(LCD_WIDTH, LCD_HEIGHT);
  touch.setMirrorXY(true, true);

  lv_indev_drv_init(&indevDrv);
  indevDrv.type = LV_INDEV_TYPE_POINTER;
  indevDrv.read_cb = readCb;
  lv_indev_drv_register(&indevDrv);
  pinMode(PIN_TP_INT, INPUT_PULLUP);
  attachInterrupt(PIN_TP_INT, onTouchInt, FALLING);
  return true;
}

void touchSetGate(bool (*gate)(uint32_t ms)) { gateFn = gate; }

}  // namespace hal
}  // namespace sg
