# Feature flags for this keymap.
#
# Flags set to "no" here override the keyboard-level defaults from
# keyboards/lily58/light/keyboard.json: those are emitted with "?=" into a
# generated rules.mk that is included *before* this file, so this one wins.
# Everything off below is off to save flash on the atmega32u4 (~28K usable
# after the Caterina bootloader).

BOOTMAGIC_ENABLE = no   # kb default: yes. No hold-a-key-on-plug bootloader
                        # escape any more -- use the Pro Micro reset pads.
ENCODER_ENABLE = no     # kb default: yes. No encoders fitted.
RGBLIGHT_ENABLE = no    # kb default: yes. The keymap has no RGB keycodes, so
                        # this was paying for the driver plus every animation
                        # enabled in config.h.
EXTRAKEY_ENABLE = no    # The media keys in encoder_update_user were the only
                        # consumers; re-enable this if the encoders come back.
GRAVE_ESC_ENABLE = no   # Defaults to yes. No QK_GESC in the keymap.
MAGIC_ENABLE = no       # Defaults to yes. No magic keycodes in the keymap.
SPACE_CADET_ENABLE = no # Defaults to yes. No SC_* in the keymap.
UNICODE_ENABLE = no     # macOS Unicode Hex Input is hand-rolled in keymap.c.

DIGITIZER_ENABLE = yes  # absolute-position point grid on _ADJUST
LTO_ENABLE = yes
MOUSEKEY_ENABLE = yes
OLED_ENABLE = yes
TAP_DANCE_ENABLE = yes  # Only TAP_SPC_ENT still needs it.
