#include "screen_power.h"

namespace sg {

bool ScreenPower::onTouch(uint32_t ms) {
  const bool consumed = mode_ != On;
  wake(ms);
  return consumed;
}

void ScreenPower::wake(uint32_t ms) {
  mode_ = On;
  lastActivity_ = ms;
}

ScreenPower::Mode ScreenPower::update(uint32_t ms) {
  const uint32_t idle = ms > lastActivity_ ? ms - lastActivity_ : 0;
  if (idle >= kOffMs) mode_ = Off;
  else if (idle >= kDimMs) mode_ = Dim;
  else mode_ = On;
  return mode_;
}

}  // namespace sg
