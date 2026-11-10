#!/usr/bin/env bash
# Builds and runs every unit test. Each tests/test_<x>.cpp is linked with the
# pure-logic sources listed in TEST_SOURCES below (no Arduino code).
set -euo pipefail
cd "$(dirname "$0")/.."
SRC=firmware/stargotchi/src
TEST_SOURCES="$(ls $SRC/game/*.cpp $SRC/storage/*.cpp 2>/dev/null || true)"
mkdir -p tests/bin
fail=0
for t in tests/test_*.cpp; do
  bin="tests/bin/$(basename "${t%.cpp}")"
  g++ -std=c++17 -O1 -Wall -Wextra -Werror -I"$SRC" -Itests "$t" $TEST_SOURCES -o "$bin"
  "$bin" || fail=1
done
for t in tests/test_*.py; do
  python3 "$t" || fail=1
done
exit $fail
