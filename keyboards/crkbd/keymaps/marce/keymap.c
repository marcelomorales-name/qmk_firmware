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
//   - The ex2 columns carry volume (top row) and repeat/alt repeat (home
//     row, closest to the thumbs) directly on the base layer.
//   - Backslash is gone from the base layer (freed up for Backspace on the
//     top-row outer key); still reachable via _LOWER's home row.
//   - Ctrl is no longer a dedicated thumb key (only 3 thumb keys/side, and
//     Lower/Raise/Space/Enter/Alt/AltGr already claim all six). It's now a
//     mod-tap on Z/Slash (hold = Ctrl, tap = the letter) so Ctrl+C/V/X etc.
//     stay reachable without a layer shift.
//   - The digitizer 3x3 grid on _ADJUST maps 1:1 onto Corne's 3 physical
//     rows now (top/mid/bottom), instead of being squeezed into 2 rows.
// Everything else (layer structure, tap dance, digitizer logic, the Mac/PC
// AltGr-Unicode fake layer) is unchanged from the Lily58 source.

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

// Digitizer point keys: each moves the digitizer cursor to a fixed absolute
// position on screen without clicking, mirroring the numpad's spatial layout
// (7=top-left, 5=center, 3=bottom-right, ...). Order must match
// digitizer_points[] below.
enum custom_keycodes {
    DIG_TL = SAFE_RANGE,
    DIG_TC,
    DIG_TR,
    DIG_ML,
    DIG_MC,
    DIG_MR,
    DIG_BL,
    DIG_BC,
    DIG_BR,
    ALTGR_MC, // toggles AltGr between PC-passthrough and Mac Unicode-fake modes
    CW_CTL,   // tap: toggle Caps Word. hold: Ctrl. CW_TOGG isn't a basic
              // keycode so it can't use the built-in xxxx_T() mod-tap macros;
              // handled by hand in process_record_user/matrix_scan_user below.
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Space, twice for Enter
    [TAP_SPC_ENT] = ACTION_TAP_DANCE_DOUBLE(KC_SPC, KC_ENT),
};

#define RAISE MO(_RAISE)
#define LOWER MO(_LOWER)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // Base layer: QWERTY. The extra ex2 column (stacked above each side's
    // innermost thumb key) carries volume on the top row and repeat/alt
    // repeat on the home row (closest to the thumbs), no LOWER needed.
    [_QWERTY] = LAYOUT_split_3x6_3_ex2(
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_VOLU,                            KC_VOLD, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,                  // top row
        LGUI_T(KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G, QK_REPEAT_KEY,              QK_ALT_REPEAT_KEY, KC_H, KC_J, KC_K, KC_L, KC_SCLN, RGUI_T(KC_QUOT), // home row
        LSFT_T(KC_GRV), LCTL_T(KC_Z), KC_X, KC_C, KC_V, KC_B,                     KC_N, KC_M, KC_COMM, KC_DOT, RCTL_T(KC_SLSH), KC_RSFT,           // bottom row
                              KC_LALT, LOWER, LGUI_T(KC_SPC),      TD(TAP_SPC_ENT), RAISE, KC_RALT                                                  // thumbs
        ),

    // Lower layer: numbers, symbols, mouse keys (held via LOWER). Top/home/
    // bottom rows are exactly the Lily58 marce layout's Lower top/home/
    // bottom rows -- for the bottom row, which is 7 keys/side on Lily58,
    // that means dropping its innermost (extra) column on each side.
    [_LOWER] = LAYOUT_split_3x6_3_ex2(
        KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_TRNS,                           KC_TRNS, KC_MINS, KC_EQL, KC_GRV, KC_LBRC, KC_RBRC, KC_BSLS,     // top row
        KC_TRNS, KC_6, KC_7, KC_8, KC_9, KC_0, KC_TRNS,                          KC_TRNS, MS_LEFT, MS_DOWN, MS_UP, MS_RGHT, KC_NO, KC_TRNS,       // home row
        KC_TRNS, CW_CTL, TO(_NUMERIC), MS_BTN2, MS_BTN3, MS_BTN1,                 MS_WHLL, MS_WHLD, MS_WHLU, MS_WHLR, KC_NO, KC_TRNS,              // bottom row
                              KC_TRNS, KC_TRNS, KC_SPC,            KC_ENT, KC_TRNS, KC_TRNS                                                         // thumbs
        ),

    // Raise layer: function keys, navigation (held via RAISE). Top/home/
    // bottom rows are exactly the Lily58 marce layout's Raise top/home/
    // bottom rows (Lily58's bottom-row innermost columns were both KC_NO,
    // so dropping them to fit 6/side changes nothing).
    [_RAISE] = LAYOUT_split_3x6_3_ex2(
        KC_TRNS, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_TRNS,                      KC_TRNS, KC_PSCR, KC_PAUS, KC_APP, KC_NO, KC_NO, KC_NO,          // top row
        KC_TRNS, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_TRNS,                     KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_INS, KC_NO,        // home row
        KC_TRNS, LCTL_T(KC_F11), KC_F12, KC_F13, KC_F14, KC_F15,                  KC_HOME, KC_END, KC_PGUP, KC_PGDN, LCTL_T(KC_DEL), KC_TRNS,      // bottom row
                              KC_TRNS, KC_TRNS, KC_NO,             KC_NO, KC_TRNS, KC_TRNS                                                          // thumbs
        ),

    // Adjust layer: reboot/bootloader, media keys (LOWER+RAISE). Right half
    // hosts the digitizer point grid, one physical row per grid row.
    [_ADJUST] = LAYOUT_split_3x6_3_ex2(
        QK_BOOT, QK_RBT, ALTGR_MC, KC_NO, TO(_NUMERIC), KC_NO, KC_TRNS,           KC_TRNS, DIG_TL, DIG_TC, DIG_TR, KC_NO, KC_NO, KC_KB_POWER,      // top row
        KC_NO, KC_PAUS, KC_SCRL, KC_NUM, KC_CAPS, KC_NO, KC_TRNS,                 KC_TRNS, DIG_ML, DIG_MC, DIG_MR, KC_NO, KC_NO, KC_NO,            // home row
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,                                 DIG_BL, DIG_BC, DIG_BR, KC_NO, KC_NO, KC_NO,                     // bottom row
                              KC_NO, KC_TRNS, KC_NO,               KC_NO, KC_TRNS, KC_NO                                                            // thumbs
        ),

    // Numeric layer: numpad on the right hand, arrows on the left, entered/
    // exited via TO(_NUMERIC)/TO(0) (TO(0) sits directly below TAB in the
    // top-left corner). The numpad's operator column (/,*,-) and its
    // rightmost digit column (9,6,3) both moved one column left onto the
    // left hand, right next to the arrow column. LOWER/RAISE sit in the
    // same thumb slots as every other layer; Space/Enter moved to the
    // remaining inner thumb keys, and Shift/Ctrl/Alt fill the otherwise-
    // unused left bottom row (mirrored by RCTL/RSFT flanking KC_PDOT on the
    // right). LOWER/RAISE stay held-only escapes back to those layers,
    // returning to _NUMERIC on release. The left outer thumb key falls
    // through to base layer's Alt; the right one is KC_P0 instead (below
    // KC_PDOT, which sits in the same spot as KC_DOT on the base layer).
    [_NUMERIC] = LAYOUT_split_3x6_3_ex2(
        KC_TAB, KC_NO, KC_UP, KC_NO, KC_PSLS, KC_P9, KC_NO,                       KC_NO, KC_P7, KC_P8, KC_NO, KC_PPLS, KC_NO, KC_BSPC,             // top row
        TO(0), KC_LEFT, KC_DOWN, KC_RGHT, KC_PAST, KC_P6, KC_NO,                  KC_NO, KC_P4, KC_P5, KC_NO, KC_PCMM, KC_NO, KC_NO,               // home row
        KC_LSFT, KC_LCTL, KC_LALT, KC_NO, KC_PMNS, KC_P3,                         KC_P1, KC_P2, KC_RCTL, KC_PDOT, KC_RSFT, KC_PEQL,                // bottom row
                              KC_TRNS, LOWER, KC_SPC,               KC_ENT, RAISE, KC_P0                                                            // thumbs
        ),
};

#ifdef RGB_MATRIX_ENABLE
static void update_rgb_matrix(uint8_t layer);
#endif

layer_state_t layer_state_set_user(layer_state_t state) {
    state = update_tri_layer_state(state, _RAISE, _LOWER, _ADJUST);
#ifdef RGB_MATRIX_ENABLE
    update_rgb_matrix(get_highest_layer(state));
#endif
    return state;
}

// Absolute screen positions for the DIG_* keys, indexed by keycode - DIG_TL.
// Order must match the custom_keycodes enum above.
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
// press is swallowed and the keyboard fakes the us(altgr-intl) XKB AltGr
// layer itself by typing Unicode characters via QMK's UC_MAC Unicode-Hex
// Input, since macOS has no equivalent host-side AltGr layer. The mapping
// below (including the dead-key composition) is transcribed from the
// upstream xkeyboard-config `symbols/us` "intl"/"altgr-intl" sections.

enum dead_key {
    DEAD_NONE = 0,
    DEAD_GRAVE,
    DEAD_ACUTE,
    DEAD_TILDE,
    DEAD_CIRCUMFLEX,
    DEAD_DIAERESIS,
    DEAD_CEDILLA,
    DEAD_OGONEK,
    DEAD_BREVE,
    DEAD_ABOVERING,
    DEAD_DOUBLEACUTE,
    DEAD_MACRON,
    DEAD_BELOWDOT,
    DEAD_ABOVEDOT,
    DEAD_CARON,
    DEAD_HORN,
    DEAD_HOOK,
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

// Keys with no AltGr mapping in us(altgr-intl) (f, g, h, b, ...) are simply
// absent here, so a lookup miss falls back to typing the plain key.
static const altgr_entry_t altgr_map[] PROGMEM = {
    {KC_GRV, ALTGR_DEAD(DEAD_GRAVE), ALTGR_DEAD(DEAD_TILDE)},
    {KC_1, 0x00B9, 0x00A1},                              // ¹ ¡
    {KC_2, 0x00B2, ALTGR_DEAD(DEAD_DOUBLEACUTE)},         // ²
    {KC_3, 0x00B3, ALTGR_DEAD(DEAD_MACRON)},              // ³
    {KC_4, 0x00A4, 0x00A3},                               // ¤ £
    {KC_5, 0x20AC, ALTGR_DEAD(DEAD_CEDILLA)},             // €
    {KC_6, ALTGR_DEAD(DEAD_CIRCUMFLEX), 0x00BC},          // ¼
    {KC_7, ALTGR_DEAD(DEAD_HORN), 0x00BD},                // ½
    {KC_8, ALTGR_DEAD(DEAD_OGONEK), 0x00BE},              // ¾
    {KC_9, 0x2018, ALTGR_DEAD(DEAD_BREVE)},               // '
    {KC_0, 0x2019, ALTGR_DEAD(DEAD_ABOVERING)},           // '
    {KC_MINS, 0x00A5, ALTGR_DEAD(DEAD_BELOWDOT)},         // ¥
    {KC_EQL, 0x00D7, 0x00F7},                             // × ÷

    {KC_Q, 0x00E4, 0x00C4}, // ä Ä
    {KC_W, 0x00E5, 0x00C5}, // å Å
    {KC_E, 0x00E9, 0x00C9}, // é É
    {KC_R, 0x00EB, 0x00CB}, // ë Ë
    {KC_T, 0x00FE, 0x00DE}, // þ Þ
    {KC_Y, 0x00FC, 0x00DC}, // ü Ü
    {KC_U, 0x00FA, 0x00DA}, // ú Ú
    {KC_I, 0x00ED, 0x00CD}, // í Í
    {KC_O, 0x00F3, 0x00D3}, // ó Ó
    {KC_P, 0x00F6, 0x00D6}, // ö Ö
    {KC_LBRC, 0x00AB, 0x201C}, // « "
    {KC_RBRC, 0x00BB, 0x201D}, // » "

    {KC_A, 0x00E1, 0x00C1}, // á Á
    {KC_S, 0x00DF, 0x00A7}, // ß §
    {KC_D, 0x00F0, 0x00D0}, // ð Ð
    {KC_J, 0x00EF, 0x00CF}, // ï Ï
    {KC_K, 0x0153, 0x0152}, // œ Œ
    {KC_L, 0x00F8, 0x00D8}, // ø Ø
    {KC_SCLN, 0x00B6, 0x00B0}, // ¶ °
    {KC_QUOT, ALTGR_DEAD(DEAD_ACUTE), ALTGR_DEAD(DEAD_DIAERESIS)},

    {KC_Z, 0x00E6, 0x00C6}, // æ Æ
    {KC_X, 0x0153, 0x0152}, // œ Œ (upstream duplicates K's mapping here)
    {KC_C, 0x00A9, 0x00A2}, // © ¢
    {KC_V, 0x00AE, 0x00AE}, // ®
    {KC_N, 0x00F1, 0x00D1}, // ñ Ñ
    {KC_M, 0x00B5, 0x00B5}, // µ
    {KC_COMM, 0x00E7, 0x00C7}, // ç Ç
    {KC_DOT, ALTGR_DEAD(DEAD_ABOVEDOT), ALTGR_DEAD(DEAD_CARON)},
    {KC_SLSH, 0x00BF, ALTGR_DEAD(DEAD_HOOK)}, // ¿
    {KC_BSLS, 0x00AC, 0x00A6},                // ¬ ¦
};

// Spacing glyph typed when a dead key isn't followed by a known combiner
// (0 = no clean standalone glyph, so nothing extra is typed).
static const uint16_t dead_spacing[DEAD_HOOK + 1] PROGMEM = {
    [DEAD_GRAVE]       = 0x0060, // `
    [DEAD_ACUTE]       = 0x00B4, // ´
    [DEAD_TILDE]       = 0x007E, // ~
    [DEAD_CIRCUMFLEX]  = 0x005E, // ^
    [DEAD_DIAERESIS]   = 0x00A8, // ¨
    [DEAD_CEDILLA]     = 0x00B8, // ¸
    [DEAD_OGONEK]      = 0x02DB, // ˛
    [DEAD_BREVE]       = 0x02D8, // ˘
    [DEAD_ABOVERING]   = 0x02DA, // ˚
    [DEAD_DOUBLEACUTE] = 0x02DD, // ˝
    [DEAD_MACRON]      = 0x00AF, // ¯
    [DEAD_ABOVEDOT]    = 0x02D9, // ˙
    [DEAD_CARON]       = 0x02C7, // ˇ
};

typedef struct {
    uint8_t  dead;
    uint16_t keycode;
    uint32_t codepoint; // lowercase/base form; see altgr_dead_case_adjust
} dead_combo_t;

static const dead_combo_t dead_combos[] PROGMEM = {
    {DEAD_GRAVE, KC_A, 0x00E0}, {DEAD_GRAVE, KC_E, 0x00E8}, {DEAD_GRAVE, KC_I, 0x00EC}, {DEAD_GRAVE, KC_O, 0x00F2}, {DEAD_GRAVE, KC_U, 0x00F9},

    {DEAD_ACUTE, KC_A, 0x00E1}, {DEAD_ACUTE, KC_E, 0x00E9}, {DEAD_ACUTE, KC_I, 0x00ED}, {DEAD_ACUTE, KC_O, 0x00F3}, {DEAD_ACUTE, KC_U, 0x00FA}, {DEAD_ACUTE, KC_Y, 0x00FD},

    {DEAD_TILDE, KC_A, 0x00E3}, {DEAD_TILDE, KC_N, 0x00F1}, {DEAD_TILDE, KC_O, 0x00F5},

    {DEAD_CIRCUMFLEX, KC_A, 0x00E2}, {DEAD_CIRCUMFLEX, KC_E, 0x00EA}, {DEAD_CIRCUMFLEX, KC_I, 0x00EE}, {DEAD_CIRCUMFLEX, KC_O, 0x00F4}, {DEAD_CIRCUMFLEX, KC_U, 0x00FB},

    {DEAD_DIAERESIS, KC_A, 0x00E4}, {DEAD_DIAERESIS, KC_E, 0x00EB}, {DEAD_DIAERESIS, KC_I, 0x00EF}, {DEAD_DIAERESIS, KC_O, 0x00F6}, {DEAD_DIAERESIS, KC_U, 0x00FC},

    {DEAD_CEDILLA, KC_C, 0x00E7},

    {DEAD_CARON, KC_Z, 0x017E}, {DEAD_CARON, KC_C, 0x010D}, {DEAD_CARON, KC_S, 0x0161},

    {DEAD_ABOVEDOT, KC_Z, 0x017C},

    {DEAD_OGONEK, KC_A, 0x0105}, {DEAD_OGONEK, KC_E, 0x0119},

    {DEAD_BREVE, KC_A, 0x0103},

    {DEAD_DOUBLEACUTE, KC_O, 0x0151}, {DEAD_DOUBLEACUTE, KC_U, 0x0171},

    {DEAD_MACRON, KC_A, 0x0101}, {DEAD_MACRON, KC_E, 0x0113}, {DEAD_MACRON, KC_I, 0x012B}, {DEAD_MACRON, KC_O, 0x014D}, {DEAD_MACRON, KC_U, 0x016B},

    {DEAD_ABOVERING, KC_A, 0x00E5}, {DEAD_ABOVERING, KC_U, 0x016F},
};

// All dead_combos codepoints above are either Latin-1 Supplement accented
// vowels (uppercase = lowercase - 0x20) or Latin Extended-A pairs in the
// 0100-0177 block (uppercase = lowercase - 1). Both hold for every value
// used here; this is not a general Unicode case-folding rule.
static uint32_t altgr_dead_case_adjust(uint32_t cp, bool shift) {
    if (!shift) return cp;
    if (cp >= 0x00E0 && cp <= 0x00FE && cp != 0x00F7) return cp - 0x20;
    if (cp >= 0x0100 && cp <= 0x0177) return cp - 1;
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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
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
#ifdef RGB_MATRIX_ENABLE
            update_rgb_matrix(get_highest_layer(layer_state));
#endif
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
    return true;
}

void keyboard_post_init_user(void) {
    set_unicode_input_mode(UNICODE_MODE_MACOS);
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
