# CLAUDE.md

Personal QMK keymap `keyboards/beekeeb/piantor/keymaps/marce` (Piantor, RP2040, 42 keys, `LAYOUT_split_3x6_3`). It is a
port of `keyboards/crkbd/keymaps/marce` (which is itself a port of lily58's); logic common to the boards lives in
`users/marce/` (see its README.md and PLAN.md). Read those before changing anything shared.

## Files

- `keymap.c` — layers (`keymaps[]`), Piantor-only keycode `CW_CTL`, `process_record_marce_kb`, `matrix_scan_user`
  (CW_CTL timer). No OLED, encoders or RGB: the board has none.
- `rules.mk` — Caps Word, Repeat Key, `USER_NAME := marce`. No `config.h`; shared defines come from `users/marce/`.
- `gen_keymap_viz.py` -> `keymap-viz.html`, `flash.sh` (compile in the toolbox, copy .uf2 to the RPI-RP2 drive on the host).

Build: `qmk compile -kb beekeeb/piantor -km marce`. No tests; verify by building and flashing.

## Corne -> Piantor mapping

The Piantor is the Corne without its two "ex2" keys per side, so every row and thumb is key-for-key identical.

| Corne | Piantor |
|---|---|
| `NUM_LAYER` on the top-row left ex2 key | replaces `KC_RSFT` (base, bottom-right outer); `KC_NO` there on LOWER/RAISE/ADJUST |
| `TO(0)` on the `_NUMERIC` home-row left ex2 key | `_NUMERIC` bottom-right outer (where `KC_RSFT` was) |
| repeat / alt repeat on the home-row ex2 keys (base) | `_RAISE` top row, right of `KC_APP` |
| RGB layer colour, OLED, encoders | dropped (hardware absent) |

## Invariants

- `_NUMERIC` must stay below `_LOWER`/`_RAISE`/`_ADJUST` (defined in `users/marce/marce.h`).
- Edit shared behavior in `users/marce/`, then build **all** boards.
