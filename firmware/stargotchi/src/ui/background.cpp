// Home-page scenery: an alien planet horizon built from flat LVGL shapes
// (cheap to redraw behind the animated bot). Day and night variants.
#include "background.h"

namespace sg {
namespace ui {

namespace {

lv_obj_t *dayLayer = nullptr;
lv_obj_t *nightLayer = nullptr;

lv_obj_t *layer(lv_obj_t *parent) {
  lv_obj_t *l = lv_obj_create(parent);
  lv_obj_remove_style_all(l);
  lv_obj_set_size(l, 466, 466);
  lv_obj_set_pos(l, 0, 0);
  lv_obj_clear_flag(l, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return l;
}

lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color, int radius = 0) {
  lv_obj_t *o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_clear_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

void bands(lv_obj_t *parent, int y0, int y1, const uint32_t *colors, int n) {
  const int h = (y1 - y0 + n - 1) / n;
  for (int i = 0; i < n; i++) box(parent, 0, y0 + i * h, 466, h + 1, colors[i]);
}

constexpr int kHorizon = 252;

// Mesa silhouettes along the horizon: {x, width, height}
const int kMesas[][3] = {{28, 54, 10}, {70, 22, 18}, {150, 70, 8}, {268, 40, 14},
                         {300, 18, 22}, {360, 80, 9}};

void scenery(lv_obj_t *l, bool night) {
  static const uint32_t daySky[] = {0x0B1026, 0x142048, 0x1E3466, 0x2B4E86, 0x3C6EA3,
                                    0x5B8FB8, 0x86B3C9, 0xA9C8CF};
  static const uint32_t nightSky[] = {0x010208, 0x03050F, 0x060A1A, 0x0A1026, 0x0F1733,
                                      0x141E40, 0x1A264D, 0x22305A};
  static const uint32_t dayGround[] = {0x8A5E44, 0x7A5240, 0x6B4A3A, 0x5A3E31, 0x4A3329,
                                       0x3B2921, 0x2E211B};
  static const uint32_t nightGround[] = {0x3A2A2A, 0x332424, 0x2B1F1F, 0x241A1A, 0x1D1515,
                                         0x161010, 0x0F0B0B};
  bands(l, 0, kHorizon, night ? nightSky : daySky, 8);
  bands(l, kHorizon, 466, night ? nightGround : dayGround, 7);

  if (night) {
    // star field (fixed pattern)
    for (int i = 0; i < 42; i++) {
      const int x = (i * 97 + 31) % 430 + 18;
      const int y = (i * 53 + 17) % 220 + 8;
      const int s = (i % 7 == 0) ? 3 : 2;
      box(l, x, y, s, s, i % 3 ? 0xB4B2A9 : 0xF1EFE8);
    }
  }

  // ringed planet (upper left) and a small moon (upper right)
  const uint32_t planet = night ? 0x5A4E66 : 0xC9A27A;
  const uint32_t planetShade = night ? 0x433A4D : 0xA9805C;
  // ring first so the planet body hides its middle (ring passes behind)
  lv_obj_t *ring = box(l, 92, 126, 124, 20, planet, LV_RADIUS_CIRCLE);
  lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_color(ring, lv_color_hex(night ? 0x7F77DD : 0xFAC775), 0);
  lv_obj_set_style_border_width(ring, 2, 0);
  box(l, 122, 104, 64, 64, planet, LV_RADIUS_CIRCLE);
  box(l, 124, 140, 60, 6, planetShade);  // cloud bands on the planet
  box(l, 130, 154, 48, 5, planetShade);
  box(l, 332, 96, 18, 18, night ? 0xD3D1C7 : 0xF1EFE8, LV_RADIUS_CIRCLE);

  // horizon mesas and ground stripes
  const uint32_t mesa = night ? 0x1B1424 : 0x4A3B52;
  for (const auto &m : kMesas) box(l, m[0], kHorizon - m[2], m[1], m[2], mesa);
  const uint32_t stripe = night ? 0x423030 : 0x9C6E50;
  box(l, 40, 300, 60, 2, stripe);
  box(l, 330, 318, 80, 2, stripe);
  box(l, 120, 400, 50, 2, stripe);
  box(l, 300, 420, 40, 2, stripe);
}

}  // namespace

void bgBuild(lv_obj_t *parent) {
  dayLayer = layer(parent);
  scenery(dayLayer, false);
  nightLayer = layer(parent);
  scenery(nightLayer, true);
  lv_obj_add_flag(nightLayer, LV_OBJ_FLAG_HIDDEN);
}

void bgSetNight(bool night) {
  if (!dayLayer) return;
  const bool shown = !lv_obj_has_flag(nightLayer, LV_OBJ_FLAG_HIDDEN);
  if (shown == night) return;
  if (night) {
    lv_obj_clear_flag(nightLayer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(dayLayer, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(dayLayer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(nightLayer, LV_OBJ_FLAG_HIDDEN);
  }
}

}  // namespace ui
}  // namespace sg
