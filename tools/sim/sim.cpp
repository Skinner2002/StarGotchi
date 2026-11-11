// Desktop renderer: draws the real UI with LVGL into a 466x466 buffer and
// writes PNG-ready PPM screenshots. Used to check layouts without hardware.
// Build/run: tools/sim/run_sim.sh
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include "game/pet.h"
#include "ui/ui.h"
#include "ui/minigame.h"

static lv_color_t fb[466 * 466];
static lv_color_t buf[466 * 466 / 4];

static void flush(lv_disp_drv_t *d, const lv_area_t *a, lv_color_t *c) {
  for (int y = a->y1; y <= a->y2; y++)
    for (int x = a->x1; x <= a->x2; x++) fb[y * 466 + x] = *c++;
  lv_disp_flush_ready(d);
}

static void run(int ms) {
  for (int i = 0; i < ms; i += 5) {
    lv_tick_inc(5);
    lv_timer_handler();
  }
}

static void shot(const char *name) {
  run(300);
  std::string path = std::string("sim_out/") + name + ".ppm";
  FILE *f = fopen(path.c_str(), "wb");
  fprintf(f, "P6\n466 466\n255\n");
  for (int i = 0; i < 466 * 466; i++) {
    uint16_t v = fb[i].full;
    int dx = i % 466 - 233, dy = i / 466 - 233;
    bool in = dx * dx + dy * dy <= 233 * 233;
    unsigned char rgb[3] = {(unsigned char)(in ? ((v >> 11) & 31) * 255 / 31 : 40),
                            (unsigned char)(in ? ((v >> 5) & 63) * 255 / 63 : 40),
                            (unsigned char)(in ? (v & 31) * 255 / 31 : 40)};
    fwrite(rgb, 1, 3, f);
  }
  fclose(f);
}

int main() {
  lv_init();
  static lv_disp_draw_buf_t db;
  lv_disp_draw_buf_init(&db, buf, nullptr, 466 * 466 / 4);
  static lv_disp_drv_t dd;
  lv_disp_drv_init(&dd);
  dd.hor_res = 466;
  dd.ver_res = 466;
  dd.flush_cb = flush;
  dd.draw_buf = &db;
  lv_disp_drv_register(&dd);

  const uint32_t day0 = 1767571200u;  // 2026-01-05 00:00
  sg::Pet p = sg::newPet(day0 + 10 * 3600);
  p.stage = sg::Stage::Junior;
  p.line = 1;
  p.oil = 70; p.mood = 45; p.power = 85; p.cond = 30; p.xp = 340;
  sg::ui::build([](sg::ui::Action, uint32_t) {}, {200, true});
  sg::ui::refresh(p, day0 + 3 * 86400 + 14 * 3600, 84, false);
  shot("home_day");

  p.stage = sg::Stage::Astro; p.line = 2; p.docked = true; p.cond = 12;
  sg::ui::refresh(p, day0 + 8 * 86400 + 23 * 3600, 52, true);
  shot("home_night");

  p.shutdown = true; p.stage = sg::Stage::Scrap;
  sg::ui::refresh(p, day0 + 9 * 86400 + 12 * 3600, -1, true);
  shot("home_shutdown");

  p = sg::newPet(day0);
  p.stage = sg::Stage::Combat; p.line = 0; p.xp = 1234; p.careSum = 7000; p.careSamples = 100;
  sg::ui::refresh(p, day0 + 9 * 86400 + 12 * 3600, 77, false);
  sg::ui::debugShowPage(0);
  shot("stats");
  sg::ui::debugShowPage(2);
  sg::ui::showTimePrompt(true);
  shot("setup");
  sg::ui::debugShowPage(1);

  sg::ui::MinigameIO io{[]() { return 0.3f; }, [](uint16_t, uint16_t) {}, [](uint16_t) {}};
  sg::ui::minigameStart(io, 42);
  shot("minigame_ready");
  run(6000);
  shot("minigame");
  return 0;
}
