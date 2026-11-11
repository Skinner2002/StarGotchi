// Stats page: level, XP, age, stage, average care, battery.
#include <stdio.h>
#include <string.h>

#include "../assets/bot_frames.h"
#include "pages.h"
#include "theme.h"

#ifndef SG_VERSION
#define SG_VERSION "dev"
#endif

namespace sg {
namespace ui {

namespace {
lv_obj_t *rows[6];

void setText(lv_obj_t *l, const char *t) {
  if (strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t);
}
}  // namespace

void statsBuild(lv_obj_t *tile) {
  lv_obj_t *title = theme::label(tile, "STATS", theme::big(), theme::kHolo);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 60);
  for (int i = 0; i < 6; i++) {
    rows[i] = theme::label(tile, "", theme::big(), i % 2 ? theme::kMuted : 0xD3D1C7);
    lv_obj_align(rows[i], LV_ALIGN_TOP_MID, 0, 120 + i * 40);
  }
  lv_obj_t *ver = theme::label(tile, "StarGotchi v" SG_VERSION, theme::small(), 0x5F5E5A);
  lv_obj_align(ver, LV_ALIGN_TOP_MID, 0, 380);
}

void statsRefresh(const Pet &p, uint32_t now, int battery) {
  char buf[40];
  snprintf(buf, sizeof buf, "Level %u", level(p));
  setText(rows[0], buf);
  snprintf(buf, sizeof buf, "XP %lu", static_cast<unsigned long>(p.xp));
  setText(rows[1], buf);
  const uint32_t age = now > p.bornAt ? (now - p.bornAt) / 86400u : 0;
  snprintf(buf, sizeof buf, "Age %lu day%s", static_cast<unsigned long>(age), age == 1 ? "" : "s");
  setText(rows[2], buf);
  setText(rows[3], p.shutdown ? "Offline" : stageName(p.stage, p.line));
  const int care = p.careSamples ? static_cast<int>(p.careSum / p.careSamples + 0.5f) : 0;
  snprintf(buf, sizeof buf, "Care %d%%", care);
  setText(rows[4], buf);
  if (battery >= 0) snprintf(buf, sizeof buf, "Battery %d%%", battery);
  else snprintf(buf, sizeof buf, "No battery");
  setText(rows[5], buf);
}

}  // namespace ui
}  // namespace sg
