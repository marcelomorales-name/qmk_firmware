#pragma once

#include "quantum.h"

// Shared definitions for the lily58 and crkbd "marce" keymaps. Anything that
// depends on a board's physical layout stays in that board's keymap.c.

// _NUMERIC must stay below _LOWER/_RAISE/_ADJUST: QMK resolves keys from the
// highest active layer down, so LOWER/RAISE held while _NUMERIC is on only
// overlay it (instead of being shadowed by it) if they outrank it here.
// layer_state_set_user also relies on this to keep _NUMERIC out of the
// LOWER+RAISE -> _ADJUST tri-layer logic.
enum layers {
    _QWERTY,
    _NUMERIC,
    _LOWER,
    _RAISE,
    _ADJUST,
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

// Tap Dance declarations. Only the space/enter double-tap still needs the tap
// dance framework -- it's the one behavior here that genuinely keys off a
// second tap. The hold-vs-tap keys are cheaper as custom keycodes.
enum {
    TAP_SPC_ENT,
};

// QMK's keymap introspection requires tap_dance_actions[] itself to be defined
// in keymap.c, so the shared entries are a macro each keymap expands in its own
// table (and can extend with board-only entries):
//     tap_dance_action_t tap_dance_actions[] = {MARCE_TAP_DANCE_ACTIONS};
#define MARCE_TAP_DANCE_ACTIONS \
    /* Tap once for Space, twice for Enter */ \
    [TAP_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT),

// Keycodes common to both boards. Each keymap defines its own board-only
// keycodes in a separate enum starting at MARCE_SAFE_RANGE.
enum marce_keycodes {
    ALTGR_MC = SAFE_RANGE, // toggles AltGr between PC-passthrough and Mac Unicode-fake modes
    // _NUMERIC's numpad-grid cells, numbered 1-9 like the numpad they
    // stand in for, plus 0 (on a thumb key rather than the grid proper).
    // Handled in process_record_user: plain digit normally, or an F-key
    // while NUM_GRD_F/NUM_GRD_G is held (see numeric_grid_mode).
    // Must stay contiguous and in order -- including NUMGRID_0 landing right
    // after NUMGRID_9 -- since process_record_user range-checks them and
    // derives the digit from the offset (0 lands at offset 9, i.e. digit 10,
    // which is exactly the numpad-style F10/F20 it should produce).
    NUMGRID_1,
    NUMGRID_2,
    NUMGRID_3,
    NUMGRID_4,
    NUMGRID_5,
    NUMGRID_6,
    NUMGRID_7,
    NUMGRID_8,
    NUMGRID_9,
    NUMGRID_0,
    NUM_GRD_F, // tap: F; hold: numeric_grid_mode = GRID_F
    NUM_GRD_G, // tap: G; hold: numeric_grid_mode = GRID_G
    // Digitizer point keys: each moves the digitizer cursor to a fixed absolute
    // position on screen without clicking, mirroring the numpad's spatial layout
    // (7=top-left, 5=center, 3=bottom-right, ...). Must stay contiguous and in
    // this order: the handler indexes digitizer_points[] by keycode - DIG_TL.
    DIG_TL,
    DIG_TC,
    DIG_TR,
    DIG_ML,
    DIG_MC,
    DIG_MR,
    DIG_BL,
    DIG_BC,
    DIG_BR,
    MARCE_SAFE_RANGE, // first keycode available to a board's own enum
};
