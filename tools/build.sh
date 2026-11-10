#!/usr/bin/env bash
# Compiles the firmware with arduino-cli. Usage: tools/build.sh [version]
# Requires: core esp32:esp32@3.3.10, libs lvgl 8.4.0, GFX Library for Arduino 1.6.4,
# SensorLib 0.3.1, XPowersLib 0.2.6, and firmware/lv_conf.h copied next to the
# Arduino libraries folder (see .github/workflows/build.yml).
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
