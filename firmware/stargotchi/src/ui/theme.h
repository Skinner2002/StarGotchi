#pragma once
#include <lvgl.h>

namespace sg {
namespace theme {

constexpr uint32_t kOil = 0xEF9F27;
constexpr uint32_t kMood = 0xD85A30;
constexpr uint32_t kMoodText = 0xF0997B;
constexpr uint32_t kPower = 0x5DCAA5;
constexpr uint32_t kCond = 0x378ADD;
constexpr uint32_t kHolo = 0x85B7EB;
constexpr uint32_t kMuted = 0xB4B2A9;
constexpr uint32_t kTrack = 0x2C2C2A;
constexpr uint32_t kPanel = 0x16171C;
constexpr uint32_t kWarn = 0xE24B4A;

inline const lv_font_t *big() { return &lv_font_unscii_16; }
inline const lv_font_t *small() { return &lv_font_unscii_8; }

inline lv_obj_t *label(lv_obj_t *parent, const char *text, const lv_font_t *font, uint32_t color) {
  lv_obj_t *l = lv_label_create(parent);
  lv_label_set_text(l, text);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
  return l;
}

// Flat dark button with a coloured border.
inline lv_obj_t *button(lv_obj_t *parent, int w, int h, uint32_t border) {
  lv_obj_t *b = lv_btn_create(parent);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_bg_color(b, lv_color_hex(kTrack), 0);
  lv_obj_set_style_bg_color(b, lv_color_hex(0x444441), LV_STATE_PRESSED);
  lv_obj_set_style_border_color(b, lv_color_hex(border), 0);
  lv_obj_set_style_border_width(b, 2, 0);
  lv_obj_set_style_radius(b, 12, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_set_style_pad_all(b, 0, 0);
  return b;
}

}  // namespace theme
}  // namespace sg
