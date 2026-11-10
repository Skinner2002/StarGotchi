#pragma once
#include <stddef.h>
#include <stdint.h>

namespace sg {
namespace hal {

// Raw blob storage in NVS (namespace "stargotchi", key "pet").
bool nvsLoad(uint8_t *buf, size_t cap, size_t &len);
bool nvsSave(const uint8_t *buf, size_t len);
// Small settings (brightness, sound) in the same namespace.
uint8_t nvsGetU8(const char *key, uint8_t def);
void nvsSetU8(const char *key, uint8_t value);

}  // namespace hal
}  // namespace sg
