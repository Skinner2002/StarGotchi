#pragma once
// Display power state machine: On -> Dim after 30 s idle -> Off after 60 s.
// A touch while Dim/Off only wakes the screen (it is consumed).
#include <stdint.h>

namespace sg {

class ScreenPower {
 public:
  enum Mode : uint8_t { On, Dim, Off };
  static constexpr uint32_t kDimMs = 30000;
  static constexpr uint32_t kOffMs = 60000;

  bool onTouch(uint32_t ms);  // true = consumed (screen was Dim/Off)
  void wake(uint32_t ms);     // e.g. an alert: turn on without a touch
  Mode update(uint32_t ms);
  Mode mode() const { return mode_; }

 private:
  Mode mode_ = On;
  uint32_t lastActivity_ = 0;
};

}  // namespace sg
