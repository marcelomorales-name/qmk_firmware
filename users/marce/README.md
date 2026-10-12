# users/marce

Logic shared by the `marce` keymaps of `keyboards/lily58` and `keyboards/crkbd`. Each keymap opts in with
`USER_NAME := marce` in its `rules.mk`; QMK then builds `marce.c` and puts this directory on the include path.
Migration history and rationale: `PLAN.md`.

## What lives where

| Here (`users/marce/`) | In each `keymap.c` |
|---|---|
| `enum layers`, `LOWER`/`RAISE` (`marce.h`) | the `keymaps[]` arrays (the physical layouts differ: 58 vs 46 keys) |
| shared keycodes: `ALTGR_MC`, `NUMGRID_*`, `NUM_GRD_F/G`, `DIG_*` | board-only keycodes, in an enum starting at `MARCE_SAFE_RANGE` |
| `process_record_user` dispatcher | `process_record_marce_kb` for the board-only keycodes |
| Mac/PC AltGr + macOS Unicode input, digitizer grid, tri-layer logic, hold-vs-tap helpers | OLED, encoders, RGB matrix, `matrix_scan_user`, `keyboard_post_init_user` |
| `MARCE_TAP_DANCE_ACTIONS` macro | the `tap_dance_actions[]` table that expands it |
| `config.h`, `rules.mk` for what both boards use | anything board-specific |

## Hooks (weak, override in a keymap.c)

- `bool process_record_marce_kb(keycode, record)` — runs first; return `false` to swallow the key. The shared
  handlers (DIG, AltGr) only see keys it lets through.
- `layer_state_t layer_state_set_marce_kb(state)` — runs after the LOWER+RAISE -> ADJUST update.
- `void mac_altgr_mode_changed_marce_kb(void)` — after `ALTGR_MC` toggles `mac_altgr_mode`.

## Rules to keep

- `_NUMERIC` stays below `_LOWER`/`_RAISE`/`_ADJUST` in `enum layers`.
- `NUMGRID_1..NUMGRID_0` and `DIG_TL..DIG_BR` stay contiguous and in order; handlers derive indexes from the offset.
- `tap_dance_actions[]` must be defined in `keymap.c` (QMK keymap introspection), hence the macro.
- Don't put `USER_NAME := marce # comment` on one line: the trailing space ends up in the value and the build fails with
  "users/marce: Is a directory".
- Only move a feature flag into the shared `rules.mk` if both boards already enable it. Lily58 is AVR with ~10 KB free;
  sharing `CAPS_WORD_ENABLE` cost it 380 B for a feature it doesn't use.

## Building

```
qmk compile -kb lily58/rev1 -km marce            # AVR, 28672 B limit
qmk compile -kb crkbd/rev4_1/standard -km marce  # RP2040
```

Baselines after the migration: lily58 18500 B, crkbd text 50908 B.
