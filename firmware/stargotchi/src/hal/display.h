#pragma once
#include <stdint.h>

namespace sg {
namespace hal {

// Initialises the CO5300 AMOLED, LVGL, its draw buffers and the 2 ms tick.
bool displayBegin();

// 0 = off, 255 = full.
void displaySetBrightness(uint8_t level);

// Panel sleep (true) / wake (false); used when the screen times out.
void displaySleep(bool sleep);

}  // namespace hal
}  // namespace sg
