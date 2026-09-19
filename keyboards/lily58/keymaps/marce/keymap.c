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
    _NUMERIC,
    _LOWER,
    _RAISE,
    _ADJUST,
};

// Tap Dance declarations
enum {
    TAP_SPC_ENT,
    TAP_NUMERIC, // tap: one-shot _NUMERIC (next key only); hold: momentary _NUMERIC
    TAP_NUM_F,   // tap: F; hold: numeric_grid_mode = GRID_F (see below)
    TAP_NUM_G,   // tap: G; hold: numeric_grid_mode = GRID_G
};

enum custom_keycodes {
    ALTGR_MC = SAFE_RANGE, // toggles AltGr between PC-passthrough and Mac Unicode-fake modes
    // _NUMERIC's numpad-grid cells, numbered 1-9 like the numpad they
    // stand in for. Handled in process_record_user: plain digit normally,
    // or an F-key while TAP_NUM_F/TAP_NUM_G is held (see numeric_grid_mode).
    NUMGRID_1,
    NUMGRID_2,
    NUMGRID_3,
    NUMGRID_4,
    NUMGRID_5,
    NUMGRID_6,
    NUMGRID_7,
    NUMGRID_8,
    NUMGRID_9,
};

// Tap _NUMERIC: one-shot for the next keypress only. Hold _NUMERIC: momentary,
// same as MO() while the key is down. layer_held tracks which behavior fired
// so dance_reset only calls layer_off() when the hold-path actually ran.
static bool numeric_layer_held = false;

void td_numeric_finished(tap_dance_state_t *state, void *user_data) {
    if (state->pressed) {
        layer_on(_NUMERIC);
        numeric_layer_held = true;
    } else {
        set_oneshot_layer(_NUMERIC, ONESHOT_START);
    }
}

void td_numeric_reset(tap_dance_state_t *state, void *user_data) {
    if (numeric_layer_held) {
        layer_off(_NUMERIC);
        numeric_layer_held = false;
    }
}

// Holding F or G on _NUMERIC retargets the numpad grid (NUMGRID_1..9) to
// F-keys instead of switching layers: GRID_NONE types plain digits,
// GRID_F/GRID_G pick the F1-F9/F11-F19 bank (see process_record_user).
enum { GRID_NONE = 0, GRID_F, GRID_G };
static uint8_t numeric_grid_mode      = GRID_NONE;
static bool    numeric_grid_f_held    = false;
static bool    numeric_grid_g_held    = false;

void td_num_f_finished(tap_dance_state_t *state, void *user_data) {
    if (state->pressed) {
        numeric_grid_mode   = GRID_F;
        numeric_grid_f_held = true;
    } else {
        tap_code(KC_F);
    }
}

void td_num_f_reset(tap_dance_state_t *state, void *user_data) {
    if (numeric_grid_f_held) {
        numeric_grid_mode   = GRID_NONE;
        numeric_grid_f_held = false;
    }
}

void td_num_g_finished(tap_dance_state_t *state, void *user_data) {
    if (state->pressed) {
        numeric_grid_mode   = GRID_G;
        numeric_grid_g_held = true;
    } else {
        tap_code(KC_G);
    }
}

void td_num_g_reset(tap_dance_state_t *state, void *user_data) {
    if (numeric_grid_g_held) {
        numeric_grid_mode   = GRID_NONE;
        numeric_grid_g_held = false;
    }
}

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TAP_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT),
    [TAP_NUMERIC] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_numeric_finished, td_numeric_reset),
    [TAP_NUM_F]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_num_f_finished, td_num_f_reset),
    [TAP_NUM_G]   = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_num_g_finished, td_num_g_reset),
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY
    [_QWERTY] = LAYOUT(KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,                                            // number row
                       KC_TAB, LT(_NUMERIC, KC_Q), KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,                              // top row
                       LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT),                         // home row
                       KC_LSFT, LCTL_T(KC_Z), LALT_T(KC_X), KC_C, KC_V, KC_B, TD(TAP_NUMERIC), KC_NO, KC_N, KC_M, KC_COMM, RALT_T(KC_DOT), RCTL_T(KC_SLSH), KC_RSFT, // bottom row
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
    // here but moved to the numeric layer's F/G overlays (_NUMERIC_F/_NUMERIC_G).
    [_RAISE] = LAYOUT(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS,                               // number row
                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_PSCR, KC_PAUS, KC_APP, KC_NO, KC_INS, KC_TRNS,                          // top row
                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_DEL, KC_TRNS,                        // home row
                      KC_TRNS, LCTL_T(KC_NO), LALT_T(KC_NO), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_HOME, KC_PGDN, KC_PGUP, RALT_T(KC_END), RCTL_T(KC_NO), KC_TRNS, // bottom row
                      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                                                    // thumbs
                      ),

    // Adjust layer: reboot/bootloader, media keys (LOWER+RAISE).
    [_ADJUST] = LAYOUT(KC_TRNS, KC_NO, ALTGR_MC, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, // number row
                       KC_TRNS, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, // top row
                       KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS,       // home row
                       KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, // bottom row
                       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                            // thumbs
                       ),

    // Numeric layer: number-row digits/symbols (not numpad keycodes, so they
    // aren't affected by the host's Num Lock state) arranged in a numpad-style
    // grid on the right hand, WASD-as-arrows on the left.
    // Entered/exited by tapping TAP_NUMERIC (one-shot) or holding it (MO),
    // by holding Q on _QWERTY (LT), or via TG(_NUMERIC) on _LOWER for a
    // persistent toggle that this same key also switches back off.
    // LOWER/RAISE stay held-only escapes back to those layers, returning to
    // _NUMERIC on release. Holding F/G here retargets the NUMGRID_* cells
    // to F1-F9/F11-F19 instead of plain digits (see numeric_grid_mode and
    // process_record_user); no extra layer needed since it's the same nine
    // physical positions either way.
    // Holding T applies Ctrl+Shift+Gui as a modifier instead of typing T.
    [_NUMERIC] = LAYOUT(KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_NUM, KC_SLSH, LSFT(KC_8), KC_MINS, KC_NO, KC_TRNS,      // number row
                        KC_TRNS, LT(_NUMERIC, KC_Q), KC_UP, KC_E, KC_R, MT(MOD_LCTL | MOD_LSFT | MOD_LGUI, KC_T), NUMGRID_7, NUMGRID_8, NUMGRID_9, LSFT(KC_EQL), KC_NO, KC_TRNS, // top row
                        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, TD(TAP_NUM_F), TD(TAP_NUM_G), NUMGRID_4, NUMGRID_5, NUMGRID_6, KC_COMM, KC_NO, KC_TRNS, // home row
                        KC_TRNS, LCTL_T(KC_Z), LALT_T(KC_X), KC_C, KC_V, KC_B, KC_TRNS, KC_NO, NUMGRID_1, NUMGRID_2, NUMGRID_3, RALT_T(KC_EQL), RCTL_T(KC_NO), KC_TRNS, // bottom row
                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                            // thumbs
                        ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
}

// --- Mac/PC AltGr switch -----------------------------------------------
//
// PC mode: KC_RALT behaves exactly as a normal AltGr modifier (today's
// behavior, passed straight to the host). Mac mode: the physical AltGr
// press is swallowed and the keyboard types Unicode characters via QMK's
// UC_MAC Unicode-Hex Input, since macOS has no equivalent host-side AltGr
// layer. Trimmed to just the Spanish accented letters and inverted
// punctuation: á é í ó ú ü ñ ¿ ¡ (and their uppercase forms).

enum dead_key {
    DEAD_NONE = 0,
    DEAD_ACUTE,
    DEAD_TILDE,
    DEAD_DIAERESIS,
};

#define ALTGR_DEAD_BIT 0x80000000UL
#define ALTGR_DEAD(id) (ALTGR_DEAD_BIT | (uint32_t)(id))
#define ALTGR_IS_DEAD(v) (((v) &ALTGR_DEAD_BIT) != 0)
#define ALTGR_DEAD_ID(v) ((uint8_t)((v) & 0xFF))

typedef struct {
    uint16_t keycode;
    uint32_t plain;   // AltGr level (xkb level 3)
    uint32_t shifted; // AltGr+Shift level (xkb level 4)
} altgr_entry_t;

// Keys with no AltGr mapping are simply absent here, so a lookup miss falls
// back to typing the plain key.
static const altgr_entry_t altgr_map[] PROGMEM = {
    {KC_GRV, 0x0060, ALTGR_DEAD(DEAD_TILDE)},                       // ` (plain) / dead tilde -> ñ Ñ
    {KC_QUOT, ALTGR_DEAD(DEAD_ACUTE), ALTGR_DEAD(DEAD_DIAERESIS)},  // dead acute -> áéíóú / dead diaeresis -> ü
    {KC_1, 0x00A1, 0x00A1},                                          // ¡
    {KC_SLSH, 0x00BF, 0x00BF},                                       // ¿
};

// Spacing glyph typed when a dead key isn't followed by a known combiner
// (0 = no clean standalone glyph, so nothing extra is typed).
static const uint16_t dead_spacing[DEAD_DIAERESIS + 1] PROGMEM = {
    [DEAD_ACUTE]     = 0x00B4, // ´
    [DEAD_TILDE]     = 0x007E, // ~
    [DEAD_DIAERESIS] = 0x00A8, // ¨
};

typedef struct {
    uint8_t  dead;
    uint16_t keycode;
    uint32_t codepoint; // lowercase/base form; see altgr_dead_case_adjust
} dead_combo_t;

static const dead_combo_t dead_combos[] PROGMEM = {
    {DEAD_ACUTE, KC_A, 0x00E1}, {DEAD_ACUTE, KC_E, 0x00E9}, {DEAD_ACUTE, KC_I, 0x00ED}, {DEAD_ACUTE, KC_O, 0x00F3}, {DEAD_ACUTE, KC_U, 0x00FA},

    {DEAD_TILDE, KC_N, 0x00F1},

    {DEAD_DIAERESIS, KC_U, 0x00FC},
};

// All dead_combos codepoints above are Latin-1 Supplement accented letters
// in the 0xE0-0xFE range, where uppercase = lowercase - 0x20.
static uint32_t altgr_dead_case_adjust(uint32_t cp, bool shift) {
    if (!shift) return cp;
    if (cp >= 0x00E0 && cp <= 0x00FE) return cp - 0x20;
    return cp;
}

static bool altgr_lookup(uint16_t keycode, bool shift, uint32_t *out) {
    for (uint8_t i = 0; i < sizeof(altgr_map) / sizeof(altgr_map[0]); i++) {
        altgr_entry_t e;
        memcpy_P(&e, &altgr_map[i], sizeof(e));
        if (e.keycode == keycode) {
            *out = shift ? e.shifted : e.plain;
            return true;
        }
    }
    return false;
}

static bool dead_combo_lookup(uint8_t dead, uint16_t keycode, uint32_t *out) {
    for (uint8_t i = 0; i < sizeof(dead_combos) / sizeof(dead_combos[0]); i++) {
        dead_combo_t c;
        memcpy_P(&c, &dead_combos[i], sizeof(c));
        if (c.dead == dead && c.keycode == keycode) {
            *out = c.codepoint;
            return true;
        }
    }
    return false;
}

static bool     mac_altgr_mode     = false; // false = PC (passthrough), true = Mac (Unicode-fake)
static bool     ralt_held          = false;
static uint8_t  pending_dead       = DEAD_NONE;
static uint16_t altgr_swallowed_key = KC_NO;

// Sent per grid cell so release unregisters exactly what press registered,
// even if numeric_grid_mode changes mid-hold.
static uint16_t numgrid_sent[9] = {0};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= NUMGRID_1 && keycode <= NUMGRID_9) {
        uint8_t idx = keycode - NUMGRID_1;
        if (record->event.pressed) {
            uint8_t  digit = idx + 1;
            uint16_t code;
            if (numeric_grid_mode == GRID_NONE) {
                code = KC_1 + idx; // KC_1..KC_9 are contiguous
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

    // macOS uses "natural" (inverted) scrolling direction vs. PC.
    if (mac_altgr_mode && (keycode == MS_WHLU || keycode == MS_WHLD)) {
        uint16_t inverted = (keycode == MS_WHLU) ? MS_WHLD : MS_WHLU;
        if (record->event.pressed) {
            register_code(inverted);
        } else {
            unregister_code(inverted);
        }
        return false;
    }

    if (keycode == ALTGR_MC) {
        if (record->event.pressed) {
            mac_altgr_mode      = !mac_altgr_mode;
            pending_dead        = DEAD_NONE;
            ralt_held           = false;
            altgr_swallowed_key = KC_NO;
        }
        return false;
    }

    if (keycode == KC_RALT) {
        if (!mac_altgr_mode) {
            return true;
        }
        ralt_held = record->event.pressed;
        return false;
    }

    if (!record->event.pressed && keycode == altgr_swallowed_key) {
        altgr_swallowed_key = KC_NO;
        return false;
    }

    if (pending_dead != DEAD_NONE && record->event.pressed) {
        uint8_t  dead = pending_dead;
        pending_dead  = DEAD_NONE;
        uint32_t composed;
        if (dead_combo_lookup(dead, keycode, &composed)) {
            register_unicode(altgr_dead_case_adjust(composed, (get_mods() & MOD_MASK_SHIFT) != 0));
            altgr_swallowed_key = keycode;
            return false;
        }
        uint16_t spacing = pgm_read_word(&dead_spacing[dead]);
        if (spacing) {
            register_unicode(spacing);
        }
        return true; // let this keystroke also type normally
    }

    if (mac_altgr_mode && ralt_held && record->event.pressed) {
        uint32_t out;
        if (altgr_lookup(keycode, (get_mods() & MOD_MASK_SHIFT) != 0, &out)) {
            if (ALTGR_IS_DEAD(out)) {
                pending_dead = ALTGR_DEAD_ID(out);
            } else {
                register_unicode(out);
            }
            altgr_swallowed_key = keycode;
            return false;
        }
        return true;
    }

    return true;
}

void keyboard_post_init_user(void) {
    set_unicode_input_mode(UNICODE_MODE_MACOS);
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
        // AltGr mode indicator: PC (passthrough) vs Mac (Unicode-fake).
        oled_set_cursor(0, 14);
        oled_write_P(mac_altgr_mode ? PSTR("MAC") : PSTR("PC"), false);
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
