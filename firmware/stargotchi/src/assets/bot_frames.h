#pragma once
#include <lvgl.h>

#include "../game/pet.h"

namespace sg {

// Idle (0) and alternate/blink (1) frame for a stage.
const lv_img_dsc_t *botFrame(Stage stage, uint8_t frame);
const lv_img_dsc_t *shutdownFrame(uint8_t frame);
const char *stageName(Stage stage);

}  // namespace sg
