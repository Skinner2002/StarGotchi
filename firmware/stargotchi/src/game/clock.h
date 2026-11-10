#pragma once
// Calendar <-> local epoch seconds (no time zones: the RTC holds local time).
#include <stdint.h>

namespace sg {

struct DateTime {
  int year, month, day, hour, minute, second;
};

uint32_t toEpoch(const DateTime &d);
DateTime fromEpoch(uint32_t t);
bool isValidTime(const DateTime &d);  // year 2025..2099 and fields in range

}  // namespace sg
