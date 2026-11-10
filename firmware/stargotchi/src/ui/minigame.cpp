#include "minigame.h"

#include <stdio.h>

#include "../assets/sprites.h"
#include "../game/asteroid.h"
#include "theme.h"

namespace sg {
namespace ui {

namespace {

constexpr int kStars = 10;
constexpr uint32_t kReadyMs = 1500;

MinigameIO io{};
AsteroidGame game;
lv_obj_t *root = nullptr;
lv_obj_t *ship, *rockImgs[AsteroidGame::kMaxRocks], *stars[kStars];
lv_obj_t *scoreLbl, *infoLbl, *centerLbl;
lv_timer_t *timer = nullptr;
uint32_t startMs = 0, lastMs = 0;
int shownLives = AsteroidGame::kLives;
bool resultShown = false;
float starY[kStars];

void place(lv_obj_t *o, float cx, float cy) {
  lv_obj_set_pos(o, static_cast<lv_coord_t>(cx - lv_obj_get_width(o) / 2),
                 static_cast<lv_coord_t>(cy - lv_obj_get_height(o) / 2));
}

void okCb(lv_event_t *) {
  const uint16_t score = game.score();
  if (timer) lv_timer_del(timer);
  timer = nullptr;
  lv_obj_del(root);
  root = nullptr;
  if (io.finished) io.finished(score);
}

void showResult() {
  resultShown = true;
  char buf[48];
  snprintf(buf, sizeof buf, "%s\nSCORE %u", game.lives() > 0 ? "MISSION COMPLETE" : "HULL BREACH",
           game.score());
  lv_label_set_text(centerLbl, buf);
  lv_obj_clear_flag(centerLbl, LV_OBJ_FLAG_HIDDEN);
  lv_obj_t *ok = theme::button(root, 120, 48, theme::kCond);
  lv_obj_align(ok, LV_ALIGN_CENTER, 0, 70);
  lv_obj_t *l = theme::label(ok, "OK", theme::big(), theme::kHolo);
  lv_obj_center(l);
  lv_obj_add_event_cb(ok, okCb, LV_EVENT_CLICKED, nullptr);
  if (io.beep) {
    io.beep(523, 120);
    io.beep(game.lives() > 0 ? 784 : 262, 250);
  }
}

void tick(lv_timer_t *) {
  const uint32_t now = lv_tick_get();
  float dt = (now - lastMs) / 1000.f;
  lastMs = now;
  if (dt > 0.1f) dt = 0.1f;

  // background stars drift for a sense of speed
  for (int i = 0; i < kStars; i++) {
    starY[i] += dt * (60.f + 15.f * i);
    if (starY[i] > 466) starY[i] -= 466;
    lv_obj_set_y(stars[i], static_cast<lv_coord_t>(starY[i]));
  }

  if (now - startMs < kReadyMs || resultShown) return;
  if (!lv_obj_has_flag(centerLbl, LV_OBJ_FLAG_HIDDEN)) lv_obj_add_flag(centerLbl, LV_OBJ_FLAG_HIDDEN);

  game.step(dt, io.tilt ? io.tilt() : 0.f);
  place(ship, game.shipX(), AsteroidGame::kShipY);
  for (int i = 0; i < AsteroidGame::kMaxRocks; i++) {
    const Rock &r = game.rocks()[i];
    if (r.active) {
      lv_obj_clear_flag(rockImgs[i], LV_OBJ_FLAG_HIDDEN);
      place(rockImgs[i], r.x, r.y);
    } else {
      lv_obj_add_flag(rockImgs[i], LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (game.lives() < shownLives) {
    shownLives = game.lives();
    if (io.beep) io.beep(180, 150);
  }
  char buf[32];
  snprintf(buf, sizeof buf, "%u", game.score());
  lv_label_set_text(scoreLbl, buf);
  snprintf(buf, sizeof buf, "HULL %d  T-%02d", game.lives(),
           static_cast<int>(AsteroidGame::kDuration - game.elapsed() + 0.99f));
  lv_label_set_text(infoLbl, buf);
  if (game.over()) showResult();
}

}  // namespace

bool minigameActive() { return root != nullptr; }

void minigameStart(const MinigameIO &ioIn, uint32_t seed) {
  if (root) return;
  io = ioIn;
  game.begin(seed);
  shownLives = AsteroidGame::kLives;
  resultShown = false;

  root = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(root);
  lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(root, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
  lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

  for (int i = 0; i < kStars; i++) {
    stars[i] = lv_obj_create(root);
    lv_obj_remove_style_all(stars[i]);
    const int sz = i % 3 == 0 ? 3 : 2;
    lv_obj_set_size(stars[i], sz, sz);
    lv_obj_set_style_bg_opa(stars[i], LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(stars[i], lv_color_hex(i % 2 ? theme::kMuted : 0x5F5E5A), 0);
    lv_obj_set_x(stars[i], 90 + (i * 97) % 290);
    starY[i] = (i * 151) % 466;
  }
  for (int i = 0; i < AsteroidGame::kMaxRocks; i++) {
    rockImgs[i] = lv_img_create(root);
    lv_img_set_src(rockImgs[i], &sg_img_rock_0);
    lv_obj_add_flag(rockImgs[i], LV_OBJ_FLAG_HIDDEN);
  }
  ship = lv_img_create(root);
  lv_img_set_src(ship, &sg_img_ship_0);
  lv_obj_update_layout(root);
  place(ship, game.shipX(), AsteroidGame::kShipY);

  scoreLbl = theme::label(root, "0", theme::big(), theme::kOil);
  lv_obj_align(scoreLbl, LV_ALIGN_TOP_MID, 0, 28);
  infoLbl = theme::label(root, "", theme::small(), theme::kMuted);
  lv_obj_align(infoLbl, LV_ALIGN_TOP_MID, 0, 52);
  centerLbl = theme::label(root, "GET READY\nTILT TO STEER", theme::big(), theme::kHolo);
  lv_obj_set_style_text_align(centerLbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(centerLbl, LV_ALIGN_CENTER, 0, -20);

  startMs = lastMs = lv_tick_get();
  timer = lv_timer_create(tick, 33, nullptr);
}

}  // namespace ui
}  // namespace sg
