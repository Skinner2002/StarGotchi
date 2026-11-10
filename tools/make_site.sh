#!/usr/bin/env bash
# Assembles the GitHub Pages site (installer + firmware) into site/.
# Usage: tools/make_site.sh <version>   (run after tools/build.sh)
set -euo pipefail
cd "$(dirname "$0")/.."
VERSION="$1"
CORE_DIR="${ARDUINO_CORE_DIR:-$HOME/.arduino15/packages/esp32/hardware/esp32/3.3.10}"
rm -rf site && mkdir -p site/firmware
cp web/* site/
cp out/stargotchi.ino.bootloader.bin site/firmware/bootloader.bin
cp out/stargotchi.ino.partitions.bin site/firmware/partitions.bin
cp out/stargotchi.ino.bin site/firmware/stargotchi.bin
cp "$CORE_DIR/tools/partitions/boot_app0.bin" site/firmware/boot_app0.bin
echo "$VERSION" > site/version.txt
cat > site/manifest.json <<EOF
{
  "name": "StarGotchi",
  "version": "$VERSION",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "firmware/bootloader.bin", "offset": 0 },
        { "path": "firmware/partitions.bin", "offset": 32768 },
        { "path": "firmware/boot_app0.bin", "offset": 57344 },
        { "path": "firmware/stargotchi.bin", "offset": 65536 }
      ]
    }
  ]
}
EOF
python3 tools/check_manifest.py site/manifest.json
echo "site ready: version $VERSION"
