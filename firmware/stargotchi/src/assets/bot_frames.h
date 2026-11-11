#pragma once
#include <lvgl.h>

#include "../game/pet.h"

namespace sg {

// Idle (0) and blink (1) frame for a unit line and stage.
const lv_img_dsc_t *botFrame(Stage stage, uint8_t line, uint8_t frame);
// Walking frames seen from the side (the core has none and returns its front).
const lv_img_dsc_t *botSideFrame(Stage stage, uint8_t line, bool right, uint8_t frame);
const lv_img_dsc_t *shutdownFrame(uint8_t frame);
// e.g. "Nova Crawler", "Mini Hover", "Energy Core"
const char *stageName(Stage stage, uint8_t line);
const char *lineName(uint8_t line);

}  // namespace sg
