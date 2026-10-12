# Shared userspace for the lily58 and crkbd "marce" keymaps. Pulled in by
# USER_NAME := marce in each keymap's rules.mk.
#
# Only features both boards use live here; anything board-specific (OLED,
# encoders, EXTRAKEY, the flash-saving "= no" overrides on lily58, ...) stays
# in that keymap's rules.mk.
SRC += marce.c

DIGITIZER_ENABLE = yes
LTO_ENABLE = yes
MOUSEKEY_ENABLE = yes
TAP_DANCE_ENABLE = yes # TAP_SPC_ENT
