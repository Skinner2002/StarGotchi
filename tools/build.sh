#!/usr/bin/env bash
# Compiles the firmware with arduino-cli. Usage: tools/build.sh [version]
# Requires: core esp32:esp32@3.3.10 and the lvgl / GFX_Library_for_Arduino /
# SensorLib / XPowersLib copies from waveshareteam/ESP32-S3-Touch-AMOLED-1.75
# (examples/arduino/libraries, commit e4344e7) plus firmware/lv_conf.h in the
# Arduino libraries folder. See .github/workflows/build.yml.
set -euo pipefail
cd "$(dirname "$0")/.."
VERSION="${1:-dev}"
FQBN="esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc"
arduino-cli compile \
  --fqbn "$FQBN" \
  --build-property "compiler.cpp.extra_flags=-DSG_VERSION=\"${VERSION}\"" \
  --build-path build \
  --output-dir out \
  firmware/stargotchi
