#pragma once
// Versioned, CRC-protected binary save format for a Pet. Pure C++.
#include <stddef.h>
#include <stdint.h>

#include "../game/pet.h"

namespace sg {

constexpr size_t kSaveMaxSize = 64;

// Layout: "SGP", version (1), fields little-endian in Pet order, CRC32 (IEEE).
// Returns bytes written, or 0 if cap is too small.
size_t encodePet(const Pet &p, uint8_t *buf, size_t cap);

// Returns false (and leaves out untouched) on bad magic, version, size or CRC.
bool decodePet(const uint8_t *buf, size_t len, Pet &out);

uint32_t crc32(const uint8_t *data, size_t len);

}  // namespace sg
