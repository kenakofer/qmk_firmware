#include QMK_KEYBOARD_H

#define NUM_AUTO_KEYS 5
#define AUTO_INTERVAL 40
enum custom_keycodes {
    AUTO_UP = SAFE_RANGE,
    AUTO_DOWN,
    AUTO_LEFT,
    AUTO_RGHT,
    AUTO_TAB
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
    }
    return true;
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
    rgblight_mode_noeeprom(16);
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_F, KC_P, KC_B, KC_J, KC_L, KC_U, KC_Y, KC_QUOT, KC_BSPC,
                MT(MOD_RCTL, KC_TAB), KC_A, KC_R, KC_S, KC_T, KC_G, KC_M, KC_N, KC_E, KC_I, KC_O, KC_ENT,
                KC_Z, KC_LSFT, KC_X, KC_C, KC_D, KC_V, KC_K, KC_H, KC_COMM, KC_DOT, KC_UP, KC_SLSH,
                KC_LCTL, KC_LALT, MO(5), KC_LGUI, MO(2), KC_SPC, MO(3), MO(4), KC_LEFT, KC_DOWN, KC_RGHT),
	[1] = LAYOUT_planck_mit(
                KC_ESC, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC, 
                MT(MOD_RCTL, KC_TAB), KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT, 
                KC_Z, KC_LSFT, KC_X, KC_C, KC_D, KC_V, KC_K, KC_H, KC_COMM, KC_DOT, KC_UP, KC_TRNS, 
                KC_LCTL, KC_LALT, MO(5), KC_LGUI, MO(2), KC_SPC, MO(3), MO(4), KC_LEFT, KC_DOWN, KC_RGHT),
	[2] = LAYOUT_planck_mit(
                KC_TRNS, KC_GRV, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_BSLS, KC_LBRC, KC_RBRC, KC_QUOT, KC_EQL, 
                KC_TRNS, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, 
                KC_TRNS, KC_6, KC_7, KC_8, KC_9, KC_0, KC_NO, KC_SCLN, KC_COMM, KC_DOT, KC_TRNS, KC_SLSH, 
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[3] = LAYOUT_planck_mit(
                KC_TRNS, RSFT(KC_GRV), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, RSFT(KC_BSLS), RSFT(KC_LBRC), RSFT(KC_RBRC), RSFT(KC_QUOT), RSFT(KC_EQL), 
                KC_TRNS, RSFT(KC_1), RSFT(KC_2), RSFT(KC_3), RSFT(KC_4), RSFT(KC_5), RSFT(KC_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), RSFT(KC_MINS), 
                KC_TRNS, RSFT(KC_6), RSFT(KC_7), RSFT(KC_8), RSFT(KC_9), RSFT(KC_0), KC_NO, RSFT(KC_SCLN), RSFT(KC_COMM), RSFT(KC_DOT), RSFT(KC_TRNS), RSFT(KC_SLSH), 
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[4] = LAYOUT_planck_mit(
                KC_TRNS, KC_HOME, KC_PGUP, KC_PGDN, KC_END, KC_NO, KC_NO, RCS(KC_TAB), RALT(KC_LEFT), RALT(KC_RGHT), RCTL(KC_TAB), RCS(KC_EQL), 
                AUTO_TAB, AUTO_LEFT, AUTO_UP, AUTO_DOWN, AUTO_RGHT, KC_NO, KC_NO, KC_LEFT, KC_UP, KC_DOWN, KC_RGHT, RCTL(KC_MINS), 
                RCTL(KC_Z), KC_TRNS, RCTL(KC_X), RCTL(KC_C), RCTL(KC_V), KC_NO, KC_NO, RCTL(KC_LEFT), RCTL(KC_UP), RCTL(KC_DOWN), RCTL(KC_RGHT), KC_NO, 
                KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[5] = LAYOUT_planck_mit(
                KC_NO, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, 
                KC_NO, KC_BRID, KC_VOLU, KC_VOLD, KC_BRIU, RGB_MOD, RGB_RMOD, BL_STEP, KC_NO, KC_NO, KC_NO, KC_F12, 
                KC_NO, KC_F9, KC_F10, KC_F11, KC_F12, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, 
                KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO, KC_MPLY, KC_NO, KC_NO, KC_NO, DF(1), DF(0))
};

#if defined(ENCODER_ENABLE) && defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {

};
#endif // defined(ENCODER_ENABLE) && 

char laydef = 'C'; //variable current default layer; 'C' for Colemak and 'Q' for Qwerty

//call default layer state
layer_state_t default_layer_state_set_user(layer_state_t state) {
    switch(biton32(state)){
        case 0:
        laydef = 'C';
        rgblight_sethsv(0xEE, 0xFF, 0x66);
        register_code(KC_LGUI);
        tap_code(KC_F22);
        unregister_code(KC_LGUI);
        break;
        default:
        laydef = 'Q';
        rgblight_sethsv (0x00,  0xFF, 0xFF);
        register_code(KC_LGUI);
        tap_code(KC_F23);
        unregister_code(KC_LGUI);
        break;
    };
    return state;
};

layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case 2:
        rgblight_sethsv (0x88,  0xFF, 0xFF);
        break;
    case 3:
        rgblight_sethsv (0xAA,  0xFF, 0xFF);
        break;
    case 4:
        rgblight_sethsv (0xCC,  0xFF, 0xFF);
        break;
    case 5:
        rgblight_sethsv (0x44,  0xFF, 0xFF);
        break;
    default: //  for any other layers, or the default layer
        if(laydef == 'Q'){ //check if current default layer is _NUM
            rgblight_sethsv (0x00,  0xFF, 0xFF);
        } else {
            rgblight_sethsv(0xEE, 0xFF, 0x66);
        }
        break;
    }
  return state;
}





