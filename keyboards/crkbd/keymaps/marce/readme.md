# Corne v4 (marce)

A port of the [Lily58L `marce` keymap](../../../lily58/keymaps/marce/) onto a
genuine foostan Corne v4 (RP2040, `crkbd/rev4_1`, `LAYOUT_split_3x6_3`, 42
keys). Originally flashed with Vial firmware from the factory; this replaces
that with stock QMK.

Same layer structure, tap dance, digitizer point-jump keys, and the Mac/PC
AltGr Unicode-fake switch as the Lily58 keymap. See that keymap's `keymap.c`
for the detailed design notes; see this one's for what changed to fit 42
keys instead of 58 (no number row, only 3 thumb keys/side, Ctrl moved onto
Z/Slash as a mod-tap).

Hardware notes:
- Rotary encoders are real on this board (unlike Lily58L, where the encoder
  code was dead since `ENCODER_ENABLE` was never turned on) -- left encoder
  does volume / media prev-next on RAISE, right encoder does up/down /
  left/right on LOWER, same as the Lily58 mapping.
- OLED support is present in the code but `OLED_ENABLE=no` by default since
  it's unconfirmed whether this specific board has OLED modules installed.
  Flip it to `yes` in `rules.mk` if it does.
- RGB is `rgb_matrix` (per-key), not `rgblight` (underglow strip) -- this
  board's `keyboard.json` already configures it, no keymap-side setup
  needed.

Make example for this keyboard (after setting up your build environment):

    make crkbd/rev4_1:marce

Flashing example for this keyboard (put the board in bootloader mode first
-- double-tap reset on the RP2040, or hold BOOTSEL while plugging in):

    make crkbd/rev4_1:marce:flash

If key positions seem off after flashing, this board might actually be a
`rev4_0` instead of `rev4_1` -- try substituting that revision in both
commands above.

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools)
and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide)
for more information.
