#include "events.h"

#include "clock.h"

namespace sg {

uint8_t newLowStats(LowAlerts &a, const Pet &p) {
  const float v[4] = {p.oil, p.mood, p.power, p.cond};
  uint8_t mask = 0;
  for (int i = 0; i < 4; i++) {
    const bool low = v[i] < 20.f;
    if (low && !a.low[i]) mask |= 1 << i;
    a.low[i] = low;
  }
  return mask;
}

uint32_t clockSetTarget(uint32_t now, bool nowValid, uint32_t minutesOfDay) {
  DateTime d = nowValid ? fromEpoch(now) : DateTime{2026, 1, 1, 0, 0, 0};
  d.hour = static_cast<int>(minutesOfDay / 60 % 24);
  d.minute = static_cast<int>(minutesOfDay % 60);
  d.second = 0;
  return toEpoch(d);
}

void rebaseTime(Pet &p, uint32_t newNow) {
  auto shift = [&](uint32_t t) -> uint32_t {
    const int64_t v = static_cast<int64_t>(t) + (static_cast<int64_t>(newNow) - p.lastUpdate);
    return v < 0 ? 0u : static_cast<uint32_t>(v);
  };
  p.bornAt = shift(p.bornAt);
  if (p.zeroSince) p.zeroSince = shift(p.zeroSince);
  p.lastUpdate = newNow;
}

static uint32_t absDiff(uint32_t x, uint32_t y) { return x > y ? x - y : y - x; }

bool acceptClock(uint32_t last, uint32_t a, uint32_t b) {
  if (!isValidTime(fromEpoch(a))) return false;
  if (last == 0 || absDiff(a, last) <= 120) return true;
  return absDiff(a, b) <= 2;
}

}  // namespace sg
