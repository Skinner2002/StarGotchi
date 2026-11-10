#pragma once
// "Asteroid Run" mini-game simulation. Pure logic, deterministic per seed.
// Coordinates are screen pixels on the 466x466 round display.
#include <stdint.h>

#include "pet.h"

namespace sg {

struct Rock {
  float x, y, speed;
  bool active;
};

class AsteroidGame {
 public:
  static constexpr float kDuration = 45.f;
  static constexpr int kLives = 3;
  static constexpr int kMaxRocks = 8;
  static constexpr float kMinX = 93.f, kMaxX = 373.f;
  static constexpr float kShipY = 360.f, kSpawnY = 60.f, kGoneY = 420.f;
  static constexpr float kShipSpeed = 300.f;  // px/s at full tilt
  static constexpr float kHitDist = 18.f;

  void begin(uint32_t seed);
  void step(float dtSec, float tiltX);  // tiltX -1..1
  bool over() const { return lives_ <= 0 || elapsed_ >= kDuration; }
  uint16_t score() const { return score_; }
  int lives() const { return lives_; }
  float elapsed() const { return elapsed_; }
  float shipX() const { return shipX_; }
  const Rock *rocks() const { return rocks_; }
  int rockCount() const { return kMaxRocks; }

 private:
  float rand01();
  void spawn();

  Rock rocks_[kMaxRocks];
  uint32_t rng_ = 1;
  float shipX_ = 233.f, elapsed_ = 0.f, spawnIn_ = 0.f;
  int lives_ = kLives;
  uint16_t score_ = 0;
};

// mood += min(score/2, 40); power -= 10; xp += score/5; counts as a play.
void applyGameResult(Pet &p, uint16_t score);

}  // namespace sg
