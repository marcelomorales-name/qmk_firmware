/* Copyright 2017 F_YUUCHI
 * Copyright 2020 Drashna Jaelre <@drashna>
 * Copyright 2020 Ben Roesner (keycapsss.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "marce.h"

// Board-only keycodes; the shared ones live in users/marce/marce.h.
enum custom_keycodes {
    // Hold-vs-tap key, resolved in process_record_user (see hold_key_press in users/marce/marce.h).
    NUM_LAYER = MARCE_SAFE_RANGE, // tap: one-shot _NUMERIC (next key only); hold: momentary _NUMERIC
};

// Holding F or G on _NUMERIC retargets the numpad grid (NUMGRID_1..9) to
// F-keys instead of switching layers: GRID_NONE types plain digits,
// GRID_F/GRID_G pick the F1-F9/F11-F19 bank (see process_record_user).
enum { GRID_NONE = 0, GRID_F, GRID_G };
static uint8_t numeric_grid_mode = GRID_NONE;

// Tap Dance definitions (shared entries live in users/marce/marce.h)
tap_dance_action_t tap_dance_actions[] = {MARCE_TAP_DANCE_ACTIONS};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY
    [_QWERTY] = LAYOUT(KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,                                            // number row
                       KC_TAB, LT(_NUMERIC, KC_Q), KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,                              // top row
                       LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT),                         // home row
                       KC_LSFT, LCTL_T(KC_Z), LALT_T(KC_X), KC_C, KC_V, KC_B, NUM_LAYER, KC_NO, KC_N, KC_M, KC_COMM, RALT_T(KC_DOT), RCTL_T(KC_SLSH), KC_RSFT, // bottom row
                       KC_LCTL, KC_LALT, LOWER, LGUI_T(KC_SPC), TD(TAP_SPC_ENT), RAISE, KC_RALT, RCTL_T(KC_RGUI)                               // thumbs
                       ),

    // Lower layer: numbers, symbols, mouse keys (held via LOWER)
    [_LOWER] = LAYOUT(KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_TRNS,                                  // number row
                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_TRNS,                            // top row
                      KC_TRNS, KC_NO, TG(_NUMERIC), KC_NO, KC_NO, KC_NO, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_NO, KC_TRNS,                     // home row
                      KC_TRNS, LCTL_T(KC_NO), LALT_T(KC_NO), MS_BTN2, MS_BTN3, MS_BTN1, MS_WHLU, MS_WHLD, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, RCTL_T(KC_NO), KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                                                    // thumbs
                      ),

    // Raise layer: navigation (held via RAISE). Function keys used to live
    // here but moved to the numeric layer's F/G grid overlay (no extra layer:
    // see numeric_grid_mode).
    [_RAISE] = LAYOUT(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS,                               // number row
                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_PSCR, KC_PAUS, KC_APP, KC_NO, KC_INS, KC_TRNS,                          // top row
                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_DEL, KC_TRNS,                        // home row
                      KC_TRNS, LCTL_T(KC_NO), LALT_T(KC_NO), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_HOME, KC_PGDN, KC_PGUP, RALT_T(KC_END), RCTL_T(KC_NO), KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                                                    // thumbs
                      ),

    // Adjust layer: reboot/bootloader, media keys (LOWER+RAISE). Right half
    // hosts a digitizer point grid at the numpad-analog positions.
    [_ADJUST] = LAYOUT(KC_TRNS, KC_NO, ALTGR_MC, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, // number row
                       KC_TRNS, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, DIG_TL, DIG_TC, DIG_TR, KC_NO, KC_NO, KC_TRNS, // top row
                       KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, DIG_ML, DIG_MC, DIG_MR, KC_NO, KC_NO, KC_TRNS,       // home row
                       KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, DIG_BL, DIG_BC, DIG_BR, KC_NO, KC_NO, KC_TRNS, // bottom row
                       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                            // thumbs
                       ),

    // Numeric layer: number-row digits/symbols (not numpad keycodes, so they
    // aren't affected by the host's Num Lock state) arranged in a numpad-style
    // grid on the right hand, WASD-as-arrows on the left.
    // Entered/exited by tapping NUM_LAYER (one-shot) or holding it (MO),
    // by holding Q on _QWERTY (LT), or via TG(_NUMERIC) on _LOWER for a
    // persistent toggle that this same key also switches back off.
    // LOWER/RAISE stay held-only escapes back to those layers, returning to
    // _NUMERIC on release. Holding F/G here retargets the NUMGRID_* cells
    // to F1-F9/F11-F19 instead of plain digits (see numeric_grid_mode and
    // process_record_user); no extra layer needed since it's the same nine
    // physical positions either way.
    // Holding T applies Ctrl+Shift+Gui as a modifier instead of typing T.
    // 0 sits on the right hand's third thumb key (mirrors KC_RALT's position
    // on _QWERTY) rather than in the 3x3 grid, same as a numpad's 0 key
    // living below the grid; it's still a NUMGRID_* cell so NUM_GRD_F/G
    // retarget it to F10/F20 exactly like the rest.
    [_NUMERIC] = LAYOUT(KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_NUM, KC_SLSH, LSFT(KC_8), KC_MINS, KC_NO, KC_TRNS,      // number row
                        KC_TRNS, LT(_NUMERIC, KC_Q), KC_UP, KC_E, KC_R, MT(MOD_LCTL | MOD_LSFT | MOD_LGUI, KC_T), NUMGRID_7, NUMGRID_8, NUMGRID_9, LSFT(KC_EQL), KC_NO, KC_TRNS, // top row
                        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, NUM_GRD_F, NUM_GRD_G, NUMGRID_4, NUMGRID_5, NUMGRID_6, KC_COMM, KC_NO, KC_TRNS, // home row
                        KC_TRNS, LCTL_T(KC_Z), LALT_T(KC_X), KC_C, KC_V, KC_B, KC_TRNS, KC_NO, NUMGRID_1, NUMGRID_2, NUMGRID_3, RALT_T(KC_EQL), RCTL_T(KC_NO), KC_TRNS, // bottom row
                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, NUMGRID_0, KC_TRNS                          // thumbs
                        ),
};

// Sent per grid cell so release unregisters exactly what press registered,
// even if numeric_grid_mode changes mid-hold. Ten cells: NUMGRID_1..NUMGRID_9
// plus NUMGRID_0.
static uint16_t numgrid_sent[10] = {0};

bool process_record_marce_kb(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case NUM_LAYER:
            if (record->event.pressed) {
                hold_key_press(keycode);
                layer_on(_NUMERIC);
            } else {
                layer_off(_NUMERIC);
                if (hold_key_was_tap(keycode)) {
                    set_oneshot_layer(_NUMERIC, ONESHOT_START);
                }
            }
            return false;

        case NUM_GRD_F:
        case NUM_GRD_G: {
            uint8_t mine = (keycode == NUM_GRD_F) ? GRID_F : GRID_G;
            if (record->event.pressed) {
                hold_key_press(keycode);
                numeric_grid_mode = mine;
            } else {
                // Only clear if we still own the mode: if the other grid key
                // took over mid-hold, it owns it now and gets to clear it.
                if (numeric_grid_mode == mine) {
                    numeric_grid_mode = GRID_NONE;
                }
                if (hold_key_was_tap(keycode)) {
                    tap_code(keycode == NUM_GRD_F ? KC_F : KC_G);
                }
            }
            return false;
        }
    }

    if (keycode >= NUMGRID_1 && keycode <= NUMGRID_0) {
        uint8_t idx = keycode - NUMGRID_1;
        if (record->event.pressed) {
            uint8_t  digit = idx + 1; // NUMGRID_0 is idx 9 -> digit 10, giving F10/F20 below
            uint16_t code;
            if (numeric_grid_mode == GRID_NONE) {
                code = KC_1 + idx; // KC_1..KC_9, KC_0 are contiguous in that order
            } else {
                uint8_t target = digit + (numeric_grid_mode == GRID_G ? 10 : 0);
                code            = (target <= 12) ? (KC_F1 + target - 1) : (KC_F13 + target - 13);
            }
            numgrid_sent[idx] = code;
            register_code16(code);
        } else if (numgrid_sent[idx]) {
            unregister_code16(numgrid_sent[idx]);
            numgrid_sent[idx] = 0;
        }
        return false;
    }

    return true;
}

// SSD1306 OLED update loop, make sure to enable OLED_ENABLE=yes in rules.mk
#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;
    } else {
        return OLED_ROTATION_0;
    }
}

// Big current-layer digit, drawn with filled rectangles instead of a fixed
// bitmap so it can be sized independently for each half's logical canvas
// (oled_write_pixel coordinates already account for oled_init_user's
// rotation: 32w x 128h on the master at ROTATION_270, 128w x 32h on the
// slave at ROTATION_0).
static void oled_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    for (uint8_t dx = 0; dx < w; dx++) {
        for (uint8_t dy = 0; dy < h; dy++) {
            oled_write_pixel(x + dx, y + dy, true);
        }
    }
}

#    define SEG_A (1 << 0)
#    define SEG_B (1 << 1)
#    define SEG_C (1 << 2)
#    define SEG_D (1 << 3)
#    define SEG_E (1 << 4)
#    define SEG_F (1 << 5)
#    define SEG_G (1 << 6)

// Segments lit per digit, indexed by layer number (_QWERTY.._ADJUST == 0..4).
static const uint8_t PROGMEM digit_segments[5] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0 _QWERTY
    SEG_B | SEG_C,                                 // 1 _NUMERIC
    SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,         // 2 _LOWER
    SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,         // 3 _RAISE
    SEG_F | SEG_G | SEG_B | SEG_C,                 // 4 _ADJUST
};

// Draws a 7-segment-style digit in the box [x,y]..[x+w,y+h], segment
// thickness t.
static void draw_big_digit(uint8_t digit, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t t) {
    if (digit >= 5) {
        return;
    }
    uint8_t segs   = pgm_read_byte(&digit_segments[digit]);
    uint8_t half_h = (h - t) / 2;

    if (segs & SEG_A) oled_fill_rect(x, y, w, t);                           // top
    if (segs & SEG_G) oled_fill_rect(x, y + half_h, w, t);                  // middle
    if (segs & SEG_D) oled_fill_rect(x, y + h - t, w, t);                   // bottom
    if (segs & SEG_F) oled_fill_rect(x, y, t, half_h + t);                  // top-left
    if (segs & SEG_B) oled_fill_rect(x + w - t, y, t, half_h + t);          // top-right
    if (segs & SEG_E) oled_fill_rect(x, y + half_h, t, half_h + t);         // bottom-left
    if (segs & SEG_C) oled_fill_rect(x + w - t, y + half_h, t, half_h + t); // bottom-right
}

bool oled_task_user(void) {
    oled_clear();
    uint8_t layer = get_highest_layer(layer_state);
    if (is_keyboard_master()) {
        // 32w x 128h logical canvas: tall, narrow digit, centered.
        draw_big_digit(layer, 5, 32, 22, 64, 6);
        // AltGr mode indicator, drawn rather than written: a wide bar for Mac
        // (Unicode-fake), a small square for PC (passthrough). Any oled_write*
        // call here would pull in the 1.3KB glcdfont table for the sake of
        // three letters.
        oled_fill_rect(5, 108, mac_altgr_mode ? 22 : 8, 8);
    } else {
        // 128w x 32h logical canvas: wide, short digit, centered.
        draw_big_digit(layer, 52, 2, 24, 28, 5);
    }
    return false;
}
#endif // OLED_ENABLE

// Rotary encoder related code
#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {              // Encoder on master side
        if (IS_LAYER_ON(_RAISE)) { // on Raise layer
            // Cursor control
            if (clockwise) {
                tap_code(KC_MNXT);
            } else {
                tap_code(KC_MPRV);
            }
        } else {
            if (clockwise) {
                tap_code(KC_VOLU);
            } else {
                tap_code(KC_VOLD);
            }
        }
    } else if (index == 1) {       // Encoder on slave side
        if (IS_LAYER_ON(_LOWER)) { // on Lower layer
            //
            if (clockwise) {
                tap_code(KC_RIGHT);
            } else {
                tap_code(KC_LEFT);
            }
        } else {
            if (clockwise) {
                tap_code(KC_DOWN);
            } else {
                tap_code(KC_UP);
            }
        }
    }
    return true;
}
#endif
