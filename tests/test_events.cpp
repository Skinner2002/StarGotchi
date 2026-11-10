#include "check.h"
#include "game/clock.h"
#include "game/events.h"

using namespace sg;

static const uint32_t kDay0 = 1767571200u;  // 2026-01-05 00:00

TEST(low_alert_fires_once_per_drop) {
  Pet p = newPet(kDay0);
  LowAlerts a{};
  CHECK(newLowStats(a, p) == 0);
  p.oil = 19;
  CHECK(newLowStats(a, p) == 1);
  CHECK(newLowStats(a, p) == 0);
  p.oil = 50;
  CHECK(newLowStats(a, p) == 0);
  p.oil = 10;
  p.cond = 5;
  CHECK(newLowStats(a, p) == (1 | 8));
}

TEST(clock_set_keeps_date_when_valid) {
  const uint32_t now = kDay0 + 3 * 86400 + 10 * 3600 + 5 * 60 + 30;
  const uint32_t t = clockSetTarget(now, true, 22 * 60 + 15);
  const DateTime d = fromEpoch(t);
  CHECK(d.year == 2026 && d.month == 1 && d.day == 8);
  CHECK(d.hour == 22 && d.minute == 15 && d.second == 0);
}

TEST(clock_set_uses_default_date_when_invalid) {
  const DateTime d = fromEpoch(clockSetTarget(12345, false, 8 * 60));
  CHECK(d.year == 2026 && d.month == 1 && d.day == 1 && d.hour == 8 && d.minute == 0);
}

TEST(rebase_keeps_age_and_stats) {
  Pet p = newPet(1000);
  p.lastUpdate = 1000 + 5 * 86400;
  p.zeroSince = 1000 + 4 * 86400;
  p.oil = 42;
  rebaseTime(p, kDay0);
  CHECK(p.lastUpdate == kDay0);
  CHECK(p.bornAt == kDay0 - 5 * 86400);
  CHECK(p.zeroSince == kDay0 - 86400);
  CHECK_NEAR(p.oil, 42, 0.0001);
  advance(p, kDay0 + 60);
  CHECK(!p.shutdown);
}

TEST(rebase_backwards) {
  Pet p = newPet(kDay0 + 10 * 86400);
  rebaseTime(p, kDay0);
  CHECK(p.bornAt == kDay0);
  CHECK(p.zeroSince == 0);
}

TEST(stage_change_detected) {
  CHECK(evolved(Stage::Core, Stage::Mini));
  CHECK(!evolved(Stage::Mini, Stage::Mini));
}

RUN_TESTS()
