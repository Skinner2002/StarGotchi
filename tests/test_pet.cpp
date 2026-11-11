#include <ctime>
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

static void careHourly(Pet &p, uint32_t from, int hours) {
  for (int h = 1; h <= hours; h++) {
    p.oil = p.mood = p.power = p.cond = 90;
    advance(p, from + h * 3600u);
  }
}

TEST(evolves_on_age) {
  Pet p = newPet(at(0, 12));
  careHourly(p, at(0, 12), 23);
  CHECK(p.stage == Stage::Core);
  careHourly(p, at(0, 12) + 23 * 3600u, 1);
  CHECK(p.stage == Stage::Mini);
  careHourly(p, at(1, 12), 48);
  CHECK(p.stage == Stage::Junior);
  careHourly(p, at(3, 12), 96);
  CHECK(p.stage == Stage::Astro);
}

TEST(neglect_gives_scrap) {
  Pet p = newPet(at(0, 12));
  for (int h = 1; h <= 7 * 24; h++) {
    advance(p, at(0, 12) + h * 3600u);
    if (p.shutdown) rebootPet(p, p.lastUpdate);
  }
  CHECK(p.stage == Stage::Scrap);
}

TEST(play_heavy_gives_combat) {
  Pet p = newPet(at(0, 12));
  for (int i = 0; i < 5; i++) recordPlay(p);
  for (int i = 0; i < 4; i++) feed(p);
  careHourly(p, at(0, 12), 7 * 24);
  CHECK(p.stage == Stage::Combat);
}

TEST(shutdown_after_8h_two_zero) {
  Pet p = newPet(at(0, 9));
  p.oil = p.mood = 0;
  p.power = p.cond = 100;
  advance(p, at(0, 17) - 60);
  CHECK(!p.shutdown);
  advance(p, at(0, 17));
  CHECK(p.shutdown);
}

TEST(one_zero_no_shutdown) {
  Pet p = newPet(at(0, 12));
  for (int h = 1; h <= 24; h++) {
    p.oil = 0;
    p.mood = p.power = p.cond = 90;
    advance(p, at(0, 12) + h * 3600u);
  }
  CHECK(!p.shutdown);
}

TEST(stat_recovery_resets_zero_timer) {
  Pet p = newPet(at(0, 9));
  p.oil = p.mood = 0;
  advance(p, at(0, 16));
  feed(p);
  advance(p, at(0, 17));
  CHECK(!p.shutdown);
  CHECK(p.zeroSince == 0);
}

TEST(month_offline_triggers_shutdown) {
  Pet p = newPet(at(0, 12));
  const clock_t c0 = clock();
  advance(p, at(30, 12));
  const double ms = 1000.0 * (clock() - c0) / CLOCKS_PER_SEC;
  CHECK(p.shutdown);
  CHECK(ms < 100);
  CHECK(p.lastUpdate == at(30, 12));
}

TEST(reboot_halves_xp) {
  Pet p = newPet(at(0, 12));
  p.xp = 300;
  p.stage = Stage::Junior;
  CHECK(!rebootPet(p, at(0, 12)));
  p.shutdown = true;
  p.zeroSince = at(0, 1);
  CHECK(rebootPet(p, at(0, 12)));
  CHECK(p.xp == 150);
  CHECK_NEAR(p.oil, 50, 0.001);
  CHECK_NEAR(p.cond, 50, 0.001);
  CHECK(p.stage == Stage::Junior);
  CHECK(!p.shutdown);
  CHECK(p.zeroSince == 0);
}

TEST(clock_set_backwards_no_instant_evolution) {
  Pet p = newPet(at(0, 15));
  advance(p, at(0, 15, 10));
  advance(p, at(0, 13));
  advance(p, at(0, 13) + 1);
  CHECK(p.stage == Stage::Core);
  CHECK(p.bornAt == at(0, 12, 50));
}

TEST(clock_set_backwards_no_instant_shutdown) {
  Pet p = newPet(at(0, 12));
  p.oil = p.mood = 0;
  advance(p, at(0, 12, 30));
  CHECK(p.zeroSince != 0);
  advance(p, at(0, 11, 30));
  advance(p, at(0, 11, 31));
  CHECK(!p.shutdown);
  advance(p, at(0, 18, 59));
  CHECK(!p.shutdown);
  advance(p, at(0, 19, 0));
  CHECK(p.shutdown);
}

TEST(new_pet_keeps_unit_line) {
  CHECK(newPet(at(0, 12), 2).line == 2);
  CHECK(newPet(at(0, 12), 0).line == 0);
  CHECK(newPet(at(0, 12), 7).line == 1);  // wrapped into 0..2
  CHECK(newPet(at(0, 12)).line == 0);
}

TEST(reboot_keeps_unit_line) {
  Pet p = newPet(at(0, 12), 2);
  p.shutdown = true;
  rebootPet(p, at(0, 13));
  CHECK(p.line == 2);
}

RUN_TESTS()
