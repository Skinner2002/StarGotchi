#include "wander.h"

#include <stdlib.h>

namespace sg {

static uint32_t next(uint32_t &r) {
  if (r == 0) r = 1;
  r ^= r << 13;
  r ^= r >> 17;
  r ^= r << 5;
  return r;
}

WanderStep nextWander(uint32_t &rng, int current) {
  int target = current;
  while (abs(target - current) < kWanderMinStep) {
    target = static_cast<int>(next(rng) % (2 * kWanderRange + 1)) - kWanderRange;
  }
  WanderStep s;
  s.target = target;
  s.moveMs = static_cast<uint32_t>(abs(target - current)) * 1000 / kWanderSpeed;
  s.pauseMs = 1500 + next(rng) % 3001;
  return s;
}

}  // namespace sg
