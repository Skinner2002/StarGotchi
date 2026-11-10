#include "clock.h"

namespace sg {

// Howard Hinnant's days_from_civil / civil_from_days.
static int64_t daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

uint32_t toEpoch(const DateTime &d) {
  const int64_t days = daysFromCivil(d.year, d.month, d.day);
  return static_cast<uint32_t>(days * 86400 + d.hour * 3600 + d.minute * 60 + d.second);
}

DateTime fromEpoch(uint32_t t) {
  int64_t z = t / 86400 + 719468;
  const uint32_t secs = t % 86400;
  const int64_t era = z / 146097;
  const unsigned doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  const unsigned d = doy - (153 * mp + 2) / 5 + 1;
  const unsigned m = mp < 10 ? mp + 3 : mp - 9;
  const int y = static_cast<int>(yoe + era * 400) + (m <= 2);
  return DateTime{y, static_cast<int>(m), static_cast<int>(d), static_cast<int>(secs / 3600),
                  static_cast<int>(secs % 3600 / 60), static_cast<int>(secs % 60)};
}

bool isValidTime(const DateTime &d) {
  return d.year >= 2025 && d.year <= 2099 && d.month >= 1 && d.month <= 12 && d.day >= 1 &&
         d.day <= 31 && d.hour >= 0 && d.hour < 24 && d.minute >= 0 && d.minute < 60 &&
         d.second >= 0 && d.second < 60;
}

}  // namespace sg
