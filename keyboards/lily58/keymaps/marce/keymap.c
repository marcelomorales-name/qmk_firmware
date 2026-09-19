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

enum layers {
    _QWERTY,
    _NUMERIC,
    _LOWER,
    _RAISE,
    _ADJUST,
};

// Tap Dance declarations. Only the space/enter double-tap still needs the tap
// dance framework -- it's the one behavior here that genuinely keys off a
// second tap. The hold-vs-tap keys below are cheaper as custom keycodes.
enum {
    TAP_SPC_ENT,
};

enum custom_keycodes {
    ALTGR_MC = SAFE_RANGE, // toggles AltGr between PC-passthrough and Mac Unicode-fake modes
    // _NUMERIC's numpad-grid cells, numbered 1-9 like the numpad they
    // stand in for. Handled in process_record_user: plain digit normally,
    // or an F-key while NUM_GRD_F/NUM_GRD_G is held (see numeric_grid_mode).
    // Must stay contiguous and in order: process_record_user range-checks them
    // and derives the digit from the offset.
    NUMGRID_1,
    NUMGRID_2,
    NUMGRID_3,
    NUMGRID_4,
    NUMGRID_5,
    NUMGRID_6,
    NUMGRID_7,
    NUMGRID_8,
    NUMGRID_9,
    // Hold-vs-tap keys, resolved in process_record_user (see hold_key below).
    NUM_LAYER,  // tap: one-shot _NUMERIC (next key only); hold: momentary _NUMERIC
    NUM_GRD_F,  // tap: F; hold: numeric_grid_mode = GRID_F
    NUM_GRD_G,  // tap: G; hold: numeric_grid_mode = GRID_G
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
};

// Hold-vs-tap without tap dance. Pressing arms the hold behavior immediately;
// the release decides whether it was really a tap -- short, and with no other
// key pressed in between -- and only then emits the tap action. That needs no
// callback pair and no tap_dance_actions[] entry per key, and it drops the
// hold's start latency as a bonus. hold_key names the key currently down, so a
// release that arrives after a different hold key took over emits nothing.
static uint16_t hold_key         = KC_NO;
static uint16_t hold_timer       = 0;
static bool     hold_interrupted = false;

static void hold_key_press(uint16_t keycode) {
    hold_key         = keycode;
    hold_timer       = timer_read();
    hold_interrupted = false;
}

// True when this release should be treated as a tap rather than a hold.
static bool hold_key_was_tap(uint16_t keycode) {
    bool tapped = (hold_key == keycode) && !hold_interrupted && timer_elapsed(hold_timer) < TAPPING_TERM;
    if (hold_key == keycode) {
        hold_key = KC_NO;
    }
    return tapped;
}

// Holding F or G on _NUMERIC retargets the numpad grid (NUMGRID_1..9) to
// F-keys instead of switching layers: GRID_NONE types plain digits,
// GRID_F/GRID_G pick the F1-F9/F11-F19 bank (see process_record_user).
enum { GRID_NONE = 0, GRID_F, GRID_G };
static uint8_t numeric_grid_mode = GRID_NONE;

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Space, twice for Enter
    [TAP_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT),
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

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
    [_NUMERIC] = LAYOUT(KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_NUM, KC_SLSH, LSFT(KC_8), KC_MINS, KC_NO, KC_TRNS,      // number row
                        KC_TRNS, LT(_NUMERIC, KC_Q), KC_UP, KC_E, KC_R, MT(MOD_LCTL | MOD_LSFT | MOD_LGUI, KC_T), NUMGRID_7, NUMGRID_8, NUMGRID_9, LSFT(KC_EQL), KC_NO, KC_TRNS, // top row
                        KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT, NUM_GRD_F, NUM_GRD_G, NUMGRID_4, NUMGRID_5, NUMGRID_6, KC_COMM, KC_NO, KC_TRNS, // home row
                        KC_TRNS, LCTL_T(KC_Z), LALT_T(KC_X), KC_C, KC_V, KC_B, KC_TRNS, KC_NO, NUMGRID_1, NUMGRID_2, NUMGRID_3, RALT_T(KC_EQL), RCTL_T(KC_NO), KC_TRNS, // bottom row
                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS                            // thumbs
                        ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
    return update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
}

// Absolute screen positions for the DIG_* keys, indexed by keycode - DIG_TL.
// Order must match the custom_keycodes enum above. Edges are pulled in to
// 5%/95% rather than 0%/100% so the pointer lands inside the screen.
typedef struct {
    float x;
    float y;
} digitizer_point_t;

static const digitizer_point_t digitizer_points[9] PROGMEM = {
    {0.05f, 0.05f}, {0.5f, 0.05f}, {0.95f, 0.05f}, // DIG_TL, DIG_TC, DIG_TR
    {0.05f, 0.5f}, {0.5f, 0.5f}, {0.95f, 0.5f},    // DIG_ML, DIG_MC, DIG_MR
    {0.05f, 0.95f}, {0.5f, 0.95f}, {0.95f, 0.95f}, // DIG_BL, DIG_BC, DIG_BR
};

// --- Mac/PC AltGr switch -----------------------------------------------
//
// PC mode: KC_RALT behaves exactly as a normal AltGr modifier (today's
// behavior, passed straight to the host). Mac mode: the physical AltGr
// press is swallowed and the keyboard types the Unicode character itself,
// since macOS has no equivalent host-side AltGr layer.
//
// Two ways to reach an accented letter, and both work:
//
//   * Direct, one chord -- AltGr+a is á, AltGr+Shift+a is Á. Covers the
//     everyday Spanish set plus a few extras; see altgr_map.
//   * Dead keys at the usual US-international AltGr positions, for
//     everything the direct map doesn't cover: AltGr+` grave, AltGr+~
//     tilde, AltGr+' acute, AltGr+" diaeresis, AltGr+6 circumflex,
//     each followed by the letter to accent (see dead_combos).
//
// The dead-key path is checked first, so a pending dead key always wins
// over the direct mapping of the key that follows it.

enum dead_key {
    DEAD_NONE = 0,
    DEAD_GRAVE,
    DEAD_ACUTE,
    DEAD_CIRCUMFLEX,
    DEAD_TILDE,
    DEAD_DIAERESIS, // keep last: dead_spacing[] is sized from it
};

// Codepoints are 16-bit on purpose: everything below is Latin-1, so the whole
// table fits in uint16_t and the AVR generates 2-byte compares and masks
// instead of the 4-byte sequences a uint32_t would force. That also leaves
// bit 15 free as the dead-key marker.
#define ALTGR_DEAD_BIT 0x8000U
#define ALTGR_DEAD(id) (ALTGR_DEAD_BIT | (uint16_t)(id))
#define ALTGR_IS_DEAD(v) (((v) & ALTGR_DEAD_BIT) != 0)
#define ALTGR_DEAD_ID(v) ((uint8_t)((v) & 0xFF))

typedef struct {
    uint16_t keycode;
    uint16_t plain;   // AltGr level (xkb level 3)
    uint16_t shifted; // AltGr+Shift level (xkb level 4)
} altgr_entry_t;

// Keys with no AltGr mapping are simply absent here, so a lookup miss falls
// back to typing the plain key. Uppercase forms are spelled out rather than
// derived, because the shifted level isn't always the same letter (`,` gives
// ç / Ç, but `1` gives ¡ either way).
static const altgr_entry_t altgr_map[] PROGMEM = {
    // Dead keys, at the usual US-international AltGr positions.
    {KC_GRV, ALTGR_DEAD(DEAD_GRAVE), ALTGR_DEAD(DEAD_TILDE)},      // ` grave / ~ tilde
    {KC_QUOT, ALTGR_DEAD(DEAD_ACUTE), ALTGR_DEAD(DEAD_DIAERESIS)}, // ' acute / " diaeresis
    {KC_6, ALTGR_DEAD(DEAD_CIRCUMFLEX), ALTGR_DEAD(DEAD_CIRCUMFLEX)}, // ^ circumflex, either level

    // Direct one-chord accented letters.
    {KC_A, 0x00E1, 0x00C1}, // á Á
    {KC_E, 0x00E9, 0x00C9}, // é É
    {KC_I, 0x00ED, 0x00CD}, // í Í
    {KC_O, 0x00F3, 0x00D3}, // ó Ó
    {KC_U, 0x00FA, 0x00DA}, // ú Ú
    {KC_Y, 0x00FC, 0x00DC}, // ü Ü -- y, since u is already taken by ú
    {KC_N, 0x00F1, 0x00D1}, // ñ Ñ

    // Punctuation and symbols.
    {KC_1, 0x00A1, 0x00A1},    // ¡ either level
    {KC_SLSH, 0x00BF, 0x00BF}, // ¿ either level
    {KC_5, 0x20AC, 0x20AC},    // € either level
    {KC_COMM, 0x00E7, 0x00C7}, // ç Ç
};

// Spacing glyph typed when a dead key isn't followed by a known combiner
// (0 = no clean standalone glyph, so nothing extra is typed).
static const uint16_t dead_spacing[DEAD_DIAERESIS + 1] PROGMEM = {
    [DEAD_GRAVE]      = 0x0060, // `
    [DEAD_ACUTE]      = 0x00B4, // ´
    [DEAD_CIRCUMFLEX] = 0x005E, // ^
    [DEAD_TILDE]      = 0x007E, // ~
    [DEAD_DIAERESIS]  = 0x00A8, // ¨
};

typedef struct {
    uint8_t  dead;
    uint16_t keycode;
    uint16_t codepoint; // lowercase/base form; see altgr_dead_case_adjust
} dead_combo_t;

// Lowercase forms only; the uppercase ones are derived by
// altgr_dead_case_adjust, which works because every codepoint here sits in the
// 0xE0-0xFE Latin-1 block.
static const dead_combo_t dead_combos[] PROGMEM = {
    {DEAD_GRAVE, KC_A, 0x00E0}, {DEAD_GRAVE, KC_E, 0x00E8}, {DEAD_GRAVE, KC_I, 0x00EC}, {DEAD_GRAVE, KC_O, 0x00F2}, {DEAD_GRAVE, KC_U, 0x00F9}, // à è ì ò ù

    {DEAD_ACUTE, KC_A, 0x00E1}, {DEAD_ACUTE, KC_E, 0x00E9}, {DEAD_ACUTE, KC_I, 0x00ED}, {DEAD_ACUTE, KC_O, 0x00F3}, {DEAD_ACUTE, KC_U, 0x00FA}, // á é í ó ú

    {DEAD_CIRCUMFLEX, KC_A, 0x00E2}, {DEAD_CIRCUMFLEX, KC_E, 0x00EA}, {DEAD_CIRCUMFLEX, KC_I, 0x00EE}, {DEAD_CIRCUMFLEX, KC_O, 0x00F4}, {DEAD_CIRCUMFLEX, KC_U, 0x00FB}, // â ê î ô û

    {DEAD_TILDE, KC_A, 0x00E3}, {DEAD_TILDE, KC_N, 0x00F1}, {DEAD_TILDE, KC_O, 0x00F5}, // ã ñ õ

    {DEAD_DIAERESIS, KC_A, 0x00E4}, {DEAD_DIAERESIS, KC_E, 0x00EB}, {DEAD_DIAERESIS, KC_I, 0x00EF}, {DEAD_DIAERESIS, KC_O, 0x00F6}, {DEAD_DIAERESIS, KC_U, 0x00FC}, // ä ë ï ö ü
};

// All dead_combos codepoints above are Latin-1 Supplement accented letters
// in the 0xE0-0xFE range, where uppercase = lowercase - 0x20.
static uint16_t altgr_dead_case_adjust(uint16_t cp, bool shift) {
    if (!shift) return cp;
    if (cp >= 0x00E0 && cp <= 0x00FE) return cp - 0x20;
    return cp;
}

static bool altgr_lookup(uint16_t keycode, bool shift, uint16_t *out) {
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

static bool dead_combo_lookup(uint8_t dead, uint16_t keycode, uint16_t *out) {
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

// macOS Unicode Hex Input, hand-rolled. QMK's UNICODE_ENABLE would bring in all
// six input modes, the mode-cycling table and its eeprom persistence; this
// keymap only ever uses the macOS mode, for the handful of Latin-1 codepoints
// in the tables above. The host still has to have the "Unicode Hex Input"
// keyboard layout selected, exactly as with UNICODE_MODE_MACOS.
//
// Protocol: hold Left Option, tap the four hex digits, release.
static void send_mac_unicode(uint16_t cp) {
    uint8_t saved_mods = get_mods();
    clear_mods();
    clear_weak_mods();

    register_code(KC_LEFT_ALT);
    wait_ms(10); // let the host register Option before the digits arrive
    for (int8_t shift = 12; shift >= 0; shift -= 4) {
        uint8_t nibble = (cp >> shift) & 0xF;
        if (nibble == 0) {
            tap_code(KC_0);
        } else if (nibble < 10) {
            tap_code(KC_1 + nibble - 1); // KC_1..KC_9 are contiguous
        } else {
            tap_code(KC_A + nibble - 10);
        }
    }
    unregister_code(KC_LEFT_ALT);

    set_mods(saved_mods);
}

static bool     mac_altgr_mode     = false; // false = PC (passthrough), true = Mac (Unicode-fake)
static bool     ralt_held          = false;
static uint8_t  pending_dead       = DEAD_NONE;
static uint16_t altgr_swallowed_key = KC_NO;

// Sent per grid cell so release unregisters exactly what press registered,
// even if numeric_grid_mode changes mid-hold.
static uint16_t numgrid_sent[9] = {0};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Any other key going down while a hold-vs-tap key is held settles it as a
    // hold, so its release won't emit the tap action.
    if (record->event.pressed && hold_key != KC_NO && keycode != hold_key) {
        hold_interrupted = true;
    }

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

    if (keycode >= DIG_TL && keycode <= DIG_BR) {
        digitizer_point_t point;
        memcpy_P(&point, &digitizer_points[keycode - DIG_TL], sizeof(point));
        if (record->event.pressed) {
            digitizer_in_range_on();
            digitizer_set_position(point.x, point.y);
        } else {
            digitizer_in_range_off();
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

    // Several of the keys the AltGr tables name are mod-taps in the keymap
    // (' is RGUI_T(KC_QUOT), / is RCTL_T(KC_SLSH)), and a mod-tap arrives here
    // as the whole LT/MT keycode, not the letter it taps. Match on the tap
    // keycode so those positions are reachable; the raw `keycode` is still what
    // gets returned to QMK, so an unmatched mod-tap behaves exactly as before.
    uint16_t tap_kc = IS_QK_MOD_TAP(keycode) ? QK_MOD_TAP_GET_TAP_KEYCODE(keycode) : keycode;

    if (!record->event.pressed && tap_kc == altgr_swallowed_key) {
        altgr_swallowed_key = KC_NO;
        return false;
    }

    if (pending_dead != DEAD_NONE && record->event.pressed) {
        uint8_t  dead = pending_dead;
        pending_dead  = DEAD_NONE;
        uint16_t composed;
        if (dead_combo_lookup(dead, tap_kc, &composed)) {
            send_mac_unicode(altgr_dead_case_adjust(composed, (get_mods() & MOD_MASK_SHIFT) != 0));
            altgr_swallowed_key = tap_kc;
            return false;
        }
        uint16_t spacing = pgm_read_word(&dead_spacing[dead]);
        if (spacing) {
            send_mac_unicode(spacing);
        }
        return true; // let this keystroke also type normally
    }

    if (mac_altgr_mode && ralt_held && record->event.pressed) {
        uint16_t out;
        if (altgr_lookup(tap_kc, (get_mods() & MOD_MASK_SHIFT) != 0, &out)) {
            if (ALTGR_IS_DEAD(out)) {
                pending_dead = ALTGR_DEAD_ID(out);
            } else {
                send_mac_unicode(out);
            }
            altgr_swallowed_key = tap_kc;
            return false;
        }
        return true;
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
