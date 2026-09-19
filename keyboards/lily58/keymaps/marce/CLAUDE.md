# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this directory is

This is a personal QMK keymap (`keyboards/lily58/keymaps/marce`) inside the upstream `qmk_firmware` monorepo, for a modified Lily58 PCB ("Lily58L") with per-key RGB, underglow, and dual rotary encoders. It contains only:

- `keymap.c` — layers, tap dance, OLED rendering, encoder handling
- `config.h` — RGB effect selection and split-USB settings
- `rules.mk` — feature flags for this keymap (caps word, extra keycodes, tap dance, mouse keys, LTO)
- `readme.md` — hardware description and build/flash commands

The keyboard's own hardware definition (matrix, pins, `LAYOUT` macro) lives one level up in `keyboards/lily58/` (or `keyboards/lily58/light/`) and is not part of this keymap.

## Build and flash

From the root of the qmk_firmware repo (not from this directory):

```
make lily58/light:marce
make lily58/light:marce:flash
```

Substitute `light` for whatever lily58 revision is actually being targeted if different from the readme's example. Building requires the QMK build environment (`qmk setup` / the AVR-GCC toolchain) to already be configured — see QMK's own docs, not this repo, for that setup.

There is no keymap-local test suite; correctness is verified by building and flashing.

## Architecture

**Layers** (`enum layers` in keymap.c): `_QWERTY` (base) → `_LOWER` / `_RAISE` (momentary, held via `LOWER`/`RAISE` macros) → `_ADJUST` (tri-layer: active when LOWER+RAISE are both held) → `_NUMERIC` (standalone numpad layer, entered/exited via `TO(4)`/`TO(0)`, not part of the tri-layer stack).

`layer_state_set_user` special-cases `_NUMERIC` so it's excluded from `update_tri_layer_state`'s LOWER+RAISE→ADJUST logic — this is the key invariant to preserve if layers are added or reordered.

**Tap dance**: declared via the `enum { TAP_SPC_ENT, ... }` + `tap_dance_actions[]` pair — add new tap-dance behaviors by extending both in lockstep.

**OLED status display** (guarded by `#ifdef OLED_ENABLE`, not currently enabled in `rules.mk`): renders layer name, lock state, mod state, and a rolling keylogger (`add_keylog`/`code_to_name`) on the master half, and the Lily58 logo bitmap on the slave half. `oled_init_user` rotates the two halves differently. `process_record_user` feeds every keypress into the keylogger.

**Rotary encoders** (guarded by `#ifdef ENCODER_ENABLE`, not currently enabled): `encoder_update_user` dispatches by `index` (0 = master/left side, 1 = slave/right side) and by which layer is currently held — left encoder does volume by default / media prev-next on RAISE; right encoder does up/down by default / left-right on LOWER.

Both `OLED_ENABLE` and `ENCODER_ENABLE` need to be turned on in `rules.mk` (and matching hardware config in `config.h`/the keyboard-level files) for that code to actually compile in — right now those blocks are dead code under the current `rules.mk`.
