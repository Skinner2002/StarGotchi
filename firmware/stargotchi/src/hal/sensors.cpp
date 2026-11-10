// RTC, IMU and power management.
#include <Arduino.h>
#include <SensorPCF85063.hpp>
#include <SensorQMI8658.hpp>
#include <Wire.h>
#define XPOWERS_CHIP_AXP2101
#include <XPowersLib.h>

#include "../game/clock.h"
#include "hal.h"
#include "pins.h"

namespace sg {
namespace hal {

namespace {
SensorPCF85063 rtc;
SensorQMI8658 qmi;
XPowersPMU pmu;
bool rtcOk = false, imuOk = false, pmuOk = false;
float tiltFiltered = 0.f;
}  // namespace

void i2cBegin() { Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL); }

// ---- RTC ----
bool rtcBegin() {
  rtcOk = rtc.begin(Wire, PIN_I2C_SDA, PIN_I2C_SCL);
  return rtcOk;
}

static DateTime rtcRead() {
  RTC_DateTime d = rtc.getDateTime();
  return DateTime{d.getYear(), d.getMonth(), d.getDay(), d.getHour(), d.getMinute(), d.getSecond()};
}

bool rtcValid() { return rtcOk && isValidTime(rtcRead()); }

uint32_t rtcNow() {
  if (!rtcOk) return 0;
  return toEpoch(rtcRead());
}

void rtcSet(uint32_t t) {
  if (!rtcOk) return;
  const DateTime d = fromEpoch(t);
  rtc.setDateTime(d.year, d.month, d.day, d.hour, d.minute, d.second);
}

// ---- IMU ----
bool imuBegin() {
  imuOk = qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
  if (!imuOk) return false;
  qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_250Hz,
                          SensorQMI8658::LPF_MODE_0);
  qmi.enableAccelerometer();
  return true;
}

// Sign/axis chosen for the board held with USB-C at the bottom; flip
// SG_TILT_SIGN if left/right are swapped on hardware.
#ifndef SG_TILT_SIGN
#define SG_TILT_SIGN (-1.f)
#endif

float imuTiltX() {
  if (!imuOk) return 0.f;
  float x, y, z;
  if (qmi.getDataReady() && qmi.getAccelerometer(x, y, z)) {
    float v = SG_TILT_SIGN * x;  // in g
    tiltFiltered = tiltFiltered * 0.6f + v * 0.4f;
  }
  float t = tiltFiltered * 2.f;  // full deflection at ~30 degrees
  if (t > 1.f) t = 1.f;
  if (t < -1.f) t = -1.f;
  return t;
}

// ---- Power ----
bool powerBegin() {
  pmuOk = pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, PIN_I2C_SDA, PIN_I2C_SCL);
  if (!pmuOk) return false;
  pmu.enableBattDetection();
  pmu.enableBattVoltageMeasure();
  pmu.enableVbusVoltageMeasure();
  return true;
}

int batteryPercent() {
  if (!pmuOk || !pmu.isBatteryConnect()) return -1;
  return pmu.getBatteryPercent();
}

bool isCharging() { return pmuOk && pmu.isCharging(); }

}  // namespace hal
}  // namespace sg
