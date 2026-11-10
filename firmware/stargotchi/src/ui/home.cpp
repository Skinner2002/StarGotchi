// Home page: stat arcs on the rim, the bot in the middle, four icon buttons.
#include <stdio.h>
#include <string.h>

#include "../assets/bot_frames.h"
#include "../assets/sprites.h"
#include "pages.h"
#include "theme.h"

namespace sg {
namespace ui {

namespace {

constexpr int kCx = 233, kCy = 233;

lv_obj_t *arcs[4];
lv_obj_t *nameLbl, *statusLbl, *batteryLbl, *moonImg, *zzzLbl;
lv_obj_t *botImg, *flash;
lv_obj_t *btns[4];
lv_obj_t *rebootBtn;
lv_timer_t *animTimer;

Stage shownStage = Stage::Core;
bool shownShutdown = false;
bool asleep = false;
bool screenOff = false;
uint32_t animTick = 0;

void setText(lv_obj_t *l, const char *t) {
  if (strcmp(lv_label_get_text(l), t) != 0) lv_label_set_text(l, t);
}

void setVisible(lv_obj_t *o, bool v) {
  if (v) lv_obj_clear_flag(o, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t *makeArc(lv_obj_t *parent, int start, int end, bool reverse, uint32_t color) {
  lv_obj_t *a = lv_arc_create(parent);
  lv_obj_set_size(a, 452, 452);
  lv_obj_center(a);
  lv_arc_set_bg_angles(a, start, end);
  lv_arc_set_range(a, 0, 100);
  lv_arc_set_mode(a, reverse ? LV_ARC_MODE_REVERSE : LV_ARC_MODE_NORMAL);
  lv_arc_set_value(a, 0);
  lv_obj_remove_style(a, nullptr, LV_PART_KNOB);
  lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(a, 10, LV_PART_MAIN);
  lv_obj_set_style_arc_width(a, 10, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(a, lv_color_hex(theme::kTrack), LV_PART_MAIN);
  lv_obj_set_style_arc_color(a, lv_color_hex(color), LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(a, true, LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(a, true, LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(a, LV_OPA_TRANSP, 0);
  lv_obj_set_style_pad_all(a, 0, 0);
  return a;
}

void placeLabel(lv_obj_t *parent, const char *t, uint32_t color, int x, int y) {
  lv_obj_t *l = theme::label(parent, t, theme::small(), color);
  lv_obj_align(l, LV_ALIGN_CENTER, x - kCx, y - kCy);
}

void actionCb(lv_event_t *e) {
  emit(static_cast<Action>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e))));
}

lv_obj_t *iconButton(lv_obj_t *parent, const lv_img_dsc_t *icon, uint32_t color, int cx, int cy,
                     Action a) {
  lv_obj_t *b = theme::button(parent, 68, 60, color);
  lv_obj_align(b, LV_ALIGN_CENTER, cx - kCx, cy - kCy);
  lv_obj_t *img = lv_img_create(b);
  lv_img_set_src(img, icon);
  lv_obj_center(img);
  lv_obj_add_event_cb(b, actionCb, LV_EVENT_CLICKED, reinterpret_cast<void *>(static_cast<uintptr_t>(a)));
  return b;
}

void bob(void *obj, int32_t v) { lv_obj_set_style_translate_y(static_cast<lv_obj_t *>(obj), v, 0); }

void startBob() {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, botImg);
  lv_anim_set_exec_cb(&a, bob);
  lv_anim_set_values(&a, 0, -6);
  lv_anim_set_time(&a, 1100);
  lv_anim_set_playback_time(&a, 1100);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
  lv_anim_start(&a);
}

void stopBob() {
  lv_anim_del(botImg, bob);
  lv_obj_set_style_translate_y(botImg, 0, 0);
}

// 10 Hz animation clock: blinking / core shimmer / zzz.
void animCb(lv_timer_t *) {
  animTick++;
  uint8_t frame = 0;
  if (shownShutdown) frame = (animTick / 3) % 4 == 0;
  else if (shownStage == Stage::Core) frame = (animTick / 4) % 2;
  else if (asleep) frame = 1;  // eyes closed
  else frame = (animTick % 40) == 0 || (animTick % 40) == 1;
  const lv_img_dsc_t *src = shownShutdown ? shutdownFrame(frame) : botFrame(shownStage, frame);
  if (lv_img_get_src(botImg) != src) lv_img_set_src(botImg, src);
  if (asleep && !shownShutdown) {
    static const char *z[] = {"z", "zZ", "zZz", "zZz"};
    setText(zzzLbl, z[(animTick / 6) % 4]);
  }
}

void flashFade(void *obj, int32_t v) {
  lv_obj_set_style_bg_opa(static_cast<lv_obj_t *>(obj), static_cast<lv_opa_t>(v), 0);
}

}  // namespace

void homeBuild(lv_obj_t *tile) {
  arcs[0] = makeArc(tile, 182, 225, false, theme::kOil);   // OIL, upper left
  arcs[1] = makeArc(tile, 135, 178, true, theme::kMood);   // MOOD, lower left
  arcs[2] = makeArc(tile, 315, 358, true, theme::kPower);  // PWR, upper right
  arcs[3] = makeArc(tile, 2, 45, false, theme::kCond);     // COND, lower right
  placeLabel(tile, "OIL", theme::kOil, 52, 160);
  placeLabel(tile, "MOOD", theme::kMoodText, 56, 306);
  placeLabel(tile, "PWR", theme::kPower, 414, 160);
  placeLabel(tile, "COND", theme::kHolo, 410, 306);

  batteryLbl = theme::label(tile, "", theme::small(), theme::kMuted);
  lv_obj_align(batteryLbl, LV_ALIGN_TOP_MID, 0, 22);
  nameLbl = theme::label(tile, "UNIT-7  Lv.1", theme::big(), theme::kHolo);
  lv_obj_align(nameLbl, LV_ALIGN_TOP_MID, 0, 50);
  statusLbl = theme::label(tile, "", theme::small(), theme::kMuted);
  lv_obj_align(statusLbl, LV_ALIGN_TOP_MID, 0, 76);
  moonImg = lv_img_create(tile);
  lv_img_set_src(moonImg, &sg_img_moon_0);
  lv_obj_align(moonImg, LV_ALIGN_TOP_MID, 120, 64);

  // platform under the bot
  lv_obj_t *plat = lv_obj_create(tile);
  lv_obj_remove_style_all(plat);
  lv_obj_set_size(plat, 150, 8);
  lv_obj_set_style_radius(plat, 4, 0);
  lv_obj_set_style_bg_opa(plat, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(plat, lv_color_hex(theme::kTrack), 0);
  lv_obj_align(plat, LV_ALIGN_CENTER, 0, 52);

  botImg = lv_img_create(tile);
  lv_img_set_src(botImg, botFrame(Stage::Core, 0));
  lv_obj_align(botImg, LV_ALIGN_CENTER, 0, -14);
  zzzLbl = theme::label(tile, "", theme::big(), theme::kHolo);
  lv_obj_align(zzzLbl, LV_ALIGN_CENTER, 80, -80);

  btns[0] = iconButton(tile, &sg_img_icon_oil_0, theme::kOil, 113, 350, Action::Oil);
  btns[1] = iconButton(tile, &sg_img_icon_play_0, theme::kMoodText, 193, 362, Action::Play);
  btns[2] = iconButton(tile, &sg_img_icon_dock_0, theme::kPower, 273, 362, Action::Dock);
  btns[3] = iconButton(tile, &sg_img_icon_fix_0, theme::kCond, 353, 350, Action::Fix);

  rebootBtn = theme::button(tile, 160, 56, theme::kWarn);
  lv_obj_align(rebootBtn, LV_ALIGN_CENTER, 0, 125);
  lv_obj_t *rl = theme::label(rebootBtn, "REBOOT", theme::big(), theme::kWarn);
  lv_obj_center(rl);
  lv_obj_add_event_cb(rebootBtn, actionCb, LV_EVENT_CLICKED,
                      reinterpret_cast<void *>(static_cast<uintptr_t>(Action::Reboot)));
  lv_obj_add_flag(rebootBtn, LV_OBJ_FLAG_HIDDEN);

  flash = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(flash);
  lv_obj_set_size(flash, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(flash, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(flash, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(flash, LV_OBJ_FLAG_CLICKABLE);

  startBob();
  animTimer = lv_timer_create(animCb, 100, nullptr);
}

void homeRefresh(const Pet &p, uint32_t now, int battery, bool charging) {
  const float v[4] = {p.oil, p.mood, p.power, p.cond};
  for (int i = 0; i < 4; i++) {
    const int val = static_cast<int>(v[i] + 0.5f);
    if (lv_arc_get_value(arcs[i]) != val) lv_arc_set_value(arcs[i], val);
  }

  char buf[32];
  snprintf(buf, sizeof buf, "UNIT-7  Lv.%u", level(p));
  setText(nameLbl, buf);
  setText(statusLbl, statusText(p, now));
  if (battery < 0) snprintf(buf, sizeof buf, "%s", charging ? "USB" : "");
  else snprintf(buf, sizeof buf, "%d%%%s", battery, charging ? "+" : "");
  setText(batteryLbl, buf);
  setVisible(moonImg, isNight(now));

  const bool nowAsleep = isAsleep(p, now) && !p.shutdown;
  if (nowAsleep != asleep || p.shutdown != shownShutdown) {
    asleep = nowAsleep;
    if (asleep || p.shutdown || screenOff) stopBob();
    else startBob();
  }
  setVisible(zzzLbl, asleep);
  shownStage = p.stage;
  shownShutdown = p.shutdown;

  // Rust tint when the bot needs repair.
  const lv_opa_t rust = (!p.shutdown && p.cond < 20.f) ? LV_OPA_40 : LV_OPA_TRANSP;
  if (lv_obj_get_style_img_recolor_opa(botImg, 0) != rust) {
    lv_obj_set_style_img_recolor(botImg, lv_color_hex(0x993C1D), 0);
    lv_obj_set_style_img_recolor_opa(botImg, rust, 0);
  }

  for (int i = 0; i < 4; i++) setVisible(btns[i], !p.shutdown);
  setVisible(rebootBtn, p.shutdown);
  // The dock button glows while docked.
  lv_obj_set_style_bg_color(btns[2], lv_color_hex(p.docked ? 0x0F6E56 : theme::kTrack), 0);
}

void homeSetSleeping(bool sleeping) {
  screenOff = sleeping;
  if (sleeping) {
    lv_timer_pause(animTimer);
    stopBob();
  } else {
    lv_timer_resume(animTimer);
    if (!asleep && !shownShutdown) startBob();
  }
}

void homeCelebrate() {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, flash);
  lv_anim_set_exec_cb(&a, flashFade);
  lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
  lv_anim_set_time(&a, 900);
  lv_anim_set_repeat_count(&a, 2);
  lv_anim_start(&a);
}

}  // namespace ui
}  // namespace sg
