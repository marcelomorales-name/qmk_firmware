# CLAUDE.md

Personal QMK keymap `keyboards/crkbd/keymaps/marce` (Corne, `crkbd/rev4_1/standard`, RP2040, 46 keys,
`LAYOUT_split_3x6_3_ex2`). It is a port of `keyboards/lily58/keymaps/marce`; logic common to both boards lives in
`users/marce/` (see its README.md and PLAN.md). Read those before changing anything shared.

## Files

- `keymap.c` — layers (`keymaps[]`), Corne-only keycodes (`CW_CTL`, `NUM_MO`, `NUM_OS`), RGB layer color, OLED, encoders,
  `process_record_marce_kb`, `matrix_scan_user` (CW_CTL timer).
- `rules.mk`, `config.h` — Corne-only settings; shared ones come from `users/marce/`.
- `gen_keymap_viz.py` -> `keymap-viz.html`, `flash.sh` (compile in the toolbox, copy .uf2 to the RPI-RP2 drive on the host).

Build: `qmk compile -kb crkbd/rev4_1/standard -km marce`. No tests; verify by building and flashing.

## Lily58 -> Corne mapping

| Lily58 | Corne |
|---|---|
| number row (58 keys: 12/12/12/14/8) | gone; digits live on `_NUMERIC` |
| backslash on base top row | gone from base; on `_LOWER` home row |
| dedicated Ctrl thumb key | Ctrl is a mod-tap on Z and `/` (and `CW_CTL` on `_LOWER`) |
| 4 thumb keys/side | 3: Alt, LOWER, Space/Enter tap dance, RAISE, AltGr |
| `NUM_LAYER` (tap one-shot / hold momentary) | `NUM_MO` + `NUM_OS` on the ex2 columns; both together lock `_NUMERIC` (exit with `TO(0)`) |
| bottom-row innermost extra column | ex2 columns above the inner thumb keys: NUM_MO/NUM_OS (top), repeat/alt-repeat (home) |
| `NUMGRID_*` F/G overlay on `_NUMERIC` | not ported yet (PLAN.md phase 4b); `_NUMERIC` is still a `KC_P*` numpad |
| Z/X/Slash/Dot mod-taps | only Z and Slash are mod-taps |

## Invariants

- `_NUMERIC` must stay below `_LOWER`/`_RAISE`/`_ADJUST` (defined in `users/marce/marce.h`).
- Layer order must match `layer_val[]` (RGB brightness) and the OLED digit table.
- Edit shared behavior in `users/marce/`, then build **both** boards.
