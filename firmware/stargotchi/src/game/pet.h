#pragma once
// Pure game logic for one StarGotchi bot. No Arduino dependencies.
// All timestamps are local wall-clock seconds since 1970 (the RTC holds local time).
#include <stdint.h>

namespace sg {

enum class Stage : uint8_t { Core, Mini, Junior, Astro, Combat, Scrap };

struct Pet {
  float oil, mood, power, cond;  // 0..100
  Stage stage;
  uint32_t bornAt, lastUpdate;
  uint32_t xp;
  bool docked;
  bool shutdown;
  uint32_t zeroSince;  // 0 = fewer than two stats at 0
  float careSum;
  uint32_t careSamples, playCount, actionCount;
};

Pet newPet(uint32_t now);
bool isNight(uint32_t localEpoch);           // 22:00 .. 06:59
bool isAsleep(const Pet &p, uint32_t now);   // night or docked
void advance(Pet &p, uint32_t now);          // catch up in 60 s steps (max 30 days)
void feed(Pet &p);                           // oil +30, xp +10
void repair(Pet &p);                         // cond +40, xp +10
void toggleDock(Pet &p);
void recordPlay(Pet &p);                     // mini-game played
bool rebootPet(Pet &p, uint32_t now);        // only when shut down: stats 50, xp halved
uint16_t level(const Pet &p);                // 1 + xp / 100
const char *statusText(const Pet &p, uint32_t now);

}  // namespace sg
