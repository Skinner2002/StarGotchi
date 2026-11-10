#include "bot_frames.h"

#include "sprites.h"

namespace sg {

const lv_img_dsc_t *botFrame(Stage stage, uint8_t frame) {
  const bool f = frame & 1;
  switch (stage) {
    case Stage::Core: return f ? &sg_img_core_1 : &sg_img_core_0;
    case Stage::Mini: return f ? &sg_img_mini_1 : &sg_img_mini_0;
    case Stage::Junior: return f ? &sg_img_junior_1 : &sg_img_junior_0;
    case Stage::Astro: return f ? &sg_img_astro_1 : &sg_img_astro_0;
    case Stage::Combat: return f ? &sg_img_combat_1 : &sg_img_combat_0;
    case Stage::Scrap: return f ? &sg_img_scrap_1 : &sg_img_scrap_0;
  }
  return &sg_img_core_0;
}

const lv_img_dsc_t *shutdownFrame(uint8_t frame) {
  return (frame & 1) ? &sg_img_shutdown_1 : &sg_img_shutdown_0;
}

const char *stageName(Stage stage) {
  switch (stage) {
    case Stage::Core: return "Core";
    case Stage::Mini: return "Mini-Bot";
    case Stage::Junior: return "Junior-Bot";
    case Stage::Astro: return "Nova-Bot";
    case Stage::Combat: return "Combat-Bot";
    case Stage::Scrap: return "Scrap-Bot";
  }
  return "?";
}

}  // namespace sg
