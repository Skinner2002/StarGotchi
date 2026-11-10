#include "check.h"
#include "game/clock.h"

using namespace sg;

TEST(known_epochs) {
  CHECK(toEpoch({1970, 1, 1, 0, 0, 0}) == 0u);
  CHECK(toEpoch({2026, 1, 5, 0, 0, 0}) == 1767571200u);
  CHECK(toEpoch({2028, 2, 29, 23, 59, 59}) == 1835481599u);
}

TEST(roundtrip_many_days) {
  for (uint32_t t = 1767571200u; t < 1767571200u + 800u * 86400u; t += 86400u + 3671u) {
    CHECK(toEpoch(fromEpoch(t)) == t);
  }
}

TEST(fields) {
  DateTime d = fromEpoch(1835481599u);
  CHECK(d.year == 2028 && d.month == 2 && d.day == 29);
  CHECK(d.hour == 23 && d.minute == 59 && d.second == 59);
}

TEST(valid_time) {
  CHECK(!isValidTime({2000, 1, 1, 0, 0, 0}));
  CHECK(isValidTime({2026, 11, 11, 8, 0, 0}));
  CHECK(!isValidTime({2026, 13, 1, 0, 0, 0}));
}

RUN_TESTS()
