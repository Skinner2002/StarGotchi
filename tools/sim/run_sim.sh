#!/usr/bin/env bash
# Renders UI screenshots on the desktop into sim_out/*.png.
# Needs: LVGL 8.4 source in $LVGL_DIR (default ~/Arduino/libraries/lvgl), Pillow.
set -euo pipefail
cd "$(dirname "$0")/../.."
LVGL_DIR="${LVGL_DIR:-$HOME/Arduino/libraries/lvgl}"
SRC=firmware/stargotchi/src
OBJ=build/sim && mkdir -p $OBJ sim_out
CFLAGS="-O1 -DLV_CONF_INCLUDE_SIMPLE -Ifirmware -I$LVGL_DIR -I$LVGL_DIR/src -I$SRC -I$SRC/assets"
if [ ! -f $OBJ/liblvgl.a ]; then
  for f in $(find "$LVGL_DIR/src" -name '*.c'); do
    gcc $CFLAGS -c "$f" -o "$OBJ/$(echo "$f" | md5sum | cut -c1-12).o"
  done
  ar rcs $OBJ/liblvgl.a $OBJ/*.o
fi
gcc $CFLAGS -c $SRC/assets/sprites.c -o $OBJ/sprites.o
g++ -std=c++17 $CFLAGS tools/sim/sim.cpp $SRC/ui/*.cpp $SRC/game/*.cpp $SRC/assets/*.cpp \
  $OBJ/sprites.o $OBJ/liblvgl.a -o $OBJ/sim
$OBJ/sim
python3 - <<'PY'
import glob
from PIL import Image
for f in glob.glob("sim_out/*.ppm"):
    Image.open(f).save(f[:-4] + ".png")
PY
ls sim_out/*.png
