#pragma once
// Small pure helpers the app loop uses around the pet.
#include <stdint.h>

#include "pet.h"

namespace sg {

struct LowAlerts { bool low[4]; };  // oil, mood, power, cond

// Bitmask (1 oil, 2 mood, 4 power, 8 cond) of stats that just dropped below 20.
uint8_t newLowStats(LowAlerts &a, const Pet &p);

// Epoch for "set clock to minutesOfDay": keeps today's date when the clock is
// valid, otherwise uses 2026-01-01.
uint32_t clockSetTarget(uint32_t now, bool nowValid, uint32_t minutesOfDay);

// Moves all pet timestamps so that lastUpdate == newNow (age kept, no decay
// for the unknown gap). Used when the clock was not valid.
void rebaseTime(Pet &p, uint32_t newNow);

inline bool evolved(Stage before, Stage after) { return before != after; }

}  // namespace sg
