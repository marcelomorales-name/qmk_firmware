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

enum layers {
    _QWERTY,
    _LOWER,
    _RAISE,
    _ADJUST,
    _NUMERIC,
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
                       LALT_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT,                                 // home row
                       KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, QK_REPEAT_KEY, QK_ALT_REPEAT_KEY, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, // bottom row
                       KC_LCTL, KC_LALT, LOWER, LGUI_T(KC_SPC), TD(TAP_SPC_ENT), RAISE, KC_RALT, KC_RCTL                                       // thumbs
                       ),

    // Lower layer: numbers, symbols, mouse keys (held via LOWER)
    [_LOWER] = LAYOUT(KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_DEL,                                   // number row
                      KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_BSLS,                                // top row
                      KC_TRNS, KC_6, KC_7, KC_8, KC_9, KC_0, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_NO, KC_QUOT,                                  // home row
                      KC_TRNS, CW_TOGG, TO(4), MS_BTN2, MS_BTN3, MS_BTN1, MS_WHLU, MS_WHLD, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, KC_NO, KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_SPC, KC_ENT, KC_TRNS, KC_TRNS, KC_TRNS                                                      // thumbs
                      ),

    // Raise layer: function keys, navigation (held via RAISE)
    [_RAISE] = LAYOUT(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,                                  // number row
                      KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,                                  // top row
                      KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_INS, KC_NO,                          // home row
                      KC_TRNS, KC_F11, KC_F12, KC_F13, KC_F14, KC_APP, KC_PSCR, KC_PAUS, KC_HOME, KC_END, KC_PGUP, KC_PGDN, KC_DEL, KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS                                                     // thumbs
                      ),

    // Adjust layer: reboot/bootloader, media keys (LOWER+RAISE)
    [_ADJUST] = LAYOUT(QK_BOOT, QK_RBT, KC_NO, KC_NO, TO(4), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_KB_POWER,      // number row
                       KC_NO, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,        // top row
                       KC_NO, KC_NO, KC_NO, KC_NO, TO(4), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,               // home row
                       KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, // bottom row
                       KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO                                        // thumbs
                       ),

    // Numeric layer: numpad, entered/exited via TO(4)/TO(0)
    [_NUMERIC] = LAYOUT(TO(0), KC_1, KC_2, KC_3, KC_4, KC_5, KC_NUM, KC_PSLS, KC_PAST, KC_PMNS, KC_NO, KC_BSPC,          // number row
                        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_P7, KC_P8, KC_P9, KC_PPLS, KC_NO, KC_NO,                // top row
                        KC_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_P4, KC_P5, KC_P6, KC_PCMM, KC_NO, KC_NO,                // home row
                        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, TO(0), KC_NO, KC_P1, KC_P2, KC_P3, KC_PEQL, KC_NO, KC_NO, // bottom row
                        KC_LCTL, KC_LALT, KC_NO, KC_SPC, KC_ENT, KC_P0, KC_PDOT, KC_NO                                   // thumbs
                        ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
    if (get_highest_layer(state) == _NUMERIC) {
        return state;
    }
    state = update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
    return state;
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

void render_lily58_logo(void) {
    static const char PROGMEM lily58_logo[] = {// 'logo', 128x32px
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xc0, 0x40, 0x40, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0xe0, 0x70, 0x3c, 0x0e, 0x06, 0x0e, 0x3c, 0x70, 0xe0, 0x80, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0xc0, 0xc0, 0x00, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0xc0, 0x80, 0x80, 0x80, 0x81, 0x83, 0x83, 0x07, 0x07, 0x0c, 0x18, 0x70, 0xe0, 0x80, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x80, 0xb6, 0xb6, 0x80, 0xb0, 0xb0, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xf1, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x30, 0xf0, 0xf0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0xf0, 0x30, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xe1, 0x71, 0x71, 0xf1, 0xf1, 0xe1, 0xc1, 0x81, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x3f, 0xff, 0xf3, 0xe1, 0xc1, 0xc1, 0x81, 0x81, 0xc3, 0xff, 0x7f, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x20, 0x70, 0x78, 0xdc, 0xcc, 0x86, 0x06, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x02, 0x06, 0x84, 0xe1, 0xfb, 0x38, 0x1c, 0x0c, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x03, 0x06, 0x86, 0xcc, 0xdc, 0x78, 0x70, 0x20, 0x00, 0xff, 0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1f, 0x7e, 0xf8, 0xe0, 0xf0, 0x7e, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0xc0, 0xe1, 0xff, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x3e, 0xff, 0xff, 0xc1, 0xc0, 0x80, 0x81, 0x81, 0xc3, 0xc3, 0xff, 0xfe, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x06, 0x06, 0x06, 0x04, 0x04, 0x04, 0x04, 0x06, 0x06, 0x02, 0x03, 0x01, 0x01, 0x00, 0x01, 0x01, 0x03, 0x02, 0x06, 0x06, 0x04, 0x04, 0x04, 0x04, 0x06, 0x06, 0x06, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x60, 0x60, 0x70, 0x38, 0x1f, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00};
    oled_write_raw_P(lily58_logo, sizeof(lily58_logo));
}

#    define KEYLOG_LEN 6
char     keylog_str[KEYLOG_LEN] = {};
uint8_t  keylogs_str_idx        = 0;
uint16_t log_timer              = 0;

const char code_to_name[60] = {' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '};

void add_keylog(uint16_t keycode) {
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) || (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
        keycode = keycode & 0xFF;
    }

    for (uint8_t i = KEYLOG_LEN - 1; i > 0; i--) {
        keylog_str[i] = keylog_str[i - 1];
    }
    if (keycode < 60) {
        keylog_str[0] = code_to_name[keycode];
    }
    keylog_str[KEYLOG_LEN - 1] = 0;

    log_timer = timer_read();
}

void update_log(void) {
    if (timer_elapsed(log_timer) > 750) {
        add_keylog(0);
    }
}

void render_keylogger_status(void) {
    oled_write_P(PSTR("KLogr"), false);
    oled_write(keylog_str, false);
}

void render_default_layer_state(void) {
    oled_write_P(PSTR("Layer"), false);
    oled_write_P(PSTR(" "), false);
    switch (get_highest_layer(layer_state)) {
        case _QWERTY:
            oled_write_P(PSTR("QRTY"), false);
            break;
        case _LOWER:
            oled_write_ln_P(PSTR("LOW"), false);
            break;
        case _RAISE:
            oled_write_P(PSTR("HIGH"), false);
            break;
        case _ADJUST:
            oled_write_ln_P(PSTR("ADJ"), false);
            break;
        case _NUMERIC:
            oled_write_ln_P(PSTR("NUM"), false);
            break;
        default:
            oled_write_ln_P(PSTR("Undefined"), false);
    }
}

void render_keylock_status(led_t led_state) {
    oled_write_ln_P(PSTR("Lock"), false);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("N"), led_state.num_lock);
    oled_write_P(PSTR("C"), led_state.caps_lock);
    oled_write_ln_P(PSTR("S"), led_state.scroll_lock);
}

void render_mod_status(uint8_t modifiers) {
    oled_write_ln_P(PSTR("Mods"), false);
    oled_write_P(PSTR(" "), false);
    oled_write_P(PSTR("S"), (modifiers & MOD_MASK_SHIFT));
    oled_write_P(PSTR("C"), (modifiers & MOD_MASK_CTRL));
    oled_write_P(PSTR("A"), (modifiers & MOD_MASK_ALT));
    oled_write_P(PSTR("G"), (modifiers & MOD_MASK_GUI));
}

void render_status_main(void) {
    // Show keyboard layout
    render_default_layer_state();
    // Add a empty line
    oled_write_P(PSTR("-----"), false);
    // Show host keyboard led status
    render_keylock_status(host_keyboard_led_state());
    // Add a empty line
    oled_write_P(PSTR("-----"), false);
    // Show modifier status
    render_mod_status(get_mods());
    // Add a empty line
    oled_write_P(PSTR("-----"), false);
    render_keylogger_status();
}

bool oled_task_user(void) {
    update_log();
    if (is_keyboard_master()) {
        render_status_main(); // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    } else {
        render_lily58_logo();
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        add_keylog(keycode);
    }
    return true;
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
