#include "check.h"
#include "game/pet.h"

using namespace sg;

// 2026-01-05 00:00:00 local (a Monday), seconds since 1970.
static const uint32_t kDay0 = 1767571200u;
static uint32_t at(int day, int hour, int min = 0) {
  return kDay0 + day * 86400u + hour * 3600u + min * 60u;
}

TEST(awake_decay_one_hour) {
  Pet p = newPet(at(0, 12));
  advance(p, at(0, 13));
  CHECK_NEAR(p.oil, 76, 0.01);
  CHECK_NEAR(p.mood, 75, 0.01);
  CHECK_NEAR(p.power, 77, 0.01);
  CHECK_NEAR(p.cond, 78, 0.01);
}

TEST(docked_charges) {
  Pet p = newPet(at(0, 12));
  toggleDock(p);
  advance(p, at(0, 13));
  CHECK_NEAR(p.power, 90, 0.01);
  CHECK_NEAR(p.oil, 78, 0.01);
  CHECK_NEAR(p.mood, 77.5, 0.01);
  CHECK_NEAR(p.cond, 79, 0.01);
}

TEST(offline_overnight_uses_sleep_rates) {
  Pet p = newPet(at(0, 21));
  advance(p, at(1, 8));
  CHECK_NEAR(p.oil, 80 - 4 - 2 * 9 - 4, 0.1);
}

TEST(is_night_window) {
  CHECK(!isNight(at(0, 21, 59)));
  CHECK(isNight(at(0, 22)));
  CHECK(isNight(at(0, 3)));
  CHECK(isNight(at(0, 6, 59)));
  CHECK(!isNight(at(0, 7)));
}

TEST(time_backwards_is_ignored) {
  Pet p = newPet(at(0, 12));
  advance(p, at(0, 11));
  CHECK_NEAR(p.oil, 80, 0.001);
  CHECK_NEAR(p.power, 80, 0.001);
  CHECK(p.lastUpdate == at(0, 11));
  advance(p, at(0, 12));
  CHECK_NEAR(p.oil, 76, 0.01);
}

TEST(stats_clamp) {
  Pet p = newPet(at(0, 8));
  advance(p, at(2, 10));
  CHECK(p.oil >= 0 && p.mood >= 0 && p.power >= 0 && p.cond >= 0);
  Pet q = newPet(at(0, 12));
  q.oil = 90;
  feed(q);
  CHECK_NEAR(q.oil, 100, 0.001);
}

TEST(feed_repair_give_xp) {
  Pet p = newPet(at(0, 12));
  feed(p);
  repair(p);
  CHECK(p.xp == 20);
  CHECK(p.actionCount == 2);
  CHECK(level(p) == 1);
  for (int i = 0; i < 8; i++) feed(p);
  CHECK(level(p) == 2);
}

TEST(repair_adds_40) {
  Pet p = newPet(at(0, 12));
  p.cond = 10;
  repair(p);
  CHECK_NEAR(p.cond, 50, 0.001);
}

TEST(docked_wakes_full_at_day) {
  Pet p = newPet(at(0, 9));
  toggleDock(p);
  advance(p, at(0, 11));
  CHECK(p.power >= 99.99);
  CHECK(!p.docked);
  advance(p, at(0, 12));
  CHECK_NEAR(p.power, 97, 0.05);
}

TEST(actions_blocked_when_shutdown) {
  Pet p = newPet(at(0, 12));
  p.shutdown = true;
  p.oil = 10;
  feed(p);
  repair(p);
  toggleDock(p);
  CHECK_NEAR(p.oil, 10, 0.001);
  CHECK(p.xp == 0);
  CHECK(!p.docked);
}

TEST(status_text) {
  Pet p = newPet(at(0, 12));
  CHECK(std::string(statusText(p, at(0, 12))) == "Systems nominal");
  p.oil = 10;
  CHECK(std::string(statusText(p, at(0, 12))) == "Low oil!");
  CHECK(std::string(statusText(p, at(0, 23))) == "Recharging...");
  p.shutdown = true;
  CHECK(std::string(statusText(p, at(0, 12))) == "Emergency shutdown");
}

RUN_TESTS()
