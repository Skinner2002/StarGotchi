// Setup page: clock, brightness, sound, reset.
#include <stdio.h>
#include <string.h>

#include "../game/clock.h"
#include "pages.h"
#include "theme.h"

namespace sg {
namespace ui {

namespace {

lv_obj_t *clockLbl, *promptLbl, *hourRoller, *minRoller, *msgbox;
bool rollersTouched = false;

char hours[24 * 3 + 1];
char mins[60 * 3 + 1];

void fillOptions() {
  char *p = hours;
  for (int i = 0; i < 24; i++) p += sprintf(p, i ? "\n%02d" : "%02d", i);
  p = mins;
  for (int i = 0; i < 60; i++) p += sprintf(p, i ? "\n%02d" : "%02d", i);
}

lv_obj_t *roller(lv_obj_t *parent, const char *opts, int x) {
  lv_obj_t *r = lv_roller_create(parent);
  lv_roller_set_options(r, opts, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(r, 3);
  lv_obj_set_width(r, 76);
  lv_obj_set_style_text_font(r, theme::big(), 0);
  lv_obj_set_style_text_font(r, theme::big(), LV_PART_SELECTED);
  lv_obj_set_style_bg_color(r, lv_color_hex(theme::kPanel), 0);
  lv_obj_set_style_border_color(r, lv_color_hex(theme::kTrack), 0);
  lv_obj_set_style_text_color(r, lv_color_hex(theme::kMuted), 0);
  lv_obj_set_style_bg_color(r, lv_color_hex(theme::kCond), LV_PART_SELECTED);
  lv_obj_set_style_text_color(r, lv_color_white(), LV_PART_SELECTED);
  lv_obj_align(r, LV_ALIGN_TOP_MID, x, 104);
  lv_obj_add_event_cb(r, [](lv_event_t *) { rollersTouched = true; }, LV_EVENT_VALUE_CHANGED, nullptr);
  return r;
}

void setTimeCb(lv_event_t *) {
  const uint32_t m = lv_roller_get_selected(hourRoller) * 60 + lv_roller_get_selected(minRoller);
  rollersTouched = false;
  emit(Action::SetTime, m);
}

void brightCb(lv_event_t *e) {
  emit(Action::Brightness, lv_slider_get_value(lv_event_get_target(e)));
}

void volumeCb(lv_event_t *e) {
  emit(Action::Volume, lv_slider_get_value(lv_event_get_target(e)));
}

lv_obj_t *settingSlider(lv_obj_t *tile, const char *name, int y, int min, int max, int value,
                        uint32_t color, lv_event_cb_t cb) {
  lv_obj_t *l = theme::label(tile, name, theme::small(), theme::kMuted);
  lv_obj_align(l, LV_ALIGN_TOP_MID, -100, y + 2);
  lv_obj_t *slider = lv_slider_create(tile);
  lv_obj_set_width(slider, 150);
  lv_slider_set_range(slider, min, max);
  lv_slider_set_value(slider, value, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider, lv_color_hex(color), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, lv_color_hex(color), LV_PART_KNOB);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 30, y);
  lv_obj_add_event_cb(slider, cb, LV_EVENT_RELEASED, nullptr);
  lv_obj_clear_flag(slider, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);  // dragging must not flip pages
  return slider;
}

void msgboxCb(lv_event_t *e) {
  lv_obj_t *mb = lv_event_get_current_target(e);
  const char *txt = lv_msgbox_get_active_btn_text(mb);
  if (txt && strcmp(txt, "Reset") == 0) emit(Action::Reset);
  lv_msgbox_close(mb);
  msgbox = nullptr;
}

void resetCb(lv_event_t *) {
  if (msgbox) return;
  static const char *btns[] = {"Reset", "Cancel", ""};
  msgbox = lv_msgbox_create(nullptr, "Reset bot?", "Your current bot will be lost.", btns, false);
  lv_obj_set_width(msgbox, 300);
  lv_obj_center(msgbox);
  lv_obj_set_style_text_font(msgbox, theme::small(), 0);
  lv_obj_set_style_bg_color(msgbox, lv_color_hex(theme::kPanel), 0);
  lv_obj_set_style_border_color(msgbox, lv_color_hex(theme::kWarn), 0);
  lv_obj_set_style_border_width(msgbox, 2, 0);
  lv_obj_add_event_cb(msgbox, msgboxCb, LV_EVENT_VALUE_CHANGED, nullptr);
}

lv_obj_t *textButton(lv_obj_t *parent, const char *t, uint32_t color, int w, int y) {
  lv_obj_t *b = theme::button(parent, w, 40, color);
  lv_obj_align(b, LV_ALIGN_TOP_MID, 0, y);
  lv_obj_t *l = theme::label(b, t, theme::small(), color);
  lv_obj_center(l);
  return b;
}

}  // namespace

void setupBuild(lv_obj_t *tile, const Settings &s) {
  fillOptions();
  lv_obj_t *title = theme::label(tile, "SETUP", theme::big(), theme::kHolo);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
  clockLbl = theme::label(tile, "--:--", theme::big(), theme::kMuted);
  lv_obj_align(clockLbl, LV_ALIGN_TOP_MID, 0, 66);
  promptLbl = theme::label(tile, "Set the clock!", theme::small(), theme::kOil);
  lv_obj_align(promptLbl, LV_ALIGN_TOP_MID, 0, 88);
  lv_obj_add_flag(promptLbl, LV_OBJ_FLAG_HIDDEN);

  hourRoller = roller(tile, hours, -44);
  minRoller = roller(tile, mins, 44);
  lv_obj_t *colon = theme::label(tile, ":", theme::big(), theme::kMuted);
  lv_obj_align(colon, LV_ALIGN_TOP_MID, 0, 150);
  lv_obj_t *setBtn = textButton(tile, "SET TIME", theme::kCond, 160, 222);
  lv_obj_add_event_cb(setBtn, setTimeCb, LV_EVENT_CLICKED, nullptr);

  settingSlider(tile, "BRIGHT", 288, 20, 255, s.brightness, theme::kHolo, brightCb);
  settingSlider(tile, "VOLUME", 330, 0, 100, s.volume, theme::kPower, volumeCb);

  lv_obj_t *resetBtn = textButton(tile, "RESET BOT", theme::kWarn, 140, 370);
  lv_obj_add_event_cb(resetBtn, resetCb, LV_EVENT_CLICKED, nullptr);
  // Rollers need their final size before the first selection is applied.
  lv_obj_update_layout(tile);
}

void setupRefresh(uint32_t now) {
  const DateTime d = fromEpoch(now);
  char buf[8];
  snprintf(buf, sizeof buf, "%02d:%02d", d.hour, d.minute);
  if (strcmp(lv_label_get_text(clockLbl), buf) != 0) lv_label_set_text(clockLbl, buf);
  if (!rollersTouched) {
    if (lv_roller_get_selected(hourRoller) != static_cast<uint16_t>(d.hour))
      lv_roller_set_selected(hourRoller, d.hour, LV_ANIM_OFF);
    if (lv_roller_get_selected(minRoller) != static_cast<uint16_t>(d.minute))
      lv_roller_set_selected(minRoller, d.minute, LV_ANIM_OFF);
  }
}

void setupTimePrompt(bool show) {
  if (show) lv_obj_clear_flag(promptLbl, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(promptLbl, LV_OBJ_FLAG_HIDDEN);
}

}  // namespace ui
}  // namespace sg
