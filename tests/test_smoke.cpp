#include "check.h"

TEST(harness_counts_checks) {
  CHECK(1 + 1 == 2);
  CHECK_NEAR(0.1 + 0.2, 0.3, 1e-9);
}

RUN_TESTS()
