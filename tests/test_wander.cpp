#include <cstdlib>

#include "check.h"
#include "game/wander.h"

using namespace sg;

TEST(targets_stay_in_range_and_move_far_enough) {
  uint32_t rng = 12345;
  int x = 0;
  for (int i = 0; i < 500; i++) {
    const WanderStep s = nextWander(rng, x);
    CHECK(s.target >= -kWanderRange && s.target <= kWanderRange);
    CHECK(std::abs(s.target - x) >= kWanderMinStep);
    CHECK(s.pauseMs >= 1500 && s.pauseMs <= 4500);
    CHECK(s.moveMs == static_cast<uint32_t>(std::abs(s.target - x)) * 1000 / kWanderSpeed);
    x = s.target;
  }
}

TEST(visits_both_sides) {
  uint32_t rng = 7;
  int x = 0, minX = 0, maxX = 0;
  for (int i = 0; i < 50; i++) {
    x = nextWander(rng, x).target;
    if (x < minX) minX = x;
    if (x > maxX) maxX = x;
  }
  CHECK(minX < -40);
  CHECK(maxX > 40);
}

TEST(deterministic) {
  uint32_t a = 99, b = 99;
  CHECK(nextWander(a, 10).target == nextWander(b, 10).target);
}

RUN_TESTS()
