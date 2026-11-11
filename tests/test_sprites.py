"""Tests for tools/sprites.py (pixel maps -> LVGL image arrays)."""
import importlib.util
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("sprites", os.path.join(ROOT, "tools", "sprites.py"))
sprites = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sprites)

failures = 0


def check(name, cond):
    global failures
    if not cond:
        failures += 1
        print("FAIL:", name)


PALETTE = sprites.parse_palette("R #FF0000\nB #0000FF\n")

# 2x2: red, transparent / transparent, blue  -> RGB565 little-endian + alpha
frames, size = sprites.parse_sprite("t.txt", "2x2 frames=1\nR.\n.B\n", PALETTE)
check("size", size == (2, 2))
check("one_frame", len(frames) == 1)
check("bytes", frames[0] == bytes([0x00, 0xF8, 0xFF, 0, 0, 0, 0, 0, 0, 0x1F, 0x00, 0xFF]))

two, _ = sprites.parse_sprite("t.txt", "1x1 frames=2\nR\n\nB\n", PALETTE)
check("two_frames", len(two) == 2 and two[1] == bytes([0x1F, 0x00, 0xFF]))

big, bsize = sprites.parse_sprite("t.txt", "1x2 frames=1 scale=2\nR\nB\n", PALETTE)
check("scaled_size", bsize == (2, 4))
red, blue = bytes([0x00, 0xF8, 0xFF]), bytes([0x1F, 0x00, 0xFF])
check("scaled_bytes", big[0] == red * 4 + blue * 4)

# Indexed 4-bit output (what the firmware stores): 16-entry BGRA palette with
# index 0 transparent, then 2 pixels per byte, high nibble first.
idx, isize = sprites.indexed_sprite("t.txt", "2x2 frames=1\nR.\n.B\n", PALETTE)
pal = bytes([0, 0, 0, 0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF]) + bytes(13 * 4)
check("indexed_size", isize == (2, 2))
check("indexed_bytes", idx[0] == pal + bytes([0x10, 0x02]))
odd, _ = sprites.indexed_sprite("t.txt", "3x1 frames=1 scale=1\nRBR\n", PALETTE)
check("indexed_odd_width_padded", odd[0][64:] == bytes([0x12, 0x10]))

try:
    sprites.parse_sprite("bad.txt", "2x2 frames=1\nRRR\n.B\n", PALETTE)
    check("width_error_raised", False)
except ValueError as e:
    check("width_error_names_file_and_line", "bad.txt" in str(e) and "line 2" in str(e))

try:
    sprites.parse_sprite("bad.txt", "1x1 frames=1\nZ\n", PALETTE)
    check("unknown_color_raised", False)
except ValueError as e:
    check("unknown_color_message", "Z" in str(e))

# The committed C file must match what the tool generates from the sources.
gen = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "sprites.py"), "--check"],
                     cwd=ROOT, capture_output=True, text=True)
check("generated_code_up_to_date", gen.returncode == 0)
if gen.returncode:
    print(gen.stdout, gen.stderr)

print("test_sprites:", "FAILED" if failures else "ok")
sys.exit(1 if failures else 0)
