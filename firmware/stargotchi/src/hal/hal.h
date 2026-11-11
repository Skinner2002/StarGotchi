#pragma once
// Thin wrappers around the board peripherals. Device-only (Arduino).
#include <stdint.h>

namespace sg {
namespace hal {

// Shared I2C bus (SDA 15, SCL 14). Call once before the others.
void i2cBegin();

// Touch (CST9217). Registers an LVGL pointer input device.
// The gate is asked on every new press; returning true swallows that press.
bool touchBegin();
void touchSetGate(bool (*gate)(uint32_t ms));

// Real-time clock (PCF85063), local wall-clock seconds since 1970.
bool rtcBegin();
uint32_t rtcNow();
void rtcSet(uint32_t localEpoch);
bool rtcValid();

// Motion sensor (QMI8658). Sideways tilt, -1 (left) .. +1 (right).
bool imuBegin();
float imuTiltX();

// Power management (AXP2101).
bool powerBegin();
int batteryPercent();  // -1 if no battery
bool isCharging();

// Speaker (ES8311). Non-blocking: tones are queued and played by a task.
bool audioBegin();
void beep(uint16_t hz, uint16_t ms);
void setVolume(uint8_t percent);  // 0 = muted, 1..100

}  // namespace hal
}  // namespace sg
