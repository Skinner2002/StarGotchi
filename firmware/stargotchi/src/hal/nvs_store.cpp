#include "nvs_store.h"

#include <Preferences.h>

namespace sg {
namespace hal {

namespace {
constexpr const char *kNs = "stargotchi";
}

bool nvsLoad(uint8_t *buf, size_t cap, size_t &len) {
  Preferences prefs;
  if (!prefs.begin(kNs, true)) return false;
  len = prefs.getBytesLength("pet");
  bool ok = len > 0 && len <= cap && prefs.getBytes("pet", buf, len) == len;
  prefs.end();
  return ok;
}

bool nvsSave(const uint8_t *buf, size_t len) {
  Preferences prefs;
  if (!prefs.begin(kNs, false)) return false;
  bool ok = prefs.putBytes("pet", buf, len) == len;
  prefs.end();
  return ok;
}

uint8_t nvsGetU8(const char *key, uint8_t def) {
  Preferences prefs;
  if (!prefs.begin(kNs, true)) return def;
  uint8_t v = prefs.getUChar(key, def);
  prefs.end();
  return v;
}

void nvsSetU8(const char *key, uint8_t value) {
  Preferences prefs;
  if (!prefs.begin(kNs, false)) return;
  prefs.putUChar(key, value);
  prefs.end();
}

}  // namespace hal
}  // namespace sg
