# Build notes – decisions made during implementation (v1)

Every deviation from the plan, with what it costs if wrong. Generated from the build ledger.

## Rulings
- Setup: Ruling: work directly on main (no feature branch) — brand-new repo; approved design deploys installer from main on every push, branch protection for github-pages env only allows default branch — cost if wrong: history on main instead of a branch, no data loss.
- Setup: Ruling: hardware gates (T1 S7, T6 S5, T7 S2, T8 S5, T9 S2) do not block execution; they become one user checklist at the end — user is asleep/away, CI compile is the proxy — cost if wrong: rework after user test.
- - T6→T9: ScreenPower::onTouch gates touches before LVGL; T6 touchBegin registers indev — T9 needs a hook in touch read cb. Ruling: touch.h exposes `void touchSetGate(bool(*gate)(uint32_t ms))` called from read_cb on press edge — cost if wrong: small refactor.
- Task 1: Ruling: local builds use arduino-cli with a trimmed GitHub-hosted core index + vendored Waveshare libs (proxy blocks downloads.arduino.cc); CI uses the normal Arduino library manager with identical versions — cost if wrong: CI-only build differences.
- Task 1: Ruling: run_tests.sh links all game/ and storage/ sources into every test instead of "matching" sources — simpler, same result — cost if wrong: none.
- Task 1: Ruling: deploy job only on main (tags excluded) — github-pages environment allows default branch only — cost if wrong: tag releases need a main push to publish.
- Task 1: Ruling: added manifest checker test tests/test_check_manifest.py and tools/make_site.sh (shared by CI & local) — cost if wrong: none.
- Task 3: Ruling: Combat requires playCount > 0 (plan formula made a pet with zero actions Combat) — cost if wrong: none. Added test stat_recovery_resets_zero_timer.
- Task 4: Ruling: nvs_store also carries u8 settings (brightness/sound) for Task 7 setup page — cost if wrong: none.
- CI: Ruling: libraries come from Waveshare repo commit e4344e7 instead of Library Manager — LM GFX 1.6.4 fails on core 3.3.10 — cost if wrong: none (same as local).
- Task 5: Ruling: added moon sprite (night indicator) and shutdown has 2 frames (spark) — cost if wrong: none.
- Task 5: Ruling: sprite approval by user is asynchronous (previews sent); Task 7 proceeds — cost if wrong: redraw sprites later (cheap, txt sources).
- Task 5: Ruling: pixel maps were authored with a scratch drawing script (not committed); assets/sprites/*.txt are the source of truth — cost if wrong: none.
- Task 6: Ruling: hal API is one header hal/hal.h (+ i2cBegin, touchSetGate per pre-flight) instead of 5 headers; files touch.cpp, sensors.cpp, audio.cpp — cost if wrong: none.
- Task 6: Ruling: added pure game/clock.{h,cpp} (calendar<->epoch, isValidTime) with tests — RTC conversion needed testable logic — cost if wrong: none.
- Task 6: Ruling: added ScreenPower::wake(ms) for alerts — cost if wrong: none.
- Task 6: Ruling: no SG_DIAG screen — user cannot set build flags; hardware is verified through the real UI checklist instead — cost if wrong: harder fault isolation on hardware.
- Task 6: Ruling: tilt axis = accel X with sign SG_TILT_SIGN (-1), x2 gain, low-pass — unknown board orientation, tunable — cost if wrong: one sign flip after user test.
- Task 7: Ruling: sprites are pre-scaled at build time (bots x4, icons/ship/rock x3, moon x2; new `scale=` field + test) instead of lv_img zoom — plain blits keep animation smooth — cost if wrong: ~0.8 MB more flash (fits easily).
- Task 7: Ruling: ui API uses ActionHandler(Action, arg) with extra actions SetTime/Brightness/Sound and Settings struct; added debugShowPage for the renderer — cost if wrong: none.
- Task 7: Ruling: added tools/sim desktop renderer (real LVGL + UI code → PNG) as layout verification in place of the hardware photo gate — cost if wrong: on-device differences (touch, speed) still need the user test.
- Task 7: Ruling: low-condition shown as rust tint on the bot (no spark overlay) — simpler, readable — cost if wrong: cosmetic.
- Task 7: Ruling: rollers use NORMAL mode (INFINITE overflowed LVGL 13-bit coords with 60 minutes) — found via renderer — cost if wrong: none.
- Task 7: Ruling: LVGL default theme switched to dark — cost if wrong: none.
- Task 8: Ruling: tests drive the game with steering policies (toward/away from rocks) instead of a test-only rock-placement hook; added elapsed(), kShipY etc. as public constants — keeps test code out of production — cost if wrong: none.
- Task 8: Ruling: applyGameResult ignores results while shut down; minigame UI takes a MinigameIO struct (tilt/beep/finished) so ui stays hardware-free; 1.5 s "GET READY" intro — cost if wrong: none.
- Task 9: Ruling: added pure game/events.{h,cpp} (low-stat alert edge detection, clockSetTarget, rebaseTime) with tests — an invalid RTC at first boot would otherwise make the pet jump decades and shut down when the clock is set — cost if wrong: none.
- Task 9: Ruling: when the clock is invalid, the pet is rebased to the newly set time (no decay for the unknown gap); setting a valid clock later is treated as normal time movement — cost if wrong: a user who sets the clock forward by hours sees that much decay.
- Task 9: Ruling: version label moved to Stats page — cost if wrong: none.
- Task 10: Ruling: no v1.0.0 tag yet — releasing before any hardware test would ship an untested build; tag after the user's checklist passes — cost if wrong: one later tag command.
- Final: Ruling: C2 display buffers (2×48 lines internal RAM, PSRAM fallback, halt on failure), I2 interrupt-gated touch + 80 ms release debounce, I4 time prompt when no save, I5 panel sleep + paused animations — device-only code, verified by compile + desktop render; no unit test possible — cost if wrong: one hardware test round.
- Final: Ruling: "Astro-Bot" renamed "Nova-Bot" (display text, MakerWorld text, spec) — existing product name conflicts with the no-protected-names rule — cost if wrong: one string.
- Final: Ruling: date stays fixed to 2026-01-01 after RTC loss; tilt axis/feel; AXP2101/ES8311 config — need hardware; left for user test — cost if wrong: tuning round.

## Final review fixes
- Final: fixed C1 installer erased save on every install — erase_without_prompt_rejected RED→GREEN, suite 9/9 files
- Final: fixed C3 clock set backwards → instant evolution/shutdown — clock_set_backwards_no_instant_evolution / _shutdown RED→GREEN, suite 9/9
- Final: fixed I1 one-frame blackout on tap — update_with_earlier_timestamp_stays_on RED→GREEN, suite 9/9
- Final: fixed I3 garbage RTC reading applies weeks of decay — clock_reading_acceptance (compile-RED, then GREEN), suite 9/9

## Deferred minors
- pet younger than 7 days offline >30 days skips stages with a single care sample (can become Nova instead of Scrap)
- missing RTC chip → rtcNow()=0 and time prompt can never clear (no millis() fallback)
- dock button restyled every second (extra redraw)
- rollers can snap to the clock mid-drag when the minute changes
- tools/__pycache__ committed; add to .gitignore
- (done incidentally with C1) improv wait time 0
