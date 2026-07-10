#!/usr/bin/env python3
"""Generate small bundled sample images for offline validation workflows."""

from __future__ import annotations

import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
IMAGE_DIR = ROOT / "samples" / "images"


def checkerboard_rgb(width: int, height: int, tile: int = 16) -> bytes:
    rows = bytearray()
    for y in range(height):
        row = bytearray([0])
        for x in range(width):
            if ((x // tile) + (y // tile)) % 2 == 0:
                row.extend((220, 220, 220))
            else:
                row.extend((40, 90, 180))
        rows.extend(row)
    return bytes(rows)


def write_png(path: Path, width: int, height: int, rgb_rows: bytes) -> None:
    def chunk(tag: bytes, payload: bytes) -> bytes:
        crc = zlib.crc32(tag + payload) & 0xFFFFFFFF
        return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", crc)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    compressed = zlib.compress(rgb_rows, 9)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", compressed) + chunk(b"IEND", b"")
    path.write_bytes(png)


def main() -> int:
    IMAGE_DIR.mkdir(parents=True, exist_ok=True)

    specs = {
        "checkerboard.png": (128, 128, 16),
        "gradient_ramp.png": (256, 64, 8),
    }

    for filename, (width, height, tile) in specs.items():
        if filename == "gradient_ramp.png":
            rows = bytearray()
            for y in range(height):
                row = bytearray([0])
                for x in range(width):
                    value = int(255 * x / max(width - 1, 1))
                    row.extend((value, value // 2, 255 - value))
                rows.extend(row)
            rgb = bytes(rows)
        else:
            rgb = checkerboard_rgb(width, height, tile)

        write_png(IMAGE_DIR / filename, width, height, rgb)

    print(f"Generated sample images in {IMAGE_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
