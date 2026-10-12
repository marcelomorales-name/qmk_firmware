# Shared userspace for the lily58 and crkbd "marce" keymaps. Pulled in by
# USER_NAME := marce in each keymap's rules.mk.
#
# Feature flags stay in the per-keymap rules.mk for now (see PLAN.md, phase 5);
# only the shared source is registered here.
SRC += marce.c
