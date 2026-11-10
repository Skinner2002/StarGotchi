#include <cmath>

#include "check.h"
#include "game/asteroid.h"
#include "game/pet.h"

using namespace sg;

static float towardNearest(const AsteroidGame &g) {
  float best = 1e9f, target = g.shipX();
  for (int i = 0; i < g.rockCount(); i++) {
    const Rock &r = g.rocks()[i];
    if (r.active && r.y < AsteroidGame::kShipY && AsteroidGame::kShipY - r.y < best) {
      best = AsteroidGame::kShipY - r.y;
      target = r.x;
    }
  }
  const float d = target - g.shipX();
  return d > 4 ? 1.f : (d < -4 ? -1.f : 0.f);
}

static float awayFromNearest(const AsteroidGame &g) { return -towardNearest(g); }

TEST(starts_centered) {
  AsteroidGame g;
  g.begin(1);
  CHECK_NEAR(g.shipX(), 233, 0.01);
  CHECK(g.lives() == 3);
  CHECK(g.score() == 0);
  CHECK(!g.over());
}

TEST(deterministic_with_seed) {
  AsteroidGame a, b;
  a.begin(7);
  b.begin(7);
  for (int i = 0; i < 600; i++) {
    a.step(1 / 30.f, std::sin(i * 0.05f));
    b.step(1 / 30.f, std::sin(i * 0.05f));
  }
  CHECK(a.score() == b.score());
  CHECK(a.lives() == b.lives());
  CHECK_NEAR(a.rocks()[3].y, b.rocks()[3].y, 0.0001);
}

TEST(ship_clamped) {
  AsteroidGame g;
  g.begin(3);
  for (int i = 0; i < 60; i++) g.step(1 / 30.f, 1.f);
  CHECK_NEAR(g.shipX(), AsteroidGame::kMaxX, 0.01);
  for (int i = 0; i < 120; i++) g.step(1 / 30.f, -1.f);
  CHECK_NEAR(g.shipX(), AsteroidGame::kMinX, 0.01);
}

TEST(hit_costs_life_and_ends_at_zero) {
  AsteroidGame g;
  g.begin(11);
  int prevLives = 3;
  bool sawSingleHit = false;
  while (!g.over()) {
    g.step(1 / 30.f, towardNearest(g));
    if (g.lives() == prevLives - 1) sawSingleHit = true;
    prevLives = g.lives();
  }
  CHECK(sawSingleHit);
  CHECK(g.lives() == 0);
  CHECK(g.elapsed() < AsteroidGame::kDuration);
}

TEST(ends_after_45s) {
  AsteroidGame g;
  g.begin(5);
  int steps = 0;
  while (!g.over() && steps < 10000) {
    g.step(1 / 30.f, awayFromNearest(g));
    steps++;
  }
  CHECK(g.over());
  CHECK(g.lives() == 0 || g.elapsed() >= AsteroidGame::kDuration - 0.001f);
  CHECK(g.elapsed() <= AsteroidGame::kDuration + 0.05f);
  CHECK(g.score() > 0);
}

TEST(no_change_after_over) {
  AsteroidGame g;
  g.begin(5);
  while (!g.over()) g.step(0.1f, 0.f);
  const uint16_t s = g.score();
  const float x = g.shipX();
  g.step(1.f, 1.f);
  CHECK(g.score() == s);
  CHECK_NEAR(g.shipX(), x, 0.0001);
}

TEST(result_applies_to_pet) {
  Pet p = newPet(1767571200u + 12 * 3600);
  p.mood = 50;
  applyGameResult(p, 100);
  CHECK_NEAR(p.mood, 90, 0.001);
  CHECK_NEAR(p.power, 70, 0.001);
  CHECK(p.xp == 20);
  CHECK(p.playCount == 1);
  CHECK(p.actionCount == 1);
  applyGameResult(p, 10);
  CHECK_NEAR(p.mood, 95, 0.001);
}

TEST(result_ignored_when_shutdown) {
  Pet p = newPet(1767571200u + 12 * 3600);
  p.shutdown = true;
  applyGameResult(p, 100);
  CHECK(p.xp == 0);
  CHECK(p.playCount == 0);
}

RUN_TESTS()
