#pragma once
// Internal: page builders shared by ui.cpp.
#include <lvgl.h>

#include "ui.h"

namespace sg {
namespace ui {

void emit(Action a, uint32_t arg = 0);

void homeBuild(lv_obj_t *tile);
void homeRefresh(const Pet &pet, uint32_t now, int battery, bool charging);
void homeCelebrate();
void homeSetSleeping(bool sleeping);

void statsBuild(lv_obj_t *tile);
void statsRefresh(const Pet &pet, uint32_t now, int battery);

void setupBuild(lv_obj_t *tile, const Settings &s);
void setupRefresh(uint32_t now);
void setupTimePrompt(bool show);

}  // namespace ui
}  // namespace sg
