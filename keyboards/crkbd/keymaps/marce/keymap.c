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

// Ported from keyboards/lily58/keymaps/marce. This board uses
// LAYOUT_split_3x6_3_ex2 (46 keys) vs Lily58's 58 -- no number row, only 3
// thumb keys per side instead of 4, but with 2 extra keys per side (the
// "ex2" columns) stacked in the top/home rows directly above each side's
// innermost thumb key. Deliberate changes made during the port:
//   - The number row is gone; 1-0, [, ], -, and = all live on _LOWER
//     instead (top row: numbers/-/=, home row: [/]).
//   - The top-row ex2 column (left side) carries _NUMERIC-layer access via
//     the shared NUM_LAYER keycode (same tap-one-shot/hold-momentary key as
//     Lily58; crkbd used to have its own NUM_MO/NUM_OS pair here plus a
//     lock-on chord -- dropped in favor of the shared key, see PLAN.md
//     phase 4b). The right-side ex2 column it used to share with NUM_OS is
//     now free (KC_NO). The home-row ex2 columns still carry repeat/alt
//     repeat directly on the base layer.
//   - Backslash is gone from the base layer (freed up for Backspace on the
//     top-row outer key); still reachable via _LOWER's home row.
//   - Ctrl is no longer a dedicated thumb key (only 3 thumb keys/side, and
//     Lower/Raise/Space/Enter/Alt/AltGr already claim all six). It's now a
//     mod-tap on Z/Slash (hold = Ctrl, tap = the letter) so Ctrl+C/V/X etc.
//     stay reachable without a layer shift.
//   - The digitizer 3x3 grid on _ADJUST maps 1:1 onto Corne's 3 physical
//     rows now (top/mid/bottom), instead of being squeezed into 2 rows.
//   - _NUMERIC's numpad is the shared NUMGRID_1..9,0 grid (number-row digits,
//     not real numpad keycodes, so host Num Lock state doesn't matter) with
//     NUM_GRD_F/NUM_GRD_G retargeting it to F-keys -- same as Lily58, and in
//     the same physical spots: the grid itself maps 1:1 onto where KC_P1..9
//     used to sit, and F/G land on the home row's operator column, which
//     bumped the numpad's "*" key up to the top row (see _NUMERIC below).
// Layer structure, tap dance, digitizer logic, the numeric grid overlay and
// the Mac/PC AltGr-Unicode fake layer are shared with the Lily58 keymap via
// users/marce/.

#include QMK_KEYBOARD_H
#include "marce.h"

extern uint8_t is_master;

// Board-only keycodes; the shared ones (including NUM_LAYER as of phase 4b)
// live in users/marce/marce.h.
enum custom_keycodes {
    CW_CTL = MARCE_SAFE_RANGE, // tap: toggle Caps Word. hold: Ctrl. CW_TOGG isn't a basic
                               // keycode so it can't use the built-in xxxx_T() mod-tap macros;
                               // handled by hand in process_record_user/matrix_scan_user below.
};

// Tap Dance definitions (shared entries live in users/marce/marce.h)
tap_dance_action_t tap_dance_actions[] = {MARCE_TAP_DANCE_ACTIONS};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY. The extra ex2 column (stacked above each side's
    // innermost thumb key) carries _NUMERIC access (top row, left side only
    // now -- see NUM_LAYER in users/marce/marce.h; the right side's top-row
    // ex2 slot is free) and repeat/alt repeat (home row, closest to the
    // thumbs), no LOWER needed.
    [_QWERTY] = LAYOUT_split_3x6_3_ex2(KC_TAB, LT(_NUMERIC, KC_Q), KC_W, KC_E, KC_R, KC_T, NUM_LAYER, KC_NO, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,                                    // top row
                                       LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, QK_REPEAT_KEY, QK_ALT_REPEAT_KEY, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT), // home row
                                       LSFT_T(KC_GRV), LCTL_T(KC_Z), KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, RCTL_T(KC_SLSH), KC_RSFT,                      // bottom row
                                       KC_LALT, LOWER, LGUI_T(KC_SPC), TD(TAP_SPC_ENT), RAISE, KC_RALT                                                                   // thumbs
                                       ),

    // Lower layer: symbols, mouse keys (held via LOWER). The number row
    // (left hand, top/home) is gone -- reach numbers via _NUMERIC instead.
    // Right-hand top/home and the bottom row are exactly the Lily58 marce
    // layout's Lower rows -- for the bottom row, which is 7 keys/side on
    // Lily58, that means dropping its innermost (extra) column on each
    // side.
    [_LOWER] = LAYOUT_split_3x6_3_ex2(KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_TRNS, // top row
                                      KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_TRNS, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_BSLS, KC_TRNS, // home row
                                      KC_TRNS, CW_CTL, KC_NO, MS_BTN2, MS_BTN3, MS_BTN1, MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, KC_NO, KC_TRNS,            // bottom row
                                      KC_TRNS, KC_TRNS, KC_SPC, KC_ENT, KC_TRNS, KC_TRNS                                                                // thumbs
                                      ),

    // Raise layer: function keys, navigation (held via RAISE). Top/home/
    // bottom rows are exactly the Lily58 marce layout's Raise top/home/
    // bottom rows (Lily58's bottom-row innermost columns were both KC_NO,
    // so dropping them to fit 6/side changes nothing).
    [_RAISE] = LAYOUT_split_3x6_3_ex2(KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_TRNS, KC_TRNS, KC_PSCR, KC_PAUS, KC_APP, KC_NO, KC_NO, KC_TRNS,       // top row
                                      KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_INS, KC_TRNS,    // home row
                                      KC_TRNS, LCTL_T(KC_F11), KC_F12, KC_F13, KC_F14, KC_F15, KC_HOME, KC_END, KC_PGUP, KC_PGDN, LCTL_T(KC_DEL), KC_TRNS, // bottom row
                                      KC_TRNS, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_TRNS                                                                     // thumbs
                                      ),

    // Adjust layer: media keys (LOWER+RAISE). Right half hosts the
    // digitizer point grid, one physical row per grid row. No
    // reboot/bootloader keys here anymore; keyboard-power moved off the
    // outer column (onto the freed NO slot next to it) so that column can
    // stay transparent like every other layer.
    [_ADJUST] = LAYOUT_split_3x6_3_ex2(KC_TRNS, KC_NO, ALTGR_MC, KC_NO, TO(_NUMERIC), KC_KB_POWER, KC_TRNS, KC_TRNS, DIG_TL, DIG_TC, DIG_TR, KC_NO, KC_NO, KC_TRNS, // top row
                                       KC_TRNS, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, KC_TRNS, KC_TRNS, DIG_ML, DIG_MC, DIG_MR, KC_NO, KC_NO, KC_TRNS,          // home row
                                       KC_TRNS, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, DIG_BL, DIG_BC, DIG_BR, KC_NO, KC_NO, KC_TRNS,                                   // bottom row
                                       KC_NO, KC_TRNS, KC_NO, KC_NO, KC_TRNS, KC_NO                                                                                 // thumbs
                                       ),

    // Q on the base layer is LT(_NUMERIC, KC_Q): hold for _NUMERIC (also kept
    // on Q there so the layer stays held), tap for Q. T on _NUMERIC is
    // Ctrl+Shift+Gui when held, T when tapped (same as Lily58); the "/" that
    // used to sit there is gone.
    // Numeric layer: the shared NUMGRID_1..9,0 grid on the right hand (number-
    // row digits, not real numpad keycodes, so host Num Lock doesn't matter),
    // arrows on the left, entered/exited via NUM_LAYER/TO(_NUMERIC)/TO(0)
    // (TO(0) sits on the home row's left ex2 key, one step in from TAB's
    // top-left corner, so the outer columns can stay identical to every
    // other layer). NUM_GRD_F/NUM_GRD_G (hold -> retarget the grid to F-keys,
    // tap -> F/G) sit on the home row's operator column, exactly where
    // Lily58 puts them (physically F/G on the base layer): KC_PAST(*), which
    // used to live there, moved up to the top row next to KC_PSLS(/) so the
    // two still read together. KC_PPLS/KC_PMNS/KC_PEQL stay put (not part of
    // the grid -- only the nine numbered cells get the F/G overlay).
    // KC_BSPC swapped places with KC_PEQL/KC_RCTL -- it now sits on the
    // left hand's innermost column (right where the 9/6/3 column used to
    // be), so 7/8/9, 4/5/6, and 1/2/3 can each read left-to-right on the
    // right hand, and RSFT landed on the right hand's outermost column.
    // LOWER/RAISE sit in the same thumb slots as every other layer; Space/
    // Enter moved to the remaining inner thumb keys, and Ctrl/Alt fill the
    // otherwise-unused left bottom row (its outer column is KC_TRNS, so
    // Shift -- via the base layer's LSFT_T(KC_GRV) -- still lands there),
    // with RCTL flanking KC_PDOT on the right. LOWER/RAISE stay held-only
    // escapes back to those layers, returning to _NUMERIC on release. The
    // left outer thumb key falls through to base layer's Alt; the right one
    // is NUMGRID_0 instead (below KC_PDOT, which sits in the same spot as
    // KC_DOT on the base layer) -- the same thumb key KC_P0 used to occupy,
    // so no placement change there, just a different keycode. Every row's
    // first and last column is now either literally identical to the base
    // layer or KC_TRNS falling through to it, matching every other layer.
    [_NUMERIC] = LAYOUT_split_3x6_3_ex2(KC_TAB, LT(_NUMERIC, KC_Q), KC_UP, KC_NO, KC_PAST, MT(MOD_LCTL | MOD_LSFT | MOD_LGUI, KC_T), KC_NO, KC_NO, NUMGRID_7, NUMGRID_8, NUMGRID_9, KC_PPLS, KC_NO, KC_TRNS,             // top row
                                        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, NUM_GRD_F, NUM_GRD_G, TO(0), KC_NO, NUMGRID_4, NUMGRID_5, NUMGRID_6, KC_PMNS, KC_NO, KC_TRNS, // home row
                                        KC_TRNS, KC_LCTL, KC_LALT, KC_NO, KC_NO, KC_PEQL, NUMGRID_1, NUMGRID_2, NUMGRID_3, KC_PDOT, KC_RCTL, KC_RSFT,                     // bottom row
                                        KC_TRNS, LOWER, KC_SPC, KC_ENT, RAISE, NUMGRID_0                                                                                  // thumbs
                                        ),
};

#ifdef RGB_MATRIX_ENABLE
static void update_rgb_matrix(uint8_t layer);

layer_state_t layer_state_set_marce_kb(layer_state_t state) {
    update_rgb_matrix(get_highest_layer(state));
    return state;
}
#endif

#ifdef RGB_MATRIX_ENABLE
// Cool blue while AltGr fakes macOS Unicode input, warm red while it's
// passed straight through to a PC. Brightness climbs with how deep in the
// layer stack we are, brightest on _ADJUST as a "you're in the danger
// zone" cue (boot/reset live there). Layer order must match enum layers.
static const uint8_t layer_val[5] PROGMEM = {10, 22, 34, 46, 50};

static void update_rgb_matrix(uint8_t layer) {
    uint8_t hue = mac_altgr_mode ? 170 : 0; // 170 = cool blue, 0 = red
    uint8_t val = pgm_read_byte(&layer_val[layer < 5 ? layer : 0]);
    rgb_matrix_sethsv_noeeprom(hue, 255, val);
}

// Recolor as soon as ALTGR_MC flips the Mac/PC mode (see users/marce/marce.c).
void mac_altgr_mode_changed_marce_kb(void) {
    update_rgb_matrix(get_highest_layer(layer_state));
}
#endif

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

void keyboard_post_init_user(void) {
#ifdef RGB_MATRIX_ENABLE
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    update_rgb_matrix(get_highest_layer(layer_state));
#endif
}

// SSD1306 OLED update loop. Only relevant if this board actually has OLED
// modules installed and OLED_ENABLE=yes in rules.mk (off by default -- flip
// it on if you add screens).
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

// Rotary encoder handling. Corne v4 has real hardware encoders (unlike
// Lily58L where this was dead code), so this actually runs here.
#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {              // Encoder on master side
        if (IS_LAYER_ON(_RAISE)) { // on Raise layer
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
