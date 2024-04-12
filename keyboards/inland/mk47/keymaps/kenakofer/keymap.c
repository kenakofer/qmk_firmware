#include QMK_KEYBOARD_H
// clang-format off

typedef union {
  uint32_t raw;
  struct {
    uint16_t hundred_chars_typed;
    uint16_t ten_backspaces_typed;
  };
} user_stats_t;

user_stats_t user_stats;

uint32_t chars_typed = 0;
uint32_t backspaces_typed = 0;
uint16_t last_keycode = 0;
uint16_t typing_timer = 0;

void eeconfig_init_user(void) {  // EEPROM is getting reset!
    user_stats.raw = 0;
}

#define NUM_AUTO_KEYS 6
#define AUTO_INTERVAL 40
enum custom_keycodes {
    AUTO_UP = SAFE_RANGE,
    AUTO_DOWN,
    AUTO_LEFT,
    AUTO_RGHT,
    AUTO_TAB,
    AUTO_CLICK,
    ECHO_BACKS,
    ECHO_CHARS,
    RAISE_6,
    RAISE_SLASH,
};
const uint16_t kc[NUM_AUTO_KEYS] = {KC_UP, KC_DOWN, KC_LEFT, KC_RGHT, KC_TAB, KC_MS_BTN1};
const uint16_t auto_intervals[NUM_AUTO_KEYS] = {AUTO_INTERVAL, AUTO_INTERVAL, AUTO_INTERVAL, AUTO_INTERVAL, AUTO_INTERVAL, 100};


typedef struct {
    bool is_pressed;
    uint16_t timer;
} auto_key_t;

auto_key_t auto_keys[NUM_AUTO_KEYS] = {0};

char laydef = 'C'; //variable current default layer; 'C' for Colemak and 'Q' for Qwerty

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case AUTO_UP:
        case AUTO_DOWN:
        case AUTO_LEFT:
        case AUTO_RGHT:
        case AUTO_TAB:
        case AUTO_CLICK:
            if (record->event.pressed && !auto_keys[keycode - AUTO_UP].is_pressed) {
                auto_keys[keycode - AUTO_UP].is_pressed = true;
                auto_keys[keycode - AUTO_UP].timer = timer_read();
                tap_code(kc[keycode - AUTO_UP]);
            } else {
                auto_keys[keycode - AUTO_UP].is_pressed = false;
            }
            return false;
        case ECHO_BACKS:
            if (record->event.pressed) {
                // Send the number of backspaces typed as a string
                char buf[8];
                sprintf(buf, "%ld", backspaces_typed);
                SEND_STRING(buf);
            }
            return false;
        case ECHO_CHARS:
            if (record->event.pressed) {
                // Send the number of characters typed as a string
                char buf[8];
                sprintf(buf, "%ld", chars_typed);
                SEND_STRING(buf);
            }
            return false;
        case MT(MOD_LSFT, RAISE_6): // Fix: https://github.com/qmk/qmk_firmware/blob/master/docs/mod_tap.md#caveats
            if (record->tap.count && record->event.pressed) {
                tap_code16(RSFT(KC_6));
                return false;
            }
            break;
        case MT(MOD_RSFT, RAISE_SLASH): // Fix: https://github.com/qmk/qmk_firmware/blob/master/docs/mod_tap.md#caveats
            if (record->tap.count && record->event.pressed) {
                tap_code16(RSFT(KC_SLSH));
                return false;
            }
            break;
    }
    // Only count non-repeated keys pressed while set to Colemak
    if (record->event.pressed && keycode != last_keycode && laydef == 'C') {
        if (keycode == KC_BSPC) {
            // Not after Enter, and not if 2 seconds have passed since the last typed key
            if (last_keycode != KC_ENT && timer_elapsed(typing_timer) < 2000) {
                backspaces_typed++;
                if (backspaces_typed % 10 == 0) {
                    user_stats.ten_backspaces_typed++;
                    eeconfig_update_user(user_stats.raw);
                }
            }
        // Ignore modifier keys
        } else if (keycode != KC_LCTL && keycode != KC_LSFT && keycode != KC_LALT && keycode != KC_LGUI
                    && keycode != KC_RCTL && keycode != KC_RSFT && keycode != KC_RALT && keycode != KC_RGUI
                    // Also ignore the layer keys
                    && keycode != MO(2) && keycode != MO(3) && keycode != MO(4) && keycode != MO(5) && keycode != MO(6)
                    // and arrow keys
                    && keycode != KC_UP && keycode != KC_DOWN && keycode != KC_LEFT && keycode != KC_RGHT
                    ) {
            chars_typed++;
            if (chars_typed % 100 == 0) {
                user_stats.hundred_chars_typed++;
                eeconfig_update_user(user_stats.raw);
            }
            // Start the timer
            typing_timer = timer_read();
        }
        last_keycode = keycode;
    }
    return true;
}

// The array of keys to color white
const uint8_t white_keys[] = {0, 11, 12, 23, 25, 34, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46};
// Flicker the F and J keys
const uint8_t flicker_keys[] = {16, 19};

bool rgb_matrix_indicators_user(void) {
    HSV curr = rgblight_get_hsv();
    for (int i = 0; i < sizeof(white_keys); i++) {
        rgb_matrix_set_color(white_keys[i], curr.v, curr.v, curr.v); // Set the color of a specific key
    }
    // Caps Lock indicator, every .25 seconds
    if (host_keyboard_led_state().caps_lock && timer_read32() % 500 < 250) {
        rgb_matrix_set_color(12, 0, 0, 0);
    }
    for (int i = 0; i < sizeof(flicker_keys); i++) {
        //if (timer_read32() % 100 < 50) {
            rgb_matrix_set_color(flicker_keys[i], curr.v, curr.v, curr.v);
        //}
    }

    return false;
}


void matrix_scan_user(void) {

    for (int i = 0; i < NUM_AUTO_KEYS; i++) {
        if (auto_keys[i].is_pressed && timer_elapsed(auto_keys[i].timer) > auto_intervals[i]) {
            auto_keys[i].timer = timer_read();
            tap_code(kc[i]);
        }
    }
}

void keyboard_post_init_user(void) {
    // debug_enable   = true;
    // debug_matrix   = true;
    // debug_keyboard = true;
    // debug_mouse    = true;

    user_stats.raw   = eeconfig_read_user();
    chars_typed      = user_stats.hundred_chars_typed * 100 + 50;
    backspaces_typed = user_stats.ten_backspaces_typed * 10 + 5;

    rgblight_mode_noeeprom(16);
}

#define COLEMAK_LAYER 0
#define QWERTY_LAYER 1
#define GUI_LAYER 2
#define LOWER_LAYER 3
#define RAISE_LAYER 4
#define NAV_LAYER 5
#define MIDI_LAYER 6
#define FN_LAYER 7

// clang-format off

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[COLEMAK_LAYER] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_F, KC_P, KC_B, KC_J, KC_L, KC_U, KC_Y, KC_QUOT, KC_BSPC,
                MT(MOD_RCTL, KC_TAB), KC_A, KC_R, KC_S, KC_T, KC_G, KC_M, KC_N, KC_E, KC_I, KC_O, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_D, KC_V, KC_K, KC_H, KC_COMM, KC_DOT, KC_UP, MT(MOD_RSFT, KC_SLSH),
                KC_LCTL, KC_LALT, MO(FN_LAYER), LM(GUI_LAYER, MOD_LGUI), MO(LOWER_LAYER), KC_SPC, MO(RAISE_LAYER), MO(NAV_LAYER), KC_LEFT, KC_DOWN, KC_RGHT),
	[QWERTY_LAYER] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
                KC_TAB, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_UP, MT(MOD_RSFT, KC_SLSH),
                KC_LCTL, KC_LALT, MO(FN_LAYER), LM(GUI_LAYER, MOD_LGUI), MO(LOWER_LAYER), KC_SPC, MO(RAISE_LAYER), MO(NAV_LAYER), KC_LEFT, KC_DOWN, KC_RGHT),
	[GUI_LAYER] = LAYOUT_planck_mit(
                KC_GRV, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
                KC_TAB, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_UP, MT(MOD_RSFT, KC_SLSH),
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SPC, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[LOWER_LAYER] = LAYOUT_planck_mit(
                KC_GRV, RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), KC_BSLS, KC_EQL, KC_LBRC, KC_RBRC, KC_QUOT, KC_TRNS,
                KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_TRNS,
                KC_TRNS, MT(MOD_LSFT, KC_6), KC_7, KC_8, KC_9, KC_0, KC_NO, RSFT(KC_MINS), KC_COMM, KC_DOT, RSFT(KC_SCLN), MT(MOD_RSFT, KC_SLSH),
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[RAISE_LAYER] = LAYOUT_planck_mit(
                RSFT(KC_GRV), RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), RSFT(KC_BSLS), RSFT(KC_EQL), RSFT(KC_LBRC), RSFT(KC_RBRC), RSFT(KC_QUOT), KC_TRNS,
                KC_TRNS, RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), RSFT(KC_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), KC_TRNS,
                KC_TRNS, MT(MOD_LSFT, RAISE_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), KC_NO, KC_MINS, RSFT(KC_COMM), RSFT(KC_DOT), KC_SCLN, MT(MOD_RSFT, RAISE_SLASH),
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[NAV_LAYER] = LAYOUT_planck_mit(
                KC_TRNS, KC_WH_L, KC_WH_D, KC_WH_U, KC_WH_R, KC_PGUP, KC_NO, RCS(KC_TAB), RALT(KC_LEFT), RALT(KC_RGHT), RCTL(KC_TAB), KC_DEL,
                AUTO_TAB, AUTO_LEFT, AUTO_DOWN, AUTO_UP, AUTO_RGHT, KC_PGDN, KC_NO, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_TRNS,
                RCTL(KC_Z), KC_LSFT, RCTL(KC_X), RCTL(KC_C), RCTL(KC_V), KC_HOME, KC_END, RCTL(KC_LEFT), RCTL(KC_DOWN), RCTL(KC_UP), RCTL(KC_RGHT), KC_RSFT,
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RCTL(KC_MINS), RCTL(KC_EQL), RCS(KC_EQL)),
    [MIDI_LAYER] = LAYOUT_planck_mit(
        // One octave per row (12 semitones), top row is highest, left to right is C to B
        // 1st octave
        QK_MIDI_NOTE_C_4, QK_MIDI_NOTE_C_4, QK_MIDI_NOTE_D_4, QK_MIDI_NOTE_D_SHARP_4, QK_MIDI_NOTE_E_4, QK_MIDI_NOTE_F_4, QK_MIDI_NOTE_F_SHARP_4, QK_MIDI_NOTE_G_4, QK_MIDI_NOTE_G_SHARP_4, QK_MIDI_NOTE_A_4, QK_MIDI_NOTE_A_SHARP_4, QK_MIDI_NOTE_B_4,
        // 2nd octave
        QK_MIDI_NOTE_C_3, QK_MIDI_NOTE_C_SHARP_3, QK_MIDI_NOTE_D_3, QK_MIDI_NOTE_D_SHARP_3, QK_MIDI_NOTE_E_3, QK_MIDI_NOTE_F_3, QK_MIDI_NOTE_F_SHARP_3, QK_MIDI_NOTE_G_3, QK_MIDI_NOTE_G_SHARP_3, QK_MIDI_NOTE_A_3, QK_MIDI_NOTE_A_SHARP_3, QK_MIDI_NOTE_B_3,
        // 3rd octave
        QK_MIDI_NOTE_C_2, QK_MIDI_NOTE_C_SHARP_2, QK_MIDI_NOTE_D_2, QK_MIDI_NOTE_D_SHARP_2, QK_MIDI_NOTE_E_2, QK_MIDI_NOTE_F_2, QK_MIDI_NOTE_F_SHARP_2, QK_MIDI_NOTE_G_2, QK_MIDI_NOTE_G_SHARP_2, QK_MIDI_NOTE_A_2, QK_MIDI_NOTE_A_SHARP_2, QK_MIDI_NOTE_B_2,
        QK_MIDI_OFF, QK_MIDI_ON, MO(FN_LAYER), KC_NO, QK_MIDI_PITCH_BEND_DOWN, KC_NO, QK_MIDI_PITCH_BEND_UP, KC_NO, KC_NO, KC_NO, KC_NO),

	[FN_LAYER] = LAYOUT_planck_mit(
                KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, ECHO_BACKS,
                KC_CAPS, KC_BRID, KC_VOLD, KC_VOLU, KC_BRIU, RGB_MOD, RGB_RMOD, KC_NO, KC_NO, KC_F11, KC_F12, ECHO_CHARS,
                KC_F1, KC_LSFT, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, RGB_VAI, KC_RSFT,
                AUTO_CLICK, KC_MUTE, KC_TRNS, KC_NO, KC_NO, KC_MPLY, KC_NO, DF(MIDI_LAYER), DF(QWERTY_LAYER), RGB_VAD, DF(COLEMAK_LAYER))
};

#if defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {

};
#endif // defined(ENCODER_ENABLE) &&


const key_override_t colemak_ctrl_v_override = ko_make_with_layers(MOD_MASK_CTRL, KC_D, LCTL(KC_V), 1);
const key_override_t colemak_ctrl_d_override = ko_make_with_layers(MOD_MASK_CTRL, KC_V, LCTL(KC_D), 1);

// Prevents the shift key from affecting the lower layer
#define PREVENT_SHIFT_MASK (1 << LOWER_LAYER)
const key_override_t prevent_shift_1    = ko_make_with_layers(MOD_MASK_SHIFT, KC_1, KC_1, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_2    = ko_make_with_layers(MOD_MASK_SHIFT, KC_2, KC_2, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_3    = ko_make_with_layers(MOD_MASK_SHIFT, KC_3, KC_3, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_4    = ko_make_with_layers(MOD_MASK_SHIFT, KC_4, KC_4, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_5    = ko_make_with_layers(MOD_MASK_SHIFT, KC_5, KC_5, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_6    = ko_make_with_layers(MOD_MASK_SHIFT, KC_6, KC_6, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_7    = ko_make_with_layers(MOD_MASK_SHIFT, KC_7, KC_7, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_8    = ko_make_with_layers(MOD_MASK_SHIFT, KC_8, KC_8, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_9    = ko_make_with_layers(MOD_MASK_SHIFT, KC_9, KC_9, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_0    = ko_make_with_layers(MOD_MASK_SHIFT, KC_0, KC_0, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_comm = ko_make_with_layers(MOD_MASK_SHIFT, KC_COMM, KC_COMM, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_dot  = ko_make_with_layers(MOD_MASK_SHIFT, KC_DOT, KC_DOT, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_bksl = ko_make_with_layers(MOD_MASK_SHIFT, KC_BSLS, KC_BSLS, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_eql  = ko_make_with_layers(MOD_MASK_SHIFT, KC_EQL, KC_EQL, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_lbrc = ko_make_with_layers(MOD_MASK_SHIFT, KC_LBRC, KC_LBRC, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_rbrc = ko_make_with_layers(MOD_MASK_SHIFT, KC_RBRC, KC_RBRC, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_quot = ko_make_with_layers(MOD_MASK_SHIFT, KC_QUOT, KC_QUOT, PREVENT_SHIFT_MASK);
const key_override_t prevent_shift_sls  = ko_make_with_layers(MOD_MASK_SHIFT, KC_SLSH, KC_SLSH, PREVENT_SHIFT_MASK);

// Prevents the shift key from affecting the upper layer
#define PREVENT_SHIFT_MASK2 (1 << RAISE_LAYER)
const key_override_t prevent_shift_mins = ko_make_with_layers(MOD_MASK_SHIFT, KC_MINS, KC_MINS, PREVENT_SHIFT_MASK2);
const key_override_t prevent_shift_scln = ko_make_with_layers(MOD_MASK_SHIFT, KC_SCLN, KC_SCLN, PREVENT_SHIFT_MASK2);

// clang-format off
// This globally defines all key overrides to be used
const key_override_t **key_overrides = (const key_override_t *[]){
    &colemak_ctrl_v_override,
    &colemak_ctrl_d_override,
    &prevent_shift_1,
    &prevent_shift_2,
    &prevent_shift_3,
    &prevent_shift_4,
    &prevent_shift_5,
    &prevent_shift_6,
    &prevent_shift_7,
    &prevent_shift_8,
    &prevent_shift_9,
    &prevent_shift_0,
    &prevent_shift_comm,
    &prevent_shift_dot,
    &prevent_shift_bksl,
    &prevent_shift_eql,
    &prevent_shift_lbrc,
    &prevent_shift_rbrc,
    &prevent_shift_quot,
    &prevent_shift_sls,
    &prevent_shift_mins,
    &prevent_shift_scln,
    NULL // Null terminate the array of overrides!
};

void set_hue(uint16_t hue) {
    // Get the current HSV values
    HSV curr = rgblight_get_hsv();

    // Set the new HSV values, changing only the hue
    rgblight_sethsv(hue, curr.s, curr.v);
}

//call default layer state
layer_state_t default_layer_state_set_user(layer_state_t state) {
    switch(biton32(state)){
        case COLEMAK_LAYER:
        laydef = 'C';
        set_hue(0xEE);
        break;
        case MIDI_LAYER:
        laydef = 'M';
        set_hue(0x88);
        break;
        default:
        laydef = 'Q';
        set_hue(0x20);
        break;
    };
    return state;
};

layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case 2:
        set_hue (0x88);
        break;
    case 3:
        set_hue (0xAA);
        break;
    case 4:
        set_hue (0x00);
        break;
    case 5:
        set_hue (0xCC);
        break;
    case 6:
        set_hue (0x44);
        break;
    default: //  for any other layers, or the default layer
        if(laydef == 'Q'){
            set_hue (0x20);
        }
        else if(laydef == 'M'){
            set_hue(0x88);
        } else {
            set_hue(0xEE);
        }
        break;
    }
  return state;
}
