#include "bot_frames.h"

#include "sprites.h"

namespace sg {

namespace {

// [line][stage - Mini][frame]
const lv_img_dsc_t *const kFrames[kLineCount][5][2] = {
    {{&sg_img_crawler_mini_0, &sg_img_crawler_mini_1},
     {&sg_img_crawler_junior_0, &sg_img_crawler_junior_1},
     {&sg_img_crawler_nova_0, &sg_img_crawler_nova_1},
     {&sg_img_crawler_combat_0, &sg_img_crawler_combat_1},
     {&sg_img_crawler_scrap_0, &sg_img_crawler_scrap_1}},
    {{&sg_img_wheeler_mini_0, &sg_img_wheeler_mini_1},
     {&sg_img_wheeler_junior_0, &sg_img_wheeler_junior_1},
     {&sg_img_wheeler_nova_0, &sg_img_wheeler_nova_1},
     {&sg_img_wheeler_combat_0, &sg_img_wheeler_combat_1},
     {&sg_img_wheeler_scrap_0, &sg_img_wheeler_scrap_1}},
    {{&sg_img_hover_mini_0, &sg_img_hover_mini_1},
     {&sg_img_hover_junior_0, &sg_img_hover_junior_1},
     {&sg_img_hover_nova_0, &sg_img_hover_nova_1},
     {&sg_img_hover_combat_0, &sg_img_hover_combat_1},
     {&sg_img_hover_scrap_0, &sg_img_hover_scrap_1}},
};

const char *const kLineNames[kLineCount] = {"Crawler", "Wheeler", "Hover"};
const char *const kNames[kLineCount][5] = {
    {"Mini Crawler", "Junior Crawler", "Nova Crawler", "Combat Crawler", "Scrap Crawler"},
    {"Mini Wheeler", "Junior Wheeler", "Nova Wheeler", "Combat Wheeler", "Scrap Wheeler"},
    {"Mini Hover", "Junior Hover", "Nova Hover", "Combat Hover", "Scrap Hover"},
};

}  // namespace

const lv_img_dsc_t *botFrame(Stage stage, uint8_t line, uint8_t frame) {
  if (stage == Stage::Core) return (frame & 1) ? &sg_img_core_1 : &sg_img_core_0;
  const int s = static_cast<int>(stage) - 1;
  return kFrames[line % kLineCount][s][frame & 1];
}

const lv_img_dsc_t *shutdownFrame(uint8_t frame) {
  return (frame & 1) ? &sg_img_shutdown_1 : &sg_img_shutdown_0;
}

const char *stageName(Stage stage, uint8_t line) {
  if (stage == Stage::Core) return "Energy Core";
  return kNames[line % kLineCount][static_cast<int>(stage) - 1];
}

const char *lineName(uint8_t line) { return kLineNames[line % kLineCount]; }

}  // namespace sg
