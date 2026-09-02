#!/usr/bin/env python3
"""Regenerate keymap-viz.html from keymap.c + the board's keyboard.json.

Run this after editing keymap.c (new keys, new layers, etc). It re-parses
the LAYOUT(...) argument lists and the physical key positions, then
rewrites keymap-viz.html next to this script. The label/color logic for
individual keycodes lives in the HTML template below (LABEL_RULES section
of the embedded <script>) -- extend KC/MOUSE/MEDIA/NUMPAD/NAV/labelFor()
there if you introduce keycodes this script doesn't already know about.

Usage:
    python3 gen_keymap_viz.py [--keyboard-json PATH] [--out PATH]

After regenerating, open keymap-viz.html in a browser to check it, or ask
Claude Code to publish it as an artifact.
"""
import argparse
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT_KEYMAP_C = HERE / "keymap.c"
DEFAULT_KEYBOARD_JSON = HERE.parent.parent / "light" / "keyboard.json"
DEFAULT_OUT = HERE / "keymap-viz.html"

LAYER_TITLES = {
    "_QWERTY": ("Qwerty", None, "Base layer. Held modifiers double as layer taps in the thumb cluster."),
    "_LOWER": ("Lower", "var(--hue-lower)", "Numbers, symbols, mouse keys — held via the left thumb key."),
    "_RAISE": ("Raise", "var(--hue-raise)", "Function keys, navigation, arrows — held via the right thumb key."),
    "_ADJUST": ("Adjust", "var(--hue-adjust)", "Lower + Raise held together. Media transport, boot/reset, layer jump to Numeric."),
    "_NUMERIC": ("Numeric", "var(--hue-numeric)", "Standalone numpad layer, entered/exited with TO() — excluded from the tri-layer stack."),
}


def parse_keymaps(keymap_c: Path) -> "dict[str, list[str]]":
    text = keymap_c.read_text()
    m = re.search(
        r"const uint16_t PROGMEM keymaps\[\]\[MATRIX_ROWS\]\[MATRIX_COLS\] = \{(.*?)\n\};",
        text,
        re.S,
    )
    if not m:
        sys.exit(f"error: could not find the keymaps[] array in {keymap_c}")
    body = m.group(1)

    # Find each "[_LAYER] = LAYOUT(" header, then walk forward tracking paren
    # depth to find its matching close -- robust to whatever trails the
    # closing paren (trailing comma, comment, whitespace, end of array, ...).
    data = {}
    for header in re.finditer(r"\[(_\w+)\]\s*=\s*LAYOUT\(", body):
        name = header.group(1)
        start = header.end()
        depth = 1
        i = start
        while depth > 0:
            if i >= len(body):
                sys.exit(f"error: unbalanced parens in LAYOUT(...) for layer {name}")
            if body[i] == "(":
                depth += 1
            elif body[i] == ")":
                depth -= 1
            i += 1
        args = body[start : i - 1]
        args = re.sub(r"//[^\n]*", "", args)

        depth = 0
        toks = []
        cur = ""
        for ch in args:
            if ch == "(":
                depth += 1
                cur += ch
            elif ch == ")":
                depth -= 1
                cur += ch
            elif ch == "," and depth == 0:
                toks.append(cur.strip())
                cur = ""
            else:
                cur += ch
        if cur.strip():
            toks.append(cur.strip())
        data[name] = toks

    if not data:
        sys.exit(f"error: could not find any [_LAYER] = LAYOUT(...) entries in {keymap_c}")
    return data


def parse_layer_order(keymap_c: Path) -> "list[str]":
    """Return layer names in enum declaration order (the real firmware layer
    numbers), not the order they happen to appear in keymaps[] -- the two
    deliberately differ here (see the _NUMERIC-must-stay-below comment)."""
    text = keymap_c.read_text()
    m = re.search(r"enum\s+layers\s*\{(.*?)\}", text, re.S)
    if not m:
        sys.exit(f"error: could not find 'enum layers {{ ... }}' in {keymap_c}")
    names = [tok.strip() for tok in m.group(1).split(",")]
    return [n for n in names if n]


def parse_positions(keyboard_json: Path) -> "list[list[float]]":
    doc = json.loads(keyboard_json.read_text())
    try:
        layout = doc["layouts"]["LAYOUT"]["layout"]
    except KeyError:
        sys.exit(f"error: no layouts.LAYOUT.layout array in {keyboard_json}")
    positions = []
    for entry in layout:
        x, y = entry["x"], entry["y"]
        w, h = entry.get("w", 1), entry.get("h", 1)
        if w == 1 and h == 1:
            positions.append([x, y])
        else:
            positions.append([x, y, w, h])
    return positions


def build_layer_meta(layer_names: "list[str]") -> str:
    lines = []
    for i, name in enumerate(layer_names):
        label, hue, desc = LAYER_TITLES.get(name, (name.lstrip("_").title(), None, ""))
        hue_js = "null" if hue is None else json.dumps(hue)
        lines.append(
            f'  {name}: {{ label:{json.dumps(label)}, n:{i}, hue:{hue_js}, desc:{json.dumps(desc)} }}'
        )
    return "{\n" + ",\n".join(lines) + "\n}"


def render(keymap_c: Path, keyboard_json: Path, out_path: Path) -> None:
    layers = parse_keymaps(keymap_c)
    order = parse_layer_order(keymap_c)
    positions = parse_positions(keyboard_json)

    missing = [n for n in order if n not in layers]
    if missing:
        sys.exit(f"error: enum layers has {missing} but no matching [_LAYER] = LAYOUT(...) entry")
    extra = [n for n in layers if n not in order]
    if extra:
        sys.exit(f"error: {extra} has a LAYOUT(...) entry but is missing from 'enum layers'")

    for name, toks in layers.items():
        if len(toks) != len(positions):
            sys.exit(
                f"error: layer {name} has {len(toks)} keys but keyboard.json defines "
                f"{len(positions)} physical positions -- the LAYOUT() arg order no longer "
                f"matches keyboard.json's layout[] order, or a layer is missing keys."
            )

    layers = {name: layers[name] for name in order}
    layers_json = json.dumps(layers, indent=2)
    positions_json = json.dumps(positions)
    layer_meta_js = build_layer_meta(order)

    html = TEMPLATE
    html = html.replace("__POSITIONS__", positions_json)
    html = html.replace("__LAYERS__", layers_json)
    html = html.replace("__LAYER_META__", layer_meta_js)
    out_path.write_text(html)
    print(f"wrote {out_path} ({len(layers)} layers, {len(positions)} keys/layer)")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--keymap-c", type=Path, default=DEFAULT_KEYMAP_C)
    ap.add_argument("--keyboard-json", type=Path, default=DEFAULT_KEYBOARD_JSON)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = ap.parse_args()
    render(args.keymap_c, args.keyboard_json, args.out)


TEMPLATE = r"""<title>Lily58L Marce Layout</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Big+Shoulders+Display:wght@600;700&family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Mono:wght@400;500;600&display=swap">

<style>
  :root{
    --bg:#12151b;
    --surface:#191d25;
    --surface-2:#20252f;
    --key:#242a35;
    --key-alt:#1b2029;
    --line:#2c3242;
    --line-soft:#232833;
    --ink:#eef1f5;
    --ink-dim:#8d96a8;
    --ink-faint:#565f70;
    --accent:#c99a4c;
    --accent-ink:#2a1f0d;
    --danger:#e2685a;
    --danger-ink:#2c1310;
    --hue-lower:#4fb3a2;
    --hue-lower-ink:#0d211d;
    --hue-raise:#9d8cf0;
    --hue-raise-ink:#1c1830;
    --hue-adjust:#e2685a;
    --hue-adjust-ink:#2c1310;
    --hue-numeric:#d9a441;
    --hue-numeric-ink:#2a1f0d;
  }
  @media (prefers-color-scheme: light){
    :root:not([data-theme="light"]){
      --bg:#f3f1ea;
      --surface:#ffffff;
      --surface-2:#eae6db;
      --key:#ffffff;
      --key-alt:#f1efe6;
      --line:#d8d3c4;
      --line-soft:#e3dfd2;
      --ink:#221f19;
      --ink-dim:#6b6558;
      --ink-faint:#a49c89;
      --accent:#96712b;
      --accent-ink:#fbf1de;
      --danger:#b7392c;
      --danger-ink:#fcece9;
      --hue-lower:#1d7e6f;
      --hue-lower-ink:#e4f5f1;
      --hue-raise:#5c46c9;
      --hue-raise-ink:#ece8fb;
      --hue-adjust:#b7392c;
      --hue-adjust-ink:#fcece9;
      --hue-numeric:#96712b;
      --hue-numeric-ink:#fbf1de;
    }
  }
  :root[data-theme="light"]{
    --bg:#f3f1ea;
    --surface:#ffffff;
    --surface-2:#eae6db;
    --key:#ffffff;
    --key-alt:#f1efe6;
    --line:#d8d3c4;
    --line-soft:#e3dfd2;
    --ink:#221f19;
    --ink-dim:#6b6558;
    --ink-faint:#a49c89;
    --accent:#96712b;
    --accent-ink:#fbf1de;
    --danger:#b7392c;
    --danger-ink:#fcece9;
    --hue-lower:#1d7e6f;
    --hue-lower-ink:#e4f5f1;
    --hue-raise:#5c46c9;
    --hue-raise-ink:#ece8fb;
    --hue-adjust:#b7392c;
    --hue-adjust-ink:#fcece9;
    --hue-numeric:#96712b;
    --hue-numeric-ink:#fbf1de;
  }

  *{box-sizing:border-box;}
  body{
    margin:0;
    background:var(--bg);
    color:var(--ink);
    font-family:'IBM Plex Sans', ui-sans-serif, system-ui, sans-serif;
    min-height:100vh;
  }
  .wrap{
    max-width:1120px;
    margin:0 auto;
    padding:2.5rem 1.5rem 4rem;
  }
  header{
    display:flex;
    flex-wrap:wrap;
    align-items:baseline;
    justify-content:space-between;
    gap:.75rem 2rem;
    border-bottom:1px solid var(--line);
    padding-bottom:1.25rem;
    margin-bottom:1.75rem;
  }
  h1{
    font-family:'Big Shoulders Display', sans-serif;
    font-weight:700;
    font-size:clamp(2rem, 5vw, 2.75rem);
    letter-spacing:.01em;
    margin:0;
    text-wrap:balance;
    line-height:.95;
  }
  h1 span{color:var(--accent);}
  .sub{
    font-size:.9rem;
    color:var(--ink-dim);
    max-width:38ch;
    line-height:1.5;
    margin:0;
  }
  .path{
    font-family:'IBM Plex Mono', monospace;
    font-size:.72rem;
    color:var(--ink-faint);
    letter-spacing:.02em;
  }

  nav.tabs{
    display:flex;
    flex-wrap:wrap;
    gap:.5rem;
    margin-bottom:1.75rem;
  }
  .tab{
    font-family:'IBM Plex Mono', monospace;
    font-size:.78rem;
    font-weight:500;
    letter-spacing:.03em;
    padding:.5rem .9rem;
    border-radius:7px;
    border:1px solid var(--line);
    background:var(--surface);
    color:var(--ink-dim);
    cursor:pointer;
    transition:background .15s ease, color .15s ease, border-color .15s ease, transform .1s ease;
  }
  .tab:hover{transform:translateY(-1px);}
  .tab:focus-visible{outline:2px solid var(--accent); outline-offset:2px;}
  .tab .n{color:var(--ink-faint); margin-right:.4em;}
  .tab[data-active="true"]{
    color:var(--tab-ink, var(--accent-ink));
    background:var(--tab-hue, var(--accent));
    border-color:var(--tab-hue, var(--accent));
  }
  .tab[data-active="true"] .n{color:inherit; opacity:.65;}

  .panel{
    background:var(--surface);
    border:1px solid var(--line);
    border-radius:14px;
    padding:1.75rem 1.5rem 1.5rem;
  }
  .panel-head{
    display:flex;
    align-items:baseline;
    justify-content:space-between;
    gap:1rem;
    flex-wrap:wrap;
    margin-bottom:1.1rem;
  }
  .panel-head h2{
    font-family:'Big Shoulders Display', sans-serif;
    font-weight:700;
    font-size:1.5rem;
    margin:0;
    letter-spacing:.01em;
  }
  .panel-head p{
    margin:0;
    font-size:.83rem;
    color:var(--ink-dim);
    max-width:52ch;
    line-height:1.5;
  }

  .board-scroll{overflow-x:auto; padding-bottom:.25rem;}
  .board{
    position:relative;
    margin:.5rem auto 0;
  }
  .key{
    position:absolute;
    border-radius:8px;
    border:1px solid var(--line);
    background:var(--key);
    display:flex;
    flex-direction:column;
    align-items:center;
    justify-content:center;
    text-align:center;
    line-height:1.15;
    padding:2px;
    font-family:'IBM Plex Mono', monospace;
  }
  .key .main{font-size:.76rem; font-weight:500; color:var(--ink);}
  .key .sub{font-size:.56rem; color:var(--ink-dim); margin-top:1px;}

  .key.t-empty{background:transparent; border-color:var(--line-soft); border-style:dashed;}
  .key.t-trans{background:var(--key-alt); border-style:dashed; border-color:var(--line);}
  .key.t-trans .main{color:var(--ink-faint); font-size:.85rem;}
  .key.t-mod{background:var(--key-alt);}
  .key.t-mod .main{color:var(--ink-dim); font-size:.68rem; letter-spacing:.02em;}
  .key.t-dual{background:linear-gradient(160deg, var(--key) 55%, color-mix(in srgb, var(--accent) 24%, var(--key)) 55%);}
  .key.t-dual .sub{color:var(--accent);}
  .key.t-layer{background:color-mix(in srgb, var(--layer-hue) 26%, var(--key));border-color:color-mix(in srgb, var(--layer-hue) 55%, var(--line));}
  .key.t-layer .main{color:var(--layer-hue); font-size:.68rem; font-weight:600; letter-spacing:.02em;}
  .key.t-danger{background:color-mix(in srgb, var(--danger) 30%, var(--key)); border-color:color-mix(in srgb, var(--danger) 60%, var(--line));}
  .key.t-danger .main{color:var(--danger); font-size:.64rem; font-weight:600; letter-spacing:.02em;}
  .key.t-media,.key.t-nav,.key.t-numpad,.key.t-mouse{background:var(--key-alt);}
  .key.t-media .main,.key.t-nav .main,.key.t-numpad .main,.key.t-mouse .main{font-size:.66rem; color:var(--ink);}
  .key.t-toggle{background:color-mix(in srgb, var(--accent) 20%, var(--key)); border-color:color-mix(in srgb, var(--accent) 45%, var(--line));}
  .key.t-toggle .main{color:var(--accent); font-size:.62rem; font-weight:600;}

  .legend{
    display:flex;
    flex-wrap:wrap;
    gap:.55rem 1.1rem;
    margin-top:1.4rem;
    padding-top:1.1rem;
    border-top:1px solid var(--line-soft);
    font-size:.74rem;
    color:var(--ink-dim);
  }
  .legend .item{display:flex; align-items:center; gap:.4rem;}
  .legend .swatch{width:13px; height:13px; border-radius:4px; border:1px solid var(--line); flex:none;}

  .notes{
    margin-top:1.75rem;
    display:grid;
    grid-template-columns:repeat(auto-fit, minmax(230px,1fr));
    gap:1rem;
  }
  .note{
    background:var(--surface);
    border:1px solid var(--line);
    border-radius:12px;
    padding:1rem 1.1rem;
  }
  .note h3{
    font-family:'IBM Plex Mono', monospace;
    font-size:.68rem;
    text-transform:uppercase;
    letter-spacing:.08em;
    color:var(--ink-faint);
    margin:0 0 .5rem;
  }
  .note p{margin:0; font-size:.85rem; line-height:1.55; color:var(--ink-dim);}
  .note code{font-family:'IBM Plex Mono', monospace; font-size:.82em; color:var(--ink);}

  footer{
    margin-top:2.5rem;
    font-size:.72rem;
    color:var(--ink-faint);
    font-family:'IBM Plex Mono', monospace;
  }
</style>

<div class="wrap">
  <header>
    <div>
      <h1>Lily58<span>L</span> &middot; marce keymap</h1>
      <p class="sub">Five layers, one 58&#8209;key split &mdash; rendered straight from the firmware source, key positions pulled from the board's own <code>keyboard.json</code>.</p>
    </div>
    <div class="path">keyboards/lily58/keymaps/marce/keymap.c</div>
  </header>

  <nav class="tabs" id="tabs"></nav>

  <section class="panel">
    <div class="panel-head">
      <h2 id="panel-title">Qwerty</h2>
      <p id="panel-desc"></p>
    </div>
    <div class="board-scroll">
      <div class="board" id="board"></div>
    </div>
    <div class="legend">
      <div class="item"><span class="swatch t-mod" style="background:var(--key-alt)"></span>modifier</div>
      <div class="item"><span class="swatch" style="background:linear-gradient(160deg, var(--key) 55%, color-mix(in srgb, var(--accent) 24%, var(--key)) 55%)"></span>dual&#8209;role (hold / tap&#8209;dance)</div>
      <div class="item"><span class="swatch" style="background:color-mix(in srgb, var(--accent) 26%, var(--key)); border-color:color-mix(in srgb, var(--accent) 55%, var(--line))"></span>layer switch</div>
      <div class="item"><span class="swatch t-trans" style="background:var(--key-alt); border-style:dashed"></span>transparent (falls through)</div>
      <div class="item"><span class="swatch t-empty" style="border-style:dashed"></span>unassigned</div>
      <div class="item"><span class="swatch" style="background:color-mix(in srgb, var(--danger) 30%, var(--key)); border-color:color-mix(in srgb, var(--danger) 60%, var(--line))"></span>firmware (boot/reset)</div>
      <div class="item"><span class="swatch" style="background:color-mix(in srgb, var(--accent) 20%, var(--key)); border-color:color-mix(in srgb, var(--accent) 45%, var(--line))"></span>toggle</div>
    </div>
  </section>

  <div class="notes">
    <div class="note">
      <h3>Tri&#8209;layer stack</h3>
      <p>Holding <code>Lower</code> and <code>Raise</code> together activates <code>Adjust</code> &mdash; standard QMK tri&#8209;layer behaviour via <code>update_tri_layer_state</code>.</p>
    </div>
    <div class="note">
      <h3>Numeric is standalone</h3>
      <p><code>Numeric</code> is entered and left with <code>TO()</code>, not held. <code>layer_state_set_user</code> excludes it from the tri&#8209;layer check so it never gets clobbered by Lower+Raise.</p>
    </div>
    <div class="note">
      <h3>Dual&#8209;role thumb keys</h3>
      <p><code>Esc</code>/<code>Alt</code> and <code>Space</code>/<code>Gui</code> are mod&#8209;taps. The inner thumb key is a tap&#8209;dance: tap for <code>Space</code>, double&#8209;tap for <code>Enter</code>.</p>
    </div>
  </div>

  <footer>generated from keymap.c &middot; positions from light/keyboard.json &middot; regenerate with gen_keymap_viz.py</footer>
</div>

<script>
const UNIT = 54;

const POSITIONS = __POSITIONS__;

const LAYERS = __LAYERS__;

const LAYER_META = __LAYER_META__;

// token -> { main, sub, type }
function labelFor(tok){
  if (tok === "KC_NO") return { main:"", sub:"", type:"empty" };
  if (tok === "KC_TRNS") return { main:"▽", sub:"", type:"trans" };

  let m;
  if (m = tok.match(/^LALT_T\((.+)\)$/)) return { main:kc(m[1]), sub:"Alt", type:"dual" };
  if (m = tok.match(/^LGUI_T\((.+)\)$/)) return { main:kc(m[1]), sub:"Gui", type:"dual" };
  if (m = tok.match(/^LCTL_T\((.+)\)$/)) return { main:kc(m[1]), sub:"Ctrl", type:"dual" };
  if (m = tok.match(/^LSFT_T\((.+)\)$/)) return { main:kc(m[1]), sub:"Shift", type:"dual" };
  if (tok === "TD(TAP_SPC_ENT)") return { main:"Space", sub:"→ Enter", type:"dual" };
  if (tok === "LOWER") return { main:"Lower", sub:"", type:"layer", hue:"var(--hue-lower)" };
  if (tok === "RAISE") return { main:"Raise", sub:"", type:"layer", hue:"var(--hue-raise)" };
  if (tok === "TO(4)") return { main:"Numeric", sub:"→", type:"layer", hue:"var(--hue-numeric)" };
  if (tok === "TO(0)") return { main:"Base", sub:"→", type:"layer", hue:"var(--hue-lower)" };
  if (tok === "QK_BOOT") return { main:"BOOT", sub:"", type:"danger" };
  if (tok === "QK_RBT") return { main:"RESET", sub:"", type:"danger" };
  if (tok === "CW_TOGG") return { main:"Caps", sub:"Word", type:"toggle" };

  if (tok.startsWith("MS_")) return { main: MOUSE[tok] || tok, sub:"", type:"mouse" };
  if (["KC_MPLY","KC_MPRV","KC_MNXT","KC_MRWD","KC_MFFD","KC_MSTP","KC_VOLU","KC_VOLD"].includes(tok))
    return { main: MEDIA[tok] || tok, sub:"", type:"media" };
  if (tok.match(/^KC_P[0-9]$/) || ["KC_PSLS","KC_PAST","KC_PMNS","KC_PPLS","KC_PCMM","KC_PDOT","KC_PEQL","KC_NUM"].includes(tok))
    return { main: NUMPAD[tok] || tok, sub:"", type:"numpad" };
  if (["KC_PSCR","KC_PAUS","KC_INS","KC_HOME","KC_END","KC_PGUP","KC_PGDN","KC_APP","KC_LEFT","KC_RGHT","KC_UP","KC_DOWN","KC_DEL"].includes(tok))
    return { main: NAV[tok] || tok, sub:"", type: (tok==="KC_DEL"?"mod":"nav") };
  if (["KC_LCTL","KC_RCTL","KC_LALT","KC_RALT","KC_LSFT","KC_RSFT","KC_TAB","KC_ESC","KC_BSPC","KC_ENT","KC_SPC"].includes(tok))
    return { main: kc(tok), sub:"", type:"mod" };
  if (tok.match(/^KC_F([0-9]|1[0-4])$/)) return { main: tok.replace("KC_",""), sub:"", type:"mod" };

  return { main: kc(tok), sub:"", type:"alpha" };
}

const MOUSE = {
  MS_LEFT:"M ←", MS_RGHT:"M →", MS_UP:"M ↑", MS_DOWN:"M ↓",
  MS_BTN1:"Click", MS_BTN2:"Right", MS_BTN3:"Mid",
  MS_WHLU:"Wheel↑", MS_WHLD:"Wheel↓",
  MS_ACL0:"Spd 0", MS_ACL1:"Spd 1", MS_ACL2:"Spd 2"
};
const MEDIA = {
  KC_MPLY:"▶ Play", KC_MPRV:"⏮ Prev", KC_MNXT:"⏭ Next",
  KC_MRWD:"⏪ RWD", KC_MFFD:"⏩ FFD", KC_MSTP:"⏹ Stop",
  KC_VOLU:"Vol +", KC_VOLD:"Vol −"
};
const NUMPAD = {
  KC_P0:"0", KC_P1:"1", KC_P2:"2", KC_P3:"3", KC_P4:"4", KC_P5:"5",
  KC_P6:"6", KC_P7:"7", KC_P8:"8", KC_P9:"9",
  KC_PSLS:"/", KC_PAST:"×", KC_PMNS:"−", KC_PPLS:"+",
  KC_PCMM:",", KC_PDOT:".", KC_PEQL:"=", KC_NUM:"NumLk"
};
const NAV = {
  KC_PSCR:"PrtSc", KC_PAUS:"Pause", KC_INS:"Ins",
  KC_HOME:"Home", KC_END:"End", KC_PGUP:"PgUp", KC_PGDN:"PgDn", KC_APP:"Menu",
  KC_LEFT:"←", KC_RGHT:"→", KC_UP:"↑", KC_DOWN:"↓", KC_DEL:"Del"
};
const KC = {
  KC_GRV:"`", KC_1:"1", KC_2:"2", KC_3:"3", KC_4:"4", KC_5:"5", KC_6:"6", KC_7:"7", KC_8:"8", KC_9:"9", KC_0:"0",
  KC_BSPC:"⌫", KC_TAB:"↹ Tab", KC_Q:"Q", KC_W:"W", KC_E:"E", KC_R:"R", KC_T:"T", KC_Y:"Y", KC_U:"U", KC_I:"I", KC_O:"O", KC_P:"P",
  KC_BSLS:"\\", KC_ESC:"Esc", KC_A:"A", KC_S:"S", KC_D:"D", KC_F:"F", KC_G:"G", KC_H:"H", KC_J:"J", KC_K:"K", KC_L:"L",
  KC_SCLN:";", KC_QUOT:"'", KC_LSFT:"Shift", KC_RSFT:"Shift", KC_Z:"Z", KC_X:"X", KC_C:"C", KC_V:"V", KC_B:"B",
  KC_MINS:"-", KC_EQL:"=", KC_N:"N", KC_M:"M", KC_COMM:",", KC_DOT:".", KC_SLSH:"/",
  KC_LCTL:"Ctrl", KC_RCTL:"Ctrl", KC_LALT:"Alt", KC_RALT:"AltGr", KC_LBRC:"[", KC_RBRC:"]",
  KC_SPC:"Space", KC_ENT:"⏎ Enter"
};
function kc(tok){ return KC[tok] || tok.replace("KC_",""); }

let active = Object.keys(LAYER_META)[0];
try { active = localStorage.getItem("lily58-active-layer") || active; } catch(e) {}
if (!(active in LAYER_META)) active = Object.keys(LAYER_META)[0];

function buildTabs(){
  const nav = document.getElementById("tabs");
  nav.innerHTML = "";
  Object.keys(LAYER_META).forEach(key => {
    const meta = LAYER_META[key];
    const b = document.createElement("button");
    b.className = "tab";
    b.dataset.active = (key === active);
    b.style.setProperty("--tab-hue", meta.hue || "var(--accent)");
    b.innerHTML = '<span class="n">0'+meta.n+'</span>'+meta.label;
    b.addEventListener("click", () => { active = key; try{localStorage.setItem("lily58-active-layer", key);}catch(e){} render(); });
    nav.appendChild(b);
  });
}

function render(){
  document.querySelectorAll(".tab").forEach((el, i) => {
    const key = Object.keys(LAYER_META)[i];
    el.dataset.active = (key === active);
  });
  const meta = LAYER_META[active];
  document.getElementById("panel-title").textContent = meta.label;
  document.getElementById("panel-title").style.color = meta.hue || "var(--ink)";
  document.getElementById("panel-desc").textContent = meta.desc;

  const board = document.getElementById("board");
  board.innerHTML = "";
  let maxX = 0, maxY = 0;
  const toks = LAYERS[active];
  POSITIONS.forEach((pos, i) => {
    const [x,y,w=1,h=1] = pos;
    maxX = Math.max(maxX, x + w);
    maxY = Math.max(maxY, y + h);
    const info = labelFor(toks[i]);
    const div = document.createElement("div");
    div.className = "key t-" + info.type;
    if (info.hue) div.style.setProperty("--layer-hue", info.hue);
    else if (info.type === "layer") div.style.setProperty("--layer-hue", meta.hue || "var(--accent)");
    div.style.left = (x*UNIT) + "px";
    div.style.top = (y*UNIT) + "px";
    div.style.width = (w*UNIT - 6) + "px";
    div.style.height = (h*UNIT - 6) + "px";
    div.innerHTML = '<span class="main">'+info.main+'</span>' + (info.sub ? '<span class="sub">'+info.sub+'</span>' : '');
    board.appendChild(div);
  });
  board.style.width = (maxX*UNIT) + "px";
  board.style.height = (maxY*UNIT) + "px";
}

buildTabs();
render();
</script>
"""

if __name__ == "__main__":
    main()
