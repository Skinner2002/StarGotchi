#include "check.h"
#include "game/pet.h"
#include "storage/save.h"

using namespace sg;

static Pet sample() {
  Pet p = newPet(1767571200u);
  p.oil = 12.5f; p.mood = 33.f; p.power = 99.f; p.cond = 0.25f;
  p.stage = Stage::Junior;
  p.lastUpdate = 1767600000u;
  p.xp = 1234; p.docked = true; p.shutdown = false;
  p.zeroSince = 1767590000u;
  p.careSum = 4567.75f; p.careSamples = 89; p.playCount = 7; p.actionCount = 21;
  p.line = 2;
  return p;
}

static bool same(const Pet &a, const Pet &b) {
  return a.oil == b.oil && a.mood == b.mood && a.power == b.power && a.cond == b.cond &&
         a.stage == b.stage && a.bornAt == b.bornAt && a.lastUpdate == b.lastUpdate &&
         a.xp == b.xp && a.docked == b.docked && a.shutdown == b.shutdown &&
         a.zeroSince == b.zeroSince && a.careSum == b.careSum &&
         a.careSamples == b.careSamples && a.playCount == b.playCount &&
         a.actionCount == b.actionCount && a.line == b.line;
}

TEST(roundtrip) {
  uint8_t buf[128];
  const Pet p = sample();
  const size_t n = encodePet(p, buf, sizeof buf);
  CHECK(n > 0);
  Pet q = newPet(0);
  CHECK(decodePet(buf, n, q));
  CHECK(same(p, q));
}

TEST(crc_detects_flip) {
  uint8_t buf[128];
  const size_t n = encodePet(sample(), buf, sizeof buf);
  for (size_t i = 0; i < n; i++) {
    buf[i] ^= 0x01;
    Pet q = newPet(42);
    const Pet before = q;
    CHECK(!decodePet(buf, n, q));
    CHECK(same(q, before));
    buf[i] ^= 0x01;
  }
}

TEST(wrong_version_rejected) {
  uint8_t buf[128];
  const size_t n = encodePet(sample(), buf, sizeof buf);
  buf[3] = 9;
  Pet q = newPet(0);
  CHECK(!decodePet(buf, n, q));
}

TEST(truncated_rejected) {
  uint8_t buf[128];
  const size_t n = encodePet(sample(), buf, sizeof buf);
  Pet q = newPet(0);
  CHECK(!decodePet(buf, n - 1, q));
  CHECK(!decodePet(buf, 0, q));
}

TEST(small_buffer_returns_0) {
  uint8_t buf[16];
  CHECK(encodePet(sample(), buf, sizeof buf) == 0);
}

TEST(version1_save_still_loads_as_line_0) {
  uint8_t buf[128];
  const size_t n = encodePet(sample(), buf, sizeof buf);
  // v1 layout = v2 without the trailing line byte (just before the CRC).
  uint8_t v1[128];
  const size_t m = n - 1;
  for (size_t i = 0; i < m - 4; i++) v1[i] = buf[i];
  v1[3] = 1;
  const uint32_t crc = crc32(v1, m - 4);
  for (int i = 0; i < 4; i++) v1[m - 4 + i] = (crc >> (8 * i)) & 0xFF;
  Pet q = newPet(0);
  CHECK(decodePet(v1, m, q));
  CHECK(q.line == 0);
  CHECK(q.xp == 1234);
  CHECK(q.stage == Stage::Junior);
}

TEST(bad_line_rejected) {
  uint8_t buf[128];
  const size_t n = encodePet(sample(), buf, sizeof buf);
  buf[n - 5] = 3;
  const uint32_t crc = crc32(buf, n - 4);
  for (int i = 0; i < 4; i++) buf[n - 4 + i] = (crc >> (8 * i)) & 0xFF;
  Pet q = newPet(0);
  CHECK(!decodePet(buf, n, q));
}

RUN_TESTS()
