#include QMK_KEYBOARD_H
#include "marce.h"

// Shared logic for the lily58 and crkbd "marce" keymaps.

// --- Digitizer point grid ------------------------------------------------

// Absolute screen positions for the DIG_* keys, indexed by keycode - DIG_TL.
// Order must match marce_keycodes in marce.h. Edges are pulled in to 5%/95%
// rather than 0%/100% so the pointer lands inside the screen.
typedef struct {
    float x;
    float y;
} digitizer_point_t;

static const digitizer_point_t digitizer_points[9] PROGMEM = {
    {0.05f, 0.05f}, {0.5f, 0.05f}, {0.95f, 0.05f}, // DIG_TL, DIG_TC, DIG_TR
    {0.05f, 0.5f},  {0.5f, 0.5f},  {0.95f, 0.5f},  // DIG_ML, DIG_MC, DIG_MR
    {0.05f, 0.95f}, {0.5f, 0.95f}, {0.95f, 0.95f}, // DIG_BL, DIG_BC, DIG_BR
};

static bool process_digitizer(uint16_t keycode, keyrecord_t *record) {
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

bool            mac_altgr_mode     = false; // false = PC (passthrough), true = Mac (Unicode-fake)
static bool     ralt_held          = false;
static uint8_t  pending_dead       = DEAD_NONE;
static uint16_t altgr_swallowed_key = KC_NO;

// Called after ALTGR_MC flips mac_altgr_mode, e.g. so a board can recolor its
// RGB. Does nothing by default.
__attribute__((weak)) void mac_altgr_mode_changed_marce_kb(void) {}

static bool process_altgr(uint16_t keycode, keyrecord_t *record) {
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
            mac_altgr_mode_changed_marce_kb();
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

// --- Hold-vs-tap ---------------------------------------------------------

// See the description in marce.h. hold_key names the key currently down.
static uint16_t hold_key         = KC_NO;
static uint16_t hold_timer       = 0;
static bool     hold_interrupted = false;

void hold_key_press(uint16_t keycode) {
    hold_key         = keycode;
    hold_timer       = timer_read();
    hold_interrupted = false;
}

// True when this release should be treated as a tap rather than a hold.
bool hold_key_was_tap(uint16_t keycode) {
    bool tapped = (hold_key == keycode) && !hold_interrupted && timer_elapsed(hold_timer) < TAPPING_TERM;
    if (hold_key == keycode) {
        hold_key = KC_NO;
    }
    return tapped;
}

// --- Dispatcher ----------------------------------------------------------

// Board-specific keycodes are handled here first; return false to swallow the
// key. The shared handlers below only see what this lets through.
__attribute__((weak)) bool process_record_marce_kb(uint16_t keycode, keyrecord_t *record) {
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Any other key going down while a hold-vs-tap key is held settles it as a
    // hold, so its release won't emit the tap action.
    if (record->event.pressed && hold_key != KC_NO && keycode != hold_key) {
        hold_interrupted = true;
    }

    if (!process_record_marce_kb(keycode, record)) {
        return false;
    }

    if (keycode >= DIG_TL && keycode <= DIG_BR) {
        return process_digitizer(keycode, record);
    }

    return process_altgr(keycode, record);
}
