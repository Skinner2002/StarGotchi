#include "save.h"

#include <string.h>

namespace sg {

namespace {

constexpr uint8_t kVersion = 1;
// magic(3) + version(1) + 4 floats + stage(1) + bornAt + lastUpdate + xp +
// docked(1) + shutdown(1) + zeroSince + careSum + 3 counters + crc
constexpr size_t kSize = 4 + 16 + 1 + 4 + 4 + 4 + 1 + 1 + 4 + 4 + 12 + 4;

struct Writer {
  uint8_t *p;
  void u8(uint8_t v) { *p++ = v; }
  void u32(uint32_t v) { for (int i = 0; i < 4; i++) *p++ = (v >> (8 * i)) & 0xFF; }
  void f32(float v) { uint32_t u; memcpy(&u, &v, 4); u32(u); }
};

struct Reader {
  const uint8_t *p;
  uint8_t u8() { return *p++; }
  uint32_t u32() { uint32_t v = 0; for (int i = 0; i < 4; i++) v |= uint32_t(*p++) << (8 * i); return v; }
  float f32() { uint32_t u = u32(); float v; memcpy(&v, &u, 4); return v; }
};

}  // namespace

uint32_t crc32(const uint8_t *data, size_t len) {
  uint32_t crc = 0xFFFFFFFFu;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
  }
  return ~crc;
}

size_t encodePet(const Pet &p, uint8_t *buf, size_t cap) {
  if (cap < kSize) return 0;
  Writer w{buf};
  w.u8('S'); w.u8('G'); w.u8('P'); w.u8(kVersion);
  w.f32(p.oil); w.f32(p.mood); w.f32(p.power); w.f32(p.cond);
  w.u8(static_cast<uint8_t>(p.stage));
  w.u32(p.bornAt); w.u32(p.lastUpdate); w.u32(p.xp);
  w.u8(p.docked); w.u8(p.shutdown);
  w.u32(p.zeroSince); w.f32(p.careSum);
  w.u32(p.careSamples); w.u32(p.playCount); w.u32(p.actionCount);
  w.u32(crc32(buf, kSize - 4));
  return kSize;
}

bool decodePet(const uint8_t *buf, size_t len, Pet &out) {
  if (len != kSize || buf[0] != 'S' || buf[1] != 'G' || buf[2] != 'P' || buf[3] != kVersion)
    return false;
  Reader c{buf + kSize - 4};
  if (c.u32() != crc32(buf, kSize - 4)) return false;
  Reader r{buf + 4};
  Pet p{};
  p.oil = r.f32(); p.mood = r.f32(); p.power = r.f32(); p.cond = r.f32();
  const uint8_t stage = r.u8();
  if (stage > static_cast<uint8_t>(Stage::Scrap)) return false;
  p.stage = static_cast<Stage>(stage);
  p.bornAt = r.u32(); p.lastUpdate = r.u32(); p.xp = r.u32();
  p.docked = r.u8() != 0; p.shutdown = r.u8() != 0;
  p.zeroSince = r.u32(); p.careSum = r.f32();
  p.careSamples = r.u32(); p.playCount = r.u32(); p.actionCount = r.u32();
  out = p;
  return true;
}

}  // namespace sg
