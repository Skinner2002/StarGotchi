#pragma once
// LVGL user interface: three swipeable pages (Stats, Home, Setup).
#include <stdint.h>

#include "../game/pet.h"

namespace sg {
namespace ui {

enum class Action : uint8_t {
  Oil, Play, Dock, Fix, Reboot,
  SetTime,     // arg = minutes since midnight
  Brightness,  // arg = 20..255
  Volume,      // arg = 0..100 (0 = muted)
  Reset,
};
using ActionHandler = void (*)(Action, uint32_t arg);

struct Settings { uint8_t brightness; uint8_t volume; };

void build(ActionHandler handler, const Settings &settings);
void refresh(const Pet &pet, uint32_t now, int battery, bool charging);
void showTimePrompt(bool show);  // jumps to Setup and highlights the clock
void celebrate();                 // evolution flash
void goHome();
void setSleeping(bool sleeping);  // pause animations while the screen is off
void debugShowPage(int index);  // 0 Stats, 1 Home, 2 Setup (no animation)

}  // namespace ui
}  // namespace sg
