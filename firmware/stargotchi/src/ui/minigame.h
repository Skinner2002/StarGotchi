#pragma once
// Full-screen "Asteroid Run" mini-game view.
#include <stdint.h>

namespace sg {
namespace ui {

struct MinigameIO {
  float (*tilt)();                       // -1..1
  void (*beep)(uint16_t hz, uint16_t ms);
  void (*finished)(uint16_t score);      // called when the player taps OK
};

void minigameStart(const MinigameIO &io, uint32_t seed);
bool minigameActive();

}  // namespace ui
}  // namespace sg
