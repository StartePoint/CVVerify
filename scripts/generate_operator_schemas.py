#!/usr/bin/env python3
"""One-shot generator: export StepSchema from builtin operator C++ sources to JSON."""

from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILTIN = ROOT / "src/core/operators/builtin"
OUT_DIR = ROOT / "config/operator_schemas"

TYPE_MAP = {
    "Integer": "integer",
    "Double": "double",
    "Boolean": "boolean",
    "Choice": "choice",
    "String": "string",
}

CATEGORY_MAP = {
    "mean_blur": "Filtering",
    "box_filter": "Filtering",
    "bilateral_filter": "Filtering",
    "median_blur": "Filtering",
    "gaussian_blur": "Filtering",
    "brightness_contrast": "Enhancement",
    "linear_abs_transform": "Enhancement",
    "hist_equalize": "Enhancement",
    "histogram_line": "Enhancement",
    "histogram_bar": "Enhancement",
    "affine_transform": "Transform",
    "polar_transform": "Transform",
    "perspective_transform": "Transform",
    "resize": "Transform",
    "crop": "Transform",
    "rotate": "Transform",
    "flip": "Transform",
    "threshold": "Thresholding",
    "threshold_otsu": "Thresholding",
    "threshold_triangle": "Thresholding",
    "threshold_adaptive": "Thresholding",
    "canny": "Edge Detection",
    "edge_sobel": "Edge Detection",
    "edge_laplacian": "Edge Detection",
    "edge_scharr": "Edge Detection",
    "grayscale": "Filter Effects",
    "color_convert": "Filter Effects",
    "color_map": "Filter Effects",
    "pencil_sketch": "Filter Effects",
    "art_effect": "Filter Effects",
    "fiber_glow": "Filter Effects",
    "emboss": "Filter Effects",
    "special_effect": "Filter Effects",
    "contour_detect": "Detection",
    "hough_lines": "Detection",
    "hough_circles": "Detection",
    "corner_detect": "Detection",
    "connected_components": "Detection",
    "template_match": "Detection",
    "surf_keypoints": "Detection",
    "sift_keypoints": "Detection",
    "segment_kmeans": "Segmentation",
    "flood_fill": "Segmentation",
    "segment_watershed": "Segmentation",
    "grabcut": "Segmentation",
    "segment_meanshift": "Segmentation",
    "surf_match": "Feature Matching",
    "morphology": "Morphology",
}


def parse_value(raw: str | None):
    if raw is None:
        return None
    value = raw.strip()
    if value in ("", "{}"):
        return None
    if value == "true":
        return True
    if value == "false":
        return False
    if value.startswith('"') and value.endswith('"'):
        return value[1:-1]
    try:
        if "." in value:
            return float(value)
        return int(value)
    except ValueError:
        return value


def split_top_level(text: str) -> list[str]:
    tokens: list[str] = []
    pos = 0
    length = len(text)
    while pos < length:
        while pos < length and text[pos] in " \t\n\r,":
            pos += 1
        if pos >= length:
            break

        start = pos
        if text[pos] == '"':
            pos += 1
            while pos < length and text[pos] != '"':
                pos += 1
            pos += 1
            tokens.append(text[start:pos])
        elif text[pos] == "{":
            depth = 0
            while pos < length:
                if text[pos] == "{":
                    depth += 1
                elif text[pos] == "}":
                    depth -= 1
                    if depth == 0:
                        pos += 1
                        break
                pos += 1
            tokens.append(text[start:pos])
        else:
            while pos < length and text[pos] not in ",":
                pos += 1
            tokens.append(text[start:pos])
    return tokens


def parse_choices(raw: str) -> list[dict[str, str]]:
    choices: list[dict[str, str]] = []
    for match in re.finditer(r'\{"([^"]+)",\s*"([^"]+)"\}', raw):
        choices.append({"value": match.group(1), "label": match.group(2)})
    return choices


def parse_param(block: str) -> dict | None:
    inner = block.strip()
    if not inner.startswith("{") or not inner.endswith("}"):
        return None
    inner = inner[1:-1]

    tokens = split_top_level(inner)
    if len(tokens) < 4:
        return None

    key = parse_value(tokens[0])
    display = parse_value(tokens[1])
    group = parse_value(tokens[2])
    type_match = re.match(r"StepParameterType::(\w+)", tokens[3].strip())
    if not key or not display or not group or not type_match:
        return None

    ptype = TYPE_MAP.get(type_match.group(1), "string")
    tail = tokens[4:]

    default = parse_value(tail[0]) if len(tail) > 0 else None
    minimum = parse_value(tail[1]) if len(tail) > 1 else None
    maximum = parse_value(tail[2]) if len(tail) > 2 else None
    step = parse_value(tail[3]) if len(tail) > 3 else None
    visible_when = parse_value(tail[4]) if len(tail) > 4 else None
    tooltip = parse_value(tail[5]) if len(tail) > 5 else None
    choices_raw = tail[6] if len(tail) > 6 else None

    param: dict = {
        "key": key,
        "displayName": display,
        "group": group,
        "type": ptype,
    }
    if default is not None:
        param["default"] = default
    if isinstance(minimum, (int, float)):
        param["min"] = minimum
    if isinstance(maximum, (int, float)):
        param["max"] = maximum
    if isinstance(step, (int, float)):
        param["step"] = step
    if isinstance(visible_when, str) and visible_when:
        param["visibleWhen"] = visible_when
    if isinstance(tooltip, str) and tooltip:
        param["tooltip"] = tooltip

    if choices_raw:
        choices = parse_choices(choices_raw)
        if choices:
            param["choices"] = choices
    return param


def extract_param_blocks(params_block: str) -> list[str]:
    blocks: list[str] = []
    depth = 0
    start = -1
    for index, char in enumerate(params_block):
        if char == "{":
            if depth == 0:
                start = index
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0 and start >= 0:
                blocks.append(params_block[start : index + 1])
                start = -1
    return blocks


def parse_schema(text: str) -> tuple[str, str, list[dict]] | None:
    match = re.search(
        r'return\s*\{\s*"([^"]+)",\s*"([^"]+)",\s*\{(.*?)\}\s*\};',
        text,
        re.S,
    )
    if not match:
        return None

    op_id, display, params_block = match.group(1), match.group(2), match.group(3)
    params = []
    for block in extract_param_blocks(params_block):
        param = parse_param(block)
        if param:
            params.append(param)
    return op_id, display, params


def main() -> int:
    names: list[str] = []
    failures: list[str] = []

    for cpp in sorted(BUILTIN.glob("*.cpp")):
        text = cpp.read_text(encoding="utf-8")
        if "StepSchema" not in text or "::schema()" not in text:
            continue

        parsed = parse_schema(text)
        if not parsed:
            failures.append(cpp.name)
            continue

        op_id, display, params = parsed
        stem = op_id.split(".")[-1]
        names.append(stem)

        document = {
            "id": op_id,
            "displayName": display,
            "category": CATEGORY_MAP.get(stem, "Operators"),
            "parameters": params,
        }
        (OUT_DIR / f"{stem}.json").write_text(
            json.dumps(document, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
        )

    index = {"version": 1, "schemas": sorted(names)}
    (OUT_DIR / "index.json").write_text(
        json.dumps(index, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )

    print(f"generated {len(names)} schemas")
    if failures:
        print("failed:", ", ".join(failures))
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
