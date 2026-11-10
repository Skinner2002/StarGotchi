#include "pet.h"

namespace sg {

namespace {

constexpr uint32_t kStep = 60;
constexpr uint32_t kMaxCatchUp = 30u * 86400u;
constexpr uint32_t kShutdownAfter = 8u * 3600u;
constexpr uint32_t kDay = 86400u;

// Per-hour rates.
struct Rates { float oil, mood, power, cond; };
constexpr Rates kAwake{-4.f, -5.f, -3.f, -2.f};
constexpr Rates kAsleep{-2.f, -2.5f, 10.f, -1.f};

float clamp100(float v) { return v < 0.f ? 0.f : (v > 100.f ? 100.f : v); }

bool isZero(float v) { return v <= 0.0001f; }

void updateShutdown(Pet &p, uint32_t t, uint32_t end) {
  const int zeros = isZero(p.oil) + isZero(p.mood) + isZero(p.power) + isZero(p.cond);
  if (zeros < 2) {
    p.zeroSince = 0;
    return;
  }
  if (p.zeroSince == 0) p.zeroSince = t;
  if (end - p.zeroSince >= kShutdownAfter) p.shutdown = true;
}

Stage finalForm(const Pet &p) {
  const float avg = p.careSamples ? p.careSum / p.careSamples : 0.f;
  if (avg < 40.f) return Stage::Scrap;
  if (p.playCount > 0 && p.playCount * 10 >= p.actionCount * 4) return Stage::Combat;
  return Stage::Astro;
}

void updateStage(Pet &p, uint32_t end) {
  const uint32_t age = end - p.bornAt;
  if (p.stage == Stage::Core && age >= 1 * kDay) p.stage = Stage::Mini;
  if (p.stage == Stage::Mini && age >= 3 * kDay) p.stage = Stage::Junior;
  if (p.stage == Stage::Junior && age >= 7 * kDay) p.stage = finalForm(p);
}

void applyStep(Pet &p, uint32_t t, uint32_t secs) {
  if (!p.shutdown) updateShutdown(p, t, t);
  const Rates &r = isAsleep(p, t) ? kAsleep : kAwake;
  const float h = secs / 3600.f;
  p.oil = clamp100(p.oil + r.oil * h);
  p.mood = clamp100(p.mood + r.mood * h);
  p.power = clamp100(p.power + r.power * h);
  p.cond = clamp100(p.cond + r.cond * h);
  p.careSum += (p.oil + p.mood + p.power + p.cond) / 4.f;
  p.careSamples++;
  if (p.docked && p.power >= 99.95f && !isNight(t + secs)) p.docked = false;
  if (!p.shutdown) updateShutdown(p, t, t + secs);
  updateStage(p, t + secs);
}

void giveXp(Pet &p) {
  p.xp += 10;
  p.actionCount++;
}

}  // namespace

Pet newPet(uint32_t now) {
  Pet p{};
  p.oil = p.mood = p.power = p.cond = 80.f;
  p.stage = Stage::Core;
  p.bornAt = p.lastUpdate = now;
  return p;
}

bool isNight(uint32_t t) {
  const uint32_t hour = (t % 86400u) / 3600u;
  return hour >= 22 || hour < 7;
}

bool isAsleep(const Pet &p, uint32_t now) { return p.docked || isNight(now); }

void advance(Pet &p, uint32_t now) {
  if (now <= p.lastUpdate) {
    p.lastUpdate = now;
    return;
  }
  uint32_t gap = now - p.lastUpdate;
  if (gap > kMaxCatchUp) gap = kMaxCatchUp;
  uint32_t t = now - gap;
  while (gap > 0) {
    const uint32_t s = gap < kStep ? gap : kStep;
    applyStep(p, t, s);
    t += s;
    gap -= s;
  }
  p.lastUpdate = now;
}

void feed(Pet &p) {
  if (p.shutdown) return;
  p.oil = clamp100(p.oil + 30.f);
  giveXp(p);
}

void repair(Pet &p) {
  if (p.shutdown) return;
  p.cond = clamp100(p.cond + 40.f);
  giveXp(p);
}

void toggleDock(Pet &p) {
  if (p.shutdown) return;
  p.docked = !p.docked;
}

void recordPlay(Pet &p) {
  p.playCount++;
  p.actionCount++;
}

bool rebootPet(Pet &p, uint32_t now) {
  if (!p.shutdown) return false;
  p.oil = p.mood = p.power = p.cond = 50.f;
  p.xp /= 2;
  p.shutdown = false;
  p.zeroSince = 0;
  p.docked = false;
  p.lastUpdate = now;
  return true;
}

uint16_t level(const Pet &p) { return static_cast<uint16_t>(1 + p.xp / 100); }

const char *statusText(const Pet &p, uint32_t now) {
  if (p.shutdown) return "Emergency shutdown";
  if (isAsleep(p, now)) return "Recharging...";
  float low = p.oil;
  const char *msg = "Low oil!";
  if (p.mood < low) { low = p.mood; msg = "Bored!"; }
  if (p.power < low) { low = p.power; msg = "Low power!"; }
  if (p.cond < low) { low = p.cond; msg = "Needs repair!"; }
  return low < 20.f ? msg : "Systems nominal";
}

}  // namespace sg
