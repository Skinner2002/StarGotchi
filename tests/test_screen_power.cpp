#include "check.h"
#include "game/screen_power.h"

using namespace sg;

TEST(starts_on) {
  ScreenPower sp;
  CHECK(sp.update(0) == ScreenPower::On);
}

TEST(dims_after_30s) {
  ScreenPower sp;
  sp.onTouch(1000);
  CHECK(sp.update(30999) == ScreenPower::On);
  CHECK(sp.update(31000) == ScreenPower::Dim);
}

TEST(off_after_60s) {
  ScreenPower sp;
  sp.onTouch(1000);
  CHECK(sp.update(60999) == ScreenPower::Dim);
  CHECK(sp.update(61000) == ScreenPower::Off);
}

TEST(wake_touch_is_consumed) {
  ScreenPower sp;
  sp.onTouch(0);
  sp.update(70000);
  CHECK(sp.mode() == ScreenPower::Off);
  CHECK(sp.onTouch(70500) == true);
  CHECK(sp.mode() == ScreenPower::On);
  CHECK(sp.onTouch(70600) == false);
}

TEST(dim_touch_is_consumed) {
  ScreenPower sp;
  sp.onTouch(0);
  sp.update(40000);
  CHECK(sp.onTouch(40001) == true);
  CHECK(sp.mode() == ScreenPower::On);
}

TEST(touch_resets_idle) {
  ScreenPower sp;
  sp.onTouch(0);
  sp.update(20000);
  CHECK(sp.onTouch(20000) == false);
  CHECK(sp.update(45000) == ScreenPower::On);
  CHECK(sp.update(50000) == ScreenPower::Dim);
}

TEST(wake_without_touch) {
  ScreenPower sp;
  sp.onTouch(0);
  sp.update(70000);
  sp.wake(70000);
  CHECK(sp.mode() == ScreenPower::On);
  CHECK(sp.update(99999) == ScreenPower::On);
}

RUN_TESTS()
