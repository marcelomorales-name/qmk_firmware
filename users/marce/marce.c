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

    return true;
}
