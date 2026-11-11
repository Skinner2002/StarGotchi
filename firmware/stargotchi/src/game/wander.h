#pragma once
// Idle walking: picks where the bot strolls to next and how long it rests.
#include <stdint.h>

namespace sg {

constexpr int kWanderRange = 80;    // px left/right of centre
constexpr int kWanderMinStep = 30;  // never a tiny shuffle
constexpr int kWanderSpeed = 30;    // px per second

struct WanderStep {
  int target;        // x offset from centre
  uint32_t moveMs;   // walking time at kWanderSpeed
  uint32_t pauseMs;  // rest before the next step (1.5..4.5 s)
};

WanderStep nextWander(uint32_t &rng, int current);

}  // namespace sg
