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

extern uint8_t is_master;

// _NUMERIC must stay below _LOWER/_RAISE/_ADJUST: QMK resolves keys from the
// highest active layer down, so LOWER/RAISE held while _NUMERIC is on only
// overlay it (instead of being shadowed by it) if they outrank it here.
enum layers {
    _QWERTY,
    _NUMERIC,
    _LOWER,
    _RAISE,
    _ADJUST,
};

// Tap Dance declarations
enum {
    TAP_SPC_ENT,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TAP_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT),
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY
    [_QWERTY] = LAYOUT(KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,                                            // number row
                       KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,                                            // top row
                       LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT),                         // home row
                       KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, QK_REPEAT_KEY, QK_ALT_REPEAT_KEY, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, // bottom row
                       KC_LCTL, KC_LALT, LOWER, LGUI_T(KC_SPC), TD(TAP_SPC_ENT), RAISE, KC_RALT, KC_RCTL                                       // thumbs
                       ),

    // Lower layer: numbers, symbols, mouse keys (held via LOWER)
    [_LOWER] = LAYOUT(KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL,                                   // number row
                      KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_BSLS,                                // top row
                      KC_TRNS, KC_6, KC_7, KC_8, KC_9, KC_0, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_NO, KC_QUOT,                                  // home row
                      KC_TRNS, CW_TOGG, TO(_NUMERIC), MS_BTN2, MS_BTN3, MS_BTN1, MS_WHLU, MS_WHLD, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, KC_NO, KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_SPC, KC_ENT, KC_TRNS, KC_TRNS, KC_TRNS                                                      // thumbs
                      ),

    // Raise layer: function keys, navigation (held via RAISE)
    [_RAISE] = LAYOUT(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,                                 // number row
                      KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_PSCR, KC_PAUS, KC_APP, KC_NO, KC_NO, KC_NO,                             // top row
                      KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_INS, KC_NO,                          // home row
                      KC_TRNS, KC_F11, KC_F12, KC_F13, KC_F14, KC_F15, KC_NO, KC_NO, KC_HOME, KC_END, KC_PGUP, KC_PGDN, KC_DEL, KC_TRNS,     // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS                                                     // thumbs
                      ),

    // Adjust layer: reboot/bootloader, media keys (LOWER+RAISE)
    [_ADJUST] = LAYOUT(QK_BOOT, QK_RBT, KC_NO, KC_NO, TO(_NUMERIC), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_KB_POWER, // number row
                       KC_NO, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,        // top row
                       KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,               // home row
                       KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, // bottom row
                       KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO                                        // thumbs
                       ),

    // Numeric layer: numpad on the right hand, WASD-as-arrows on the left,
    // entered/exited via TO(_NUMERIC)/TO(0). LOWER/RAISE stay held-only
    // escapes back to those layers, returning to _NUMERIC on release.
    [_NUMERIC] = LAYOUT(TO(0), KC_1, KC_2, KC_3, KC_4, KC_5, KC_NUM, KC_PSLS, KC_PAST, KC_PMNS, KC_NO, KC_BSPC,           // number row
                        KC_TAB, KC_Q, KC_UP, KC_E, KC_R, KC_T, KC_P7, KC_P8, KC_P9, KC_PPLS, KC_NO, KC_NO,                // top row
                        KC_ESC, KC_LEFT, KC_DOWN, KC_RIGHT, KC_F, KC_G, KC_P4, KC_P5, KC_P6, KC_PCMM, KC_NO, KC_NO,       // home row
                        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, TO(0), KC_NO, KC_P1, KC_P2, KC_P3, KC_PEQL, KC_NO, KC_NO,  // bottom row
                        KC_LCTL, KC_LALT, LOWER, KC_SPC, KC_ENT, RAISE, KC_P0, KC_PDOT                                    // thumbs
                        ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
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
        // Caps Word indicator: only the master knows the real state (it
        // isn't synced across split like layer_state is), so it only
        // renders here.
        if (is_caps_word_on()) {
            oled_fill_rect(4, 4, 10, 10);
        }
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
