// Wires game logic, hardware and UI together.
#include "app.h"

#include <Arduino.h>
#include <lvgl.h>

#include "game/asteroid.h"
#include "game/events.h"
#include "game/pet.h"
#include "game/screen_power.h"
#include "hal/display.h"
#include "hal/hal.h"
#include "hal/nvs_store.h"
#include "storage/save.h"
#include "ui/minigame.h"
#include "ui/ui.h"

namespace sg {

namespace {

constexpr uint32_t kTickMs = 1000;
constexpr uint32_t kSaveMs = 60000;
constexpr uint32_t kBatteryMs = 10000;
constexpr uint8_t kDimLevel = 40;

Pet pet;
LowAlerts lowAlerts{};
ScreenPower screen;
ScreenPower::Mode appliedMode = ScreenPower::On;
uint8_t brightness = 200;
bool clockValid = false;
uint32_t lastTick = 0, lastSave = 0, lastBattery = 0;
uint32_t lastClock = 0;  // last accepted RTC reading
int battery = -1;
bool charging = false;

void save() {
  uint8_t buf[kSaveMaxSize];
  const size_t n = encodePet(pet, buf, sizeof buf);
  if (n) hal::nvsSave(buf, n);
  lastSave = millis();
}

void refreshUi(uint32_t now) { ui::refresh(pet, now, battery, charging); }

// Advances the pet to "now" and reacts to evolution / new low stats.
uint32_t tickPet() {
  uint32_t now = hal::rtcNow();
  if (!clockValid) return now;
  // Ignore a single garbage RTC reading instead of applying weeks of decay.
  if (!acceptClock(lastClock, now, hal::rtcNow())) return lastClock ? lastClock : now;
  lastClock = now;
  const Stage before = pet.stage;
  const bool wasShutdown = pet.shutdown;
  advance(pet, now);
  if (evolved(before, pet.stage)) {
    screen.wake(millis());
    ui::celebrate();
    hal::beep(523, 120);
    hal::beep(659, 120);
    hal::beep(784, 250);
    save();
  }
  if (!wasShutdown && pet.shutdown) {
    hal::beep(392, 200);
    hal::beep(196, 400);
    save();
  }
  if (newLowStats(lowAlerts, pet) && !isAsleep(pet, now)) hal::beep(220, 180);
  return now;
}

void applyScreenMode(ScreenPower::Mode m) {
  if (m == appliedMode) return;
  if (m == ScreenPower::Off) {
    hal::displaySleep(true);
    ui::setSleeping(true);
  } else if (appliedMode == ScreenPower::Off) {
    hal::displaySleep(false);
    ui::setSleeping(false);
  }
  appliedMode = m;
  hal::displaySetBrightness(m == ScreenPower::On ? brightness
                            : m == ScreenPower::Dim ? (brightness < kDimLevel ? brightness : kDimLevel)
                                                    : 0);
}

bool touchGate(uint32_t ms) {
  const bool consumed = screen.onTouch(ms);
  applyScreenMode(screen.mode());
  return consumed;
}

float tilt() { return hal::imuTiltX(); }

void gameFinished(uint16_t score) {
  tickPet();
  applyGameResult(pet, score);
  save();
  refreshUi(hal::rtcNow());
}

void onAction(ui::Action a, uint32_t arg) {
  const uint32_t now = tickPet();
  switch (a) {
    case ui::Action::Oil:
      feed(pet);
      hal::beep(660, 70);
      hal::beep(880, 90);
      break;
    case ui::Action::Fix:
      repair(pet);
      hal::beep(440, 80);
      hal::beep(880, 120);
      break;
    case ui::Action::Dock:
      toggleDock(pet);
      hal::beep(pet.docked ? 330 : 494, 150);
      break;
    case ui::Action::Play:
      if (pet.shutdown) return;
      hal::beep(587, 80);
      ui::minigameStart({tilt, hal::beep, gameFinished}, millis() ^ now);
      return;
    case ui::Action::Reboot:
      if (rebootPet(pet, now)) {
        lowAlerts = LowAlerts{};
        hal::beep(392, 100);
        hal::beep(523, 100);
        hal::beep(659, 200);
      }
      break;
    case ui::Action::SetTime: {
      const uint32_t target = clockSetTarget(now, clockValid, arg);
      // Always rebase: setting the clock is a correction, not time passing.
      rebaseTime(pet, target);
      hal::rtcSet(target);
      clockValid = hal::rtcValid();
      lastClock = clockValid ? target : 0;
      if (clockValid) {
        ui::showTimePrompt(false);
        ui::goHome();
      }
      hal::beep(880, 60);
      break;
    }
    case ui::Action::Brightness:
      brightness = static_cast<uint8_t>(arg);
      hal::nvsSetU8("bright", brightness);
      hal::displaySetBrightness(brightness);
      return;
    case ui::Action::Volume:
      hal::setVolume(static_cast<uint8_t>(arg));
      hal::nvsSetU8("vol", static_cast<uint8_t>(arg));
      hal::beep(660, 80);  // preview at the new volume (silent when 0)
      return;
    case ui::Action::Reset:
      pet = newPet(hal::rtcNow(), static_cast<uint8_t>(esp_random() % kLineCount));
      lowAlerts = LowAlerts{};
      ui::goHome();
      break;
  }
  save();
  refreshUi(hal::rtcNow());
}

}  // namespace

void appSetup() {
  Serial.begin(115200);
  if (!hal::displayBegin()) {
    Serial.println("StarGotchi: display init failed");
    for (;;) delay(1000);
  }
  hal::i2cBegin();
  hal::touchBegin();
  hal::rtcBegin();
  hal::imuBegin();
  hal::powerBegin();
  hal::audioBegin();

  brightness = hal::nvsGetU8("bright", 200);
  if (brightness < 20) brightness = 20;
  uint8_t volume = hal::nvsGetU8("vol", 255);
  if (volume > 100) volume = hal::nvsGetU8("sound", 1) ? 70 : 0;  // first start / old setting
  hal::setVolume(volume);
  hal::displaySetBrightness(brightness);

  clockValid = hal::rtcValid();
  const uint32_t now = hal::rtcNow();
  uint8_t buf[kSaveMaxSize];
  size_t len = 0;
  const bool loaded = hal::nvsLoad(buf, sizeof buf, len) && decodePet(buf, len, pet);
  if (!loaded) pet = newPet(now, static_cast<uint8_t>(esp_random() % kLineCount));
  if (clockValid) {
    advance(pet, now);
    lastClock = now;
  }
  newLowStats(lowAlerts, pet);  // no alert beeps for stats that were already low

  ui::build(onAction, ui::Settings{brightness, volume});
  hal::touchSetGate(touchGate);
  battery = hal::batteryPercent();
  charging = hal::isCharging();
  refreshUi(now);
  // Very first start (or a lost clock): ask for the time, as the spec says.
  if (!clockValid || !loaded) ui::showTimePrompt(true);
  screen.wake(millis());
  save();
  hal::beep(784, 60);
}

void appLoop() {
  lv_timer_handler();
  const uint32_t ms = millis();  // after the handler: touches inside it use later times

  if (ui::minigameActive()) screen.wake(ms);
  applyScreenMode(screen.update(ms));

  if (ms - lastTick >= kTickMs) {
    lastTick = ms;
    if (ms - lastBattery >= kBatteryMs) {
      lastBattery = ms;
      battery = hal::batteryPercent();
      charging = hal::isCharging();
    }
    const uint32_t now = tickPet();
    if (screen.mode() != ScreenPower::Off) refreshUi(now);
  }
  if (ms - lastSave >= kSaveMs) save();
  delay(screen.mode() == ScreenPower::Off ? 30 : 5);
}

}  // namespace sg
