"""Tests for tools/check_manifest.py (installer must never overwrite the save)."""
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOL = os.path.join(ROOT, "tools", "check_manifest.py")
failures = 0


def run(parts, sizes, erase_prompt=True):
    with tempfile.TemporaryDirectory() as d:
        os.makedirs(os.path.join(d, "firmware"))
        for (path, _), size in zip(parts, sizes):
            with open(os.path.join(d, path), "wb") as f:
                f.write(b"\xff" * size)
        manifest = {
            "name": "StarGotchi",
            "new_install_prompt_erase": erase_prompt,
            "builds": [{"chipFamily": "ESP32-S3",
                        "parts": [{"path": p, "offset": o} for p, o in parts]}],
        }
        mpath = os.path.join(d, "manifest.json")
        with open(mpath, "w") as f:
            json.dump(manifest, f)
        return subprocess.run([sys.executable, TOOL, mpath], capture_output=True).returncode


def check(name, cond):
    global failures
    if not cond:
        failures += 1
        print("FAIL:", name)


GOOD = [("firmware/bootloader.bin", 0x0), ("firmware/partitions.bin", 0x8000),
        ("firmware/boot_app0.bin", 0xE000), ("firmware/stargotchi.bin", 0x10000)]
check("good_layout_passes", run(GOOD, [0x5000, 0xC00, 0x2000, 0x90000]) == 0)
check("bootloader_too_big_overlaps_nvs", run(GOOD, [0x9800, 0xC00, 0x2000, 0x90000]) == 1)
check("erase_without_prompt_rejected", run(GOOD, [0x5000, 0xC00, 0x2000, 0x90000], erase_prompt=False) == 1)
check("merged_bin_rejected", run([("firmware/stargotchi.merged.bin", 0x0)], [0x100]) == 1)
check("part_inside_nvs_rejected", run([("firmware/x.bin", 0xA000)], [0x10]) == 1)
check("missing_file_rejected", run([("firmware/none.bin", 0x10000)], []) == 1)

print("test_check_manifest:", "FAILED" if failures else "ok")
sys.exit(1 if failures else 0)
