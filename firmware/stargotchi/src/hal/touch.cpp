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
bool wasPressed = false;
bool swallowing = false;
int16_t lastX = 0, lastY = 0;

void readCb(lv_indev_drv_t *, lv_indev_data_t *data) {
  int16_t x[5], y[5];
  const uint8_t n = touch.getPoint(x, y, 1);
  const bool pressed = n > 0;
  if (pressed && !wasPressed) swallowing = gateFn && gateFn(millis());
  if (!pressed) swallowing = false;
  wasPressed = pressed;
  if (pressed) {
    lastX = x[0];
    lastY = y[0];
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
  if (!touch.begin(Wire, TOUCH_I2C_ADDR, PIN_I2C_SDA, PIN_I2C_SCL)) return false;
  touch.setMaxCoordinates(LCD_WIDTH, LCD_HEIGHT);
  touch.setMirrorXY(true, true);

  lv_indev_drv_init(&indevDrv);
  indevDrv.type = LV_INDEV_TYPE_POINTER;
  indevDrv.read_cb = readCb;
  lv_indev_drv_register(&indevDrv);
  return true;
}

void touchSetGate(bool (*gate)(uint32_t ms)) { gateFn = gate; }

}  // namespace hal
}  // namespace sg
