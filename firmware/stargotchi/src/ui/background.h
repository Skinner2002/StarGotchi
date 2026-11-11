#pragma once
#include <lvgl.h>

namespace sg {
namespace ui {

// Builds the planet scenery as the first children of the Home tile.
void bgBuild(lv_obj_t *parent);
void bgSetNight(bool night);

}  // namespace ui
}  // namespace sg
