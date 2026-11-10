#include "asteroid.h"

namespace sg {

void AsteroidGame::begin(uint32_t seed) {
  rng_ = seed ? seed : 1;
  shipX_ = 233.f;
  elapsed_ = 0.f;
  spawnIn_ = 0.5f;
  lives_ = kLives;
  score_ = 0;
  for (Rock &r : rocks_) r = Rock{0, 0, 0, false};
}

float AsteroidGame::rand01() {
  rng_ ^= rng_ << 13;
  rng_ ^= rng_ >> 17;
  rng_ ^= rng_ << 5;
  return (rng_ & 0xFFFFFF) / float(0x1000000);
}

void AsteroidGame::spawn() {
  for (Rock &r : rocks_) {
    if (r.active) continue;
    r.x = kMinX + rand01() * (kMaxX - kMinX);
    r.y = kSpawnY;
    r.speed = 120.f + rand01() * 100.f + elapsed_ * 2.f;
    r.active = true;
    return;
  }
}

void AsteroidGame::step(float dt, float tilt) {
  if (over()) return;
  elapsed_ += dt;
  if (elapsed_ > kDuration) elapsed_ = kDuration;

  shipX_ += tilt * kShipSpeed * dt;
  if (shipX_ < kMinX) shipX_ = kMinX;
  if (shipX_ > kMaxX) shipX_ = kMaxX;

  spawnIn_ -= dt;
  if (spawnIn_ <= 0.f) {
    spawn();
    const float interval = 0.9f - 0.5f * (elapsed_ / kDuration);
    spawnIn_ += interval;
  }

  for (Rock &r : rocks_) {
    if (!r.active) continue;
    r.y += r.speed * dt;
    const float dx = r.x - shipX_, dy = r.y - kShipY;
    if (dx > -kHitDist && dx < kHitDist && dy > -kHitDist && dy < kHitDist) {
      r.active = false;
      lives_--;
    } else if (r.y > kGoneY) {
      r.active = false;
      score_++;
    }
  }
}

void applyGameResult(Pet &p, uint16_t score) {
  if (p.shutdown) return;
  float gain = score / 2.f;
  if (gain > 40.f) gain = 40.f;
  p.mood += gain;
  if (p.mood > 100.f) p.mood = 100.f;
  p.power -= 10.f;
  if (p.power < 0.f) p.power = 0.f;
  p.xp += score / 5;
  recordPlay(p);
}

}  // namespace sg
