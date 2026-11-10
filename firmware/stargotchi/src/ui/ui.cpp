#include "ui.h"

#include "pages.h"
#include "theme.h"

namespace sg {
namespace ui {

namespace {

ActionHandler handler = nullptr;
lv_obj_t *tileview = nullptr;
lv_obj_t *dots[3];

void updateDots() {
  lv_obj_t *act = lv_tileview_get_tile_act(tileview);
  const int idx = act ? lv_obj_get_x(act) / LV_HOR_RES : 1;
  for (int i = 0; i < 3; i++) {
    const bool on = i == idx;
    lv_obj_set_size(dots[i], on ? 16 : 6, 6);
    lv_obj_set_style_bg_color(dots[i], lv_color_hex(on ? theme::kHolo : 0x5F5E5A), 0);
  }
}

void tileChanged(lv_event_t *) { updateDots(); }

lv_obj_t *makeTile(int col) {
  lv_obj_t *t = lv_tileview_add_tile(tileview, col, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(t, LV_OPA_TRANSP, 0);
  lv_obj_clear_flag(t, LV_OBJ_FLAG_SCROLLABLE);
  return t;
}

}  // namespace

void emit(Action a, uint32_t arg) {
  if (handler) handler(a, arg);
}

void build(ActionHandler h, const Settings &settings) {
  handler = h;
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_clean(scr);

  tileview = lv_tileview_create(scr);
  lv_obj_set_style_bg_color(tileview, lv_color_black(), 0);
  lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *stats = makeTile(0);
  lv_obj_t *home = makeTile(1);
  lv_obj_t *setup = makeTile(2);
  statsBuild(stats);
  homeBuild(home);
  setupBuild(setup, settings);
  lv_obj_set_tile_id(tileview, 1, 0, LV_ANIM_OFF);
  lv_obj_add_event_cb(tileview, tileChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  lv_obj_t *row = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, 60, 10);
  lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, -22);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(row, 8, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);
  for (int i = 0; i < 3; i++) {
    dots[i] = lv_obj_create(row);
    lv_obj_remove_style_all(dots[i]);
    lv_obj_set_style_bg_opa(dots[i], LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dots[i], 3, 0);
  }
  updateDots();
}

void refresh(const Pet &pet, uint32_t now, int battery, bool charging) {
  homeRefresh(pet, now, battery, charging);
  statsRefresh(pet, now, battery);
  setupRefresh(now);
}

void showTimePrompt(bool show) {
  setupTimePrompt(show);
  if (show) lv_obj_set_tile_id(tileview, 2, 0, LV_ANIM_ON);
}

void celebrate() { homeCelebrate(); }

void goHome() { lv_obj_set_tile_id(tileview, 1, 0, LV_ANIM_ON); }

void debugShowPage(int index) {
  lv_obj_set_tile_id(tileview, index, 0, LV_ANIM_OFF);
  updateDots();
}

}  // namespace ui
}  // namespace sg
