// Copyright 2022 beekeeb
// SPDX-License-Identifier: GPL-2.0-or-later

// Ported from keyboards/crkbd/keymaps/marce. The Piantor is LAYOUT_split_3x6_3
// (42 keys): the Corne's 46 minus the two "ex2" keys per side. Deliberate
// changes made during the port:
//   - NUM_LAYER (the Corne's top-row left ex2 key) is gone: _NUMERIC is
//     entered by holding Q (LT(_NUMERIC, KC_Q), as on Lily58), and T on
//     _NUMERIC is Ctrl+Shift+Gui when held. The bottom-right outer key is a
//     plain KC_RSFT. _NUMERIC's copy of that spot is TO(0), an exit key for
//     when _NUMERIC was entered via TO(_NUMERIC) on _ADJUST.
//   - Repeat / alt repeat (home-row ex2 keys on the Corne) moved to _RAISE's
//     two free right-hand top-row keys, next to PrtSc/Pause/Menu.
//   - The bottom-right outer key is KC_NO (not KC_TRNS) on _LOWER/_RAISE/
//     _ADJUST so Shift doesn't fall through there.
//   - No encoders, RGB matrix or OLED on this board, so the Corne's code for
//     those is dropped.
// Everything else (rows, thumbs, _NUMERIC grid with the NUM_GRD_F/G overlay)
// is key-for-key the Corne layout. Shared logic lives in users/marce/.

#include QMK_KEYBOARD_H
#include "marce.h"

// Board-only keycodes; the shared ones live in users/marce/marce.h.
enum custom_keycodes {
    CW_CTL = MARCE_SAFE_RANGE, // tap: toggle Caps Word. hold: Ctrl. CW_TOGG isn't a basic
                               // keycode so it can't use the built-in xxxx_T() mod-tap macros;
                               // handled by hand in process_record_marce_kb/matrix_scan_user below.
};

// Tap Dance definitions (shared entries live in users/marce/marce.h)
tap_dance_action_t tap_dance_actions[] = {MARCE_TAP_DANCE_ACTIONS};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY.
    [_QWERTY] = LAYOUT_split_3x6_3(KC_TAB, LT(_NUMERIC, KC_Q), KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,                                    // top row
                                   LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT),                 // home row
                                   LSFT_T(KC_GRV), LCTL_T(KC_Z), KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, RCTL_T(KC_SLSH), KC_RSFT, // bottom row
                                   KC_LALT, LOWER, LGUI_T(KC_SPC), TD(TAP_SPC_ENT), RAISE, KC_RALT                                                // thumbs
                                   ),

    // Lower layer: numbers-less symbols and mouse keys (held via LOWER).
    [_LOWER] = LAYOUT_split_3x6_3(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_TRNS, // top row
                                  KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_BSLS, KC_TRNS, // home row
                                  KC_TRNS, CW_CTL, KC_NO, MS_BTN2, MS_BTN3, MS_BTN1, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, KC_NO, KC_NO, // bottom row
                                  KC_TRNS, KC_TRNS, KC_SPC, KC_ENT, KC_TRNS, KC_TRNS                                              // thumbs
                                  ),

    // Raise layer: function keys, navigation (held via RAISE).
    [_RAISE] = LAYOUT_split_3x6_3(KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_PSCR, KC_PAUS, KC_APP, QK_REPEAT_KEY, QK_ALT_REPEAT_KEY, KC_TRNS, // top row
                                  KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_INS, KC_TRNS,                 // home row
                                  KC_TRNS, LCTL_T(KC_F11), KC_F12, KC_F13, KC_F14, KC_F15, KC_HOME, KC_END, KC_PGUP, KC_PGDN, LCTL_T(KC_DEL), KC_NO, // bottom row
                                  KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_TRNS                                                                // thumbs
                                  ),

    // Adjust layer: media keys (LOWER+RAISE). Right half hosts the
    // digitizer point grid, one physical row per grid row.
    [_ADJUST] = LAYOUT_split_3x6_3(KC_TRNS, KC_NO, ALTGR_MC, KC_NO, TO(_NUMERIC), KC_KB_POWER, DIG_TL, DIG_TC, DIG_TR, KC_NO, KC_NO, KC_TRNS, // top row
                                   KC_TRNS, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, DIG_ML, DIG_MC, DIG_MR, KC_NO, KC_NO, KC_TRNS,           // home row
                                   KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, DIG_BL, DIG_BC, DIG_BR, KC_NO, KC_NO, KC_NO,                    // bottom row
                                   KC_NO, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_NO                                                                // thumbs
                                   ),

    // Numeric layer: the shared NUMGRID_1..9,0 grid on the right hand (number-
    // row digits, not real numpad keycodes, so host Num Lock doesn't matter),
    // arrows on the left. NUM_GRD_F/NUM_GRD_G (hold -> retarget the grid to
    // F-keys, tap -> F/G) sit on the home row's operator column. TO(0) on the
    // bottom-right outer key leaves the layer (that spot is RSFT on base).
    [_NUMERIC] = LAYOUT_split_3x6_3(KC_TAB, LT(_NUMERIC, KC_Q), KC_UP, KC_NO, KC_PAST, MT(MOD_LCTL | MOD_LSFT | MOD_LGUI, KC_T), NUMGRID_7, NUMGRID_8, NUMGRID_9, KC_PPLS, KC_NO, KC_TRNS,   // top row
                                    KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, NUM_GRD_F, NUM_GRD_G, NUMGRID_4, NUMGRID_5, NUMGRID_6, KC_PMNS, KC_NO, KC_TRNS, // home row
                                    KC_TRNS, KC_LCTL, KC_LALT, KC_NO, KC_NO, KC_PEQL, NUMGRID_1, NUMGRID_2, NUMGRID_3, KC_PDOT, KC_RCTL, TO(0),    // bottom row
                                    KC_TRNS, LOWER, KC_SPC, KC_ENT, RAISE, NUMGRID_0                                                                // thumbs
                                    ),
};

// --- CW_CTL: Caps Word tap, Ctrl hold -----------------------------------
//
// CW_TOGG isn't a basic keycode, so it can't be wrapped in a built-in
// xxxx_T() mod-tap macro (those only work on basic keycodes). Reimplements
// the same tap-vs-hold resolution by hand: holding past TAPPING_TERM, or
// pressing any other key while still held, resolves to Ctrl; releasing
// before either of those toggles Caps Word instead.
static bool     cw_ctl_held    = false; // CW_CTL currently down, unresolved
static bool     cw_ctl_is_ctrl = false; // resolved as a Ctrl hold
static uint16_t cw_ctl_timer   = 0;

void matrix_scan_user(void) {
    if (cw_ctl_held && !cw_ctl_is_ctrl && timer_elapsed(cw_ctl_timer) > TAPPING_TERM) {
        register_code(KC_LCTL);
        cw_ctl_is_ctrl = true;
    }
}

bool process_record_marce_kb(uint16_t keycode, keyrecord_t *record) {
    // Resolve a held CW_CTL as Ctrl the moment any other key is pressed.
    if (cw_ctl_held && !cw_ctl_is_ctrl && keycode != CW_CTL && record->event.pressed) {
        register_code(KC_LCTL);
        cw_ctl_is_ctrl = true;
    }

    if (keycode == CW_CTL) {
        if (record->event.pressed) {
            cw_ctl_held    = true;
            cw_ctl_is_ctrl = false;
            cw_ctl_timer   = timer_read();
        } else {
            cw_ctl_held = false;
            if (cw_ctl_is_ctrl) {
                unregister_code(KC_LCTL);
                cw_ctl_is_ctrl = false;
            } else {
                caps_word_toggle();
            }
        }
        return false;
    }

    return true;
}
