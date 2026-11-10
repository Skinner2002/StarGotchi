#!/usr/bin/env python3
"""Fails if an ESP Web Tools manifest would overwrite the NVS save area.

Usage: check_manifest.py site/manifest.json
Exit 0 = safe, 1 = unsafe or broken.
"""
import json
import os
import sys

NVS_START, NVS_END = 0x9000, 0xE000  # end exclusive (app3M_fat9M_16MB)


def main(path):
    base = os.path.dirname(os.path.abspath(path))
    with open(path) as f:
        manifest = json.load(f)
    errors = []
    # Without this flag ESP Web Tools silently erases the whole flash (and the
    # bot) on every install; with it the user gets an "Erase device" checkbox.
    if manifest.get("new_install_prompt_erase") is not True:
        errors.append("new_install_prompt_erase must be true, otherwise every install wipes the save")
    for build in manifest.get("builds", []):
        for part in build.get("parts", []):
            name, offset = part["path"], int(part["offset"])
            if "merged" in os.path.basename(name):
                errors.append(f"{name}: merged images overwrite the save area")
                continue
            file = os.path.join(base, name)
            if not os.path.isfile(file):
                errors.append(f"{name}: file missing")
                continue
            end = offset + os.path.getsize(file)
            if offset < NVS_END and end > NVS_START:
                errors.append(f"{name}: 0x{offset:X}-0x{end:X} overlaps NVS 0x9000-0xE000")
    for e in errors:
        print("check_manifest:", e)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
