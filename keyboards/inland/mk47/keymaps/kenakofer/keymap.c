#include QMK_KEYBOARD_H

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

void eeconfig_init_user(void) {  // EEPROM is getting reset!
    user_stats.raw = 0;
}

#define NUM_AUTO_KEYS 5
#define AUTO_INTERVAL 40
enum custom_keycodes {
    AUTO_UP = SAFE_RANGE,
    AUTO_DOWN,
    AUTO_LEFT,
    AUTO_RGHT,
    AUTO_TAB,
    ECHO_BACKS,
    ECHO_CHARS
};
const uint16_t kc[NUM_AUTO_KEYS] = {KC_UP, KC_DOWN, KC_LEFT, KC_RGHT, KC_TAB};


typedef struct {
    bool is_pressed;
    uint16_t timer;
} auto_key_t;

auto_key_t auto_keys[NUM_AUTO_KEYS] = {0};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case AUTO_UP:
        case AUTO_DOWN:
        case AUTO_LEFT:
        case AUTO_RGHT:
        case AUTO_TAB:
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
    }
    if (record->event.pressed && keycode != last_keycode) {
        if (keycode == KC_BSPC) {
            backspaces_typed++;
            if (backspaces_typed % 10 == 0) {
                user_stats.ten_backspaces_typed++;
                eeconfig_update_user(user_stats.raw);
            }
        } else if (keycode != KC_LCTL && keycode != KC_LSFT && keycode != KC_LALT && keycode != KC_LGUI
                    && keycode != KC_RCTL && keycode != KC_RSFT && keycode != KC_RALT && keycode != KC_RGUI
                    // Also ignore the layer keys
                    && keycode != MO(2) && keycode != MO(3) && keycode != MO(4) && keycode != MO(5)
                    // and arrow keys
                    && keycode != KC_UP && keycode != KC_DOWN && keycode != KC_LEFT && keycode != KC_RGHT
                    ) {
            chars_typed++;
            if (chars_typed % 100 == 0) {
                user_stats.hundred_chars_typed++;
                eeconfig_update_user(user_stats.raw);
            }
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
        if (auto_keys[i].is_pressed && timer_elapsed(auto_keys[i].timer) > AUTO_INTERVAL) {
            auto_keys[i].timer = timer_read();
            tap_code(kc[i]);
        }
    }
}

void keyboard_post_init_user(void) {
    user_stats.raw = eeconfig_read_user();
    chars_typed = user_stats.hundred_chars_typed * 100 + 50;
    backspaces_typed = user_stats.ten_backspaces_typed * 10 + 5;

    rgblight_mode_noeeprom(16);
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_F, KC_P, KC_B, KC_J, KC_L, KC_U, KC_Y, KC_QUOT, KC_BSPC,
                MT(MOD_RCTL, KC_TAB), KC_A, KC_R, KC_S, KC_T, KC_G, KC_M, KC_N, KC_E, KC_I, KC_O, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_D, KC_V, KC_K, KC_H, KC_COMM, KC_DOT, KC_UP, MT(MOD_RSFT, KC_SLSH),
                KC_LCTL, KC_LALT, MO(5), KC_LGUI, MO(2), KC_SPC, MO(3), MO(4), KC_LEFT, KC_DOWN, KC_RGHT),
	[1] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
                MT(MOD_RCTL, KC_TAB), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_UP, MT(MOD_RSFT, KC_SLSH),
                KC_LCTL, KC_LALT, MO(5), KC_LGUI, MO(2), KC_SPC, MO(3), MO(4), KC_LEFT, KC_DOWN, KC_RGHT),
	[2] = LAYOUT_planck_mit(
                KC_GRV, RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), KC_BSLS, KC_EQL, KC_LBRC, KC_RBRC, KC_QUOT, KC_TRNS,
                KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_TRNS,
                KC_TRNS, KC_6, KC_7, KC_8, KC_9, KC_0, KC_NO, RSFT(KC_MINS), KC_COMM, KC_DOT, RSFT(KC_SCLN), MT(MOD_RSFT, KC_SLSH),
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[3] = LAYOUT_planck_mit(
                RSFT(KC_GRV), RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), RSFT(KC_BSLS), RSFT(KC_EQL), RSFT(KC_LBRC), RSFT(KC_RBRC), RSFT(KC_QUOT), KC_TRNS,
                KC_TRNS, RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), RSFT(KC_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), KC_TRNS,
                KC_TRNS, RSFT(KC_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), KC_NO, KC_MINS, RSFT(KC_COMM), RSFT(KC_DOT), KC_SCLN, RSFT(KC_SLSH),
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[4] = LAYOUT_planck_mit(
                KC_TRNS, KC_HOME, KC_PGUP, KC_PGDN, KC_END, KC_NO, KC_NO, RCS(KC_TAB), RALT(KC_LEFT), RALT(KC_RGHT), RCTL(KC_TAB), KC_DEL,
                AUTO_TAB, AUTO_LEFT, AUTO_UP, AUTO_DOWN, AUTO_RGHT, KC_NO, KC_WH_U, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT, KC_TRNS,
                RCTL(KC_Z), KC_TRNS, RCTL(KC_X), RCTL(KC_C), RCTL(KC_V), RCTL(KC_V), KC_WH_D, RCTL(KC_LEFT), RCTL(KC_UP), RCTL(KC_DOWN), RCTL(KC_RGHT), KC_RSFT,
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RCTL(KC_MINS), RCTL(KC_EQL), RCS(KC_EQL)),
	[5] = LAYOUT_planck_mit(
                KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, ECHO_BACKS,
                KC_CAPS, KC_BRID, KC_VOLU, KC_VOLD, KC_BRIU, RGB_MOD, RGB_RMOD, KC_NO, KC_NO, KC_F11, KC_F12, ECHO_CHARS,
                KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RGB_VAI, KC_RSFT,
                KC_NO, KC_MUTE, KC_TRNS, KC_NO, KC_NO, KC_MPLY, KC_NO, KC_NO, DF(1), RGB_VAD, DF(0))
};

#if defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {

};
#endif // defined(ENCODER_ENABLE) &&


const key_override_t delete_key_override = ko_make_basic(MOD_MASK_CTRL, KC_BSPC, KC_DEL);
const key_override_t colemak_ctrl_v_override = ko_make_with_layers(MOD_MASK_CTRL, KC_D, LCTL(KC_V), 1);
const key_override_t colemak_ctrl_d_override = ko_make_with_layers(MOD_MASK_CTRL, KC_V, LCTL(KC_D), 1);

// This globally defines all key overrides to be used
const key_override_t **key_overrides = (const key_override_t *[]){
	&delete_key_override,
    &colemak_ctrl_v_override,
    &colemak_ctrl_d_override,
	NULL // Null terminate the array of overrides!
};

char laydef = 'C'; //variable current default layer; 'C' for Colemak and 'Q' for Qwerty

void set_hue(uint16_t hue) {
    // Get the current HSV values
    HSV curr = rgblight_get_hsv();

    // Set the new HSV values, changing only the hue
    rgblight_sethsv(hue, curr.s, curr.v);
}

//call default layer state
layer_state_t default_layer_state_set_user(layer_state_t state) {
    switch(biton32(state)){
        case 0:
        laydef = 'C';
        set_hue(0xEE);
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
        set_hue (0xCC);
        break;
    case 5:
        set_hue (0x44);
        break;
    default: //  for any other layers, or the default layer
        if(laydef == 'Q'){ //check if current default layer is _NUM
            set_hue (0x20);
        } else {
            set_hue(0xEE);
        }
        break;
    }
  return state;
}
