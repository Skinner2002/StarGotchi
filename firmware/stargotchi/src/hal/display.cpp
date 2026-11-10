// Display + LVGL bring-up, ported from Waveshare's 06_LVGL_Widgets example
// for the ESP32-S3-Touch-AMOLED-1.75.
#include "display.h"
#include "pins.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <lvgl.h>

namespace sg {
namespace hal {

namespace {

constexpr uint32_t kTickMs = 2;
constexpr size_t kBufLines = 48;

Arduino_DataBus *bus = nullptr;
Arduino_CO5300 *gfx = nullptr;
lv_disp_draw_buf_t drawBuf;
lv_disp_drv_t dispDrv;

// The CO5300 needs flush areas that start on even and end on odd pixels.
void rounderCb(lv_disp_drv_t *, lv_area_t *area) {
  if (area->x1 % 2 != 0) area->x1--;
  if (area->y1 % 2 != 0) area->y1--;
  if (area->x2 % 2 == 0) area->x2++;
  if (area->y2 % 2 == 0) area->y2++;
}

void flushCb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color) {
  const uint32_t w = area->x2 - area->x1 + 1;
  const uint32_t h = area->y2 - area->y1 + 1;
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color->full, w, h);
#endif
  lv_disp_flush_ready(disp);
}

void tickCb(void *) { lv_tick_inc(kTickMs); }

}  // namespace

bool displayBegin() {
  bus = new Arduino_ESP32QSPI(PIN_LCD_CS, PIN_LCD_SCLK, PIN_LCD_SDIO0, PIN_LCD_SDIO1,
                              PIN_LCD_SDIO2, PIN_LCD_SDIO3);
  gfx = new Arduino_CO5300(bus, PIN_LCD_RESET, 0, LCD_WIDTH, LCD_HEIGHT, 6, 0, 0, 0);
  if (!gfx->begin()) return false;
  gfx->fillScreen(0x0000);
  gfx->setBrightness(200);

  lv_init();

  // Two 48-line buffers in fast internal RAM (~89 KB). Full-screen buffers do
  // not fit next to the LVGL pool; fall back to PSRAM if internal RAM is short.
  const size_t px = LCD_WIDTH * kBufLines;
  auto alloc = [px]() {
    void *p = heap_caps_malloc(px * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (!p) p = heap_caps_malloc(px * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    return static_cast<lv_color_t *>(p);
  };
  lv_color_t *buf1 = alloc();
  lv_color_t *buf2 = alloc();
  if (!buf1) return false;
  lv_disp_draw_buf_init(&drawBuf, buf1, buf2, px);

  lv_disp_drv_init(&dispDrv);
  dispDrv.hor_res = LCD_WIDTH;
  dispDrv.ver_res = LCD_HEIGHT;
  dispDrv.flush_cb = flushCb;
  dispDrv.rounder_cb = rounderCb;
  dispDrv.draw_buf = &drawBuf;
  lv_disp_drv_register(&dispDrv);

  const esp_timer_create_args_t args = {
      .callback = &tickCb, .arg = nullptr, .dispatch_method = ESP_TIMER_TASK,
      .name = "lvgl_tick", .skip_unhandled_events = false};
  esp_timer_handle_t timer;
  if (esp_timer_create(&args, &timer) != ESP_OK) return false;
  esp_timer_start_periodic(timer, kTickMs * 1000);
  return true;
}

void displaySetBrightness(uint8_t level) {
  if (gfx) gfx->setBrightness(level);
}

void displaySleep(bool sleep) {
  if (!gfx) return;
  if (sleep) gfx->displayOff();
  else gfx->displayOn();
}

}  // namespace hal
}  // namespace sg
