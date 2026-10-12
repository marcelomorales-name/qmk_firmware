#pragma once

// Config shared by the lily58 and crkbd "marce" keymaps.

// Sync layer_state to the slave half so its OLED can show the real layer
// instead of always reading 0 (_QWERTY). Harmless if OLED_ENABLE is off.
#define SPLIT_LAYER_STATE_ENABLE

// Turn the OLEDs off after a minute of inactivity, back on on next keypress.
// Only takes effect if OLED_ENABLE=yes in rules.mk.
#define OLED_TIMEOUT 60000
