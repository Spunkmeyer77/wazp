#!/usr/bin/env python3
"""Renders the WAZP panel artwork (assets/ui_src) into the images the plug-in uses (assets/ui).

  python3 tools/rasterise_ui.py          (needs rsvg-convert and Pillow)

- background.jpg : the whole panel at 2x (3072 x 2048) with the moving parts removed
                   (knob pointers, the SYNC rocker and the preset name are drawn by the plug-in)
- sync_off.png / sync_on.png : the SYNC switch states at 2x
- layout.json    : knob / key / button geometry, read when Source/Layout.h is regenerated
"""
import json, os, re, subprocess, sys, tempfile
import xml.etree.ElementTree as ET
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "assets", "ui_src")
OUT = os.path.join(ROOT, "assets", "ui")
SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)
ET.register_namespace("xlink", "http://www.w3.org/1999/xlink")
SCALE = 2

KNOBS = ["glide", "osc1-range", "osc1-width", "osc1-wave", "osc2-range", "osc2-pitch", "osc2-wave",
         "lfo-frequency", "pitch-mod", "noise", "lfo-wave", "filter-frequency", "resonance", "filter-mode",
         "filter-mod1", "filter-mod2", "vca-attack", "vca-decay", "vca-sustain", "env-attack", "env-decay",
         "env-delay", "volume", "knob_osc1_level", "knob_osc2_level"]


def render(svg_path, png_path, width):
    subprocess.check_call(["rsvg-convert", "-w", str(width), svg_path, "-o", png_path])


def main():
    os.makedirs(OUT, exist_ok=True)
    tree = ET.parse(os.path.join(SRC, "WAZP_Candy_UI.svg"))
    root = tree.getroot()
    parent = {c: p for p in root.iter() for c in p}
    by_id = {e.get("id"): e for e in root.iter() if e.get("id")}

    layout = {"knobs": {}, "white_keys": [], "black_keys": []}
    for k in KNOBS:
        g = by_id[k]
        m = re.search(r"translate\(([-\d.]+)[ ,]+([-\d.]+)\)(?:\s*scale\(([-\d.]+)\))?", g.get("transform"))
        layout["knobs"][k] = {"x": float(m.group(1)), "y": float(m.group(2)), "s": float(m.group(3) or 1)}
        g.remove(list(g)[-1])                      # the pointer line (last element of every knob)

    sw = by_id["switch_sync"]
    for c in list(sw):                          # keep the SYNC / OFF / ON labels, drop the switch graphic
        if not c.tag.endswith("text"):
            sw.remove(c)
    name = by_id["preset_name"]
    parent[name].remove(name)

    for i in range(15):
        r = by_id.get(f"key-{i}")[0]
        layout["white_keys"].append([float(r.get(a)) for a in ("x", "y", "width", "height")])
    for i in range(15):
        g = by_id.get(f"black-key-{i}")
        if g is not None:
            r = g[0]
            layout["black_keys"].append([i] + [float(r.get(a)) for a in ("x", "y", "width", "height")])
    for b in ["preset_display", "preset_prev", "preset_next", "preset_save", "preset_load"]:
        r = by_id[b][0]
        layout[b] = [float(r.get(a)) for a in ("x", "y", "width", "height")]

    with tempfile.TemporaryDirectory() as tmp:
        bg_svg = os.path.join(tmp, "bg.svg")
        tree.write(bg_svg)
        bg_png = os.path.join(tmp, "bg.png")
        render(bg_svg, bg_png, 1536 * SCALE)
        Image.open(bg_png).convert("RGB").save(os.path.join(OUT, "background.jpg"), quality=92)

        sw_png = os.path.join(tmp, "sw.png")
        render(os.path.join(SRC, "components", "Switch_Toggle.svg"), sw_png, 136 * SCALE)
        im = Image.open(sw_png)
        # both states are 68 x 46 cells centred at (34, 23) and (102, 23)
        im.crop((0, 0, 68 * SCALE, 46 * SCALE)).save(os.path.join(OUT, "sync_off.png"))
        im.crop((68 * SCALE, 0, 136 * SCALE, 46 * SCALE)).save(os.path.join(OUT, "sync_on.png"))

    with open(os.path.join(OUT, "layout.json"), "w") as f:
        json.dump(layout, f, indent=1)
    print("rendered to", OUT)


if __name__ == "__main__":
    sys.exit(main())
