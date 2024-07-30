
#include "pwindow.h"
#include "plimits.h"
#include "pstacktrace.h"
#include "SDL.h"
#include "pkeyboard.h"
#include "util.h"
    
void pkeyboard_init(pdevice_t *const device) {
    pkeyboard_t *keyboard = (void*)device;
    pmemexpand(keyboard->keycodes , &((pkeycode_t){-1}), sizeof(keyboard->keycodes), sizeof(pkeycode_t));
    pmemexpand(keyboard->scancodes, &((usize){-1}), sizeof(keyboard->scancodes), sizeof(usize));
    memset(keyboard->keys, 0, sizeof(keyboard->scancodes));

    keyboard->keycodes[SDL_SCANCODE_A] = PTSD_KEY_A;
    keyboard->keycodes[SDL_SCANCODE_B] = PTSD_KEY_B;
    keyboard->keycodes[SDL_SCANCODE_C] = PTSD_KEY_C;
    keyboard->keycodes[SDL_SCANCODE_D] = PTSD_KEY_D;
    keyboard->keycodes[SDL_SCANCODE_E] = PTSD_KEY_E;
    keyboard->keycodes[SDL_SCANCODE_F] = PTSD_KEY_F;
    keyboard->keycodes[SDL_SCANCODE_G] = PTSD_KEY_G;
    keyboard->keycodes[SDL_SCANCODE_H] = PTSD_KEY_H;
    keyboard->keycodes[SDL_SCANCODE_I] = PTSD_KEY_I;
    keyboard->keycodes[SDL_SCANCODE_J] = PTSD_KEY_J;
    keyboard->keycodes[SDL_SCANCODE_K] = PTSD_KEY_K;
    keyboard->keycodes[SDL_SCANCODE_L] = PTSD_KEY_L;
    keyboard->keycodes[SDL_SCANCODE_M] = PTSD_KEY_M;
    keyboard->keycodes[SDL_SCANCODE_N] = PTSD_KEY_N;
    keyboard->keycodes[SDL_SCANCODE_O] = PTSD_KEY_O;
    keyboard->keycodes[SDL_SCANCODE_P] = PTSD_KEY_P;
    keyboard->keycodes[SDL_SCANCODE_Q] = PTSD_KEY_Q;
    keyboard->keycodes[SDL_SCANCODE_R] = PTSD_KEY_R;
    keyboard->keycodes[SDL_SCANCODE_S] = PTSD_KEY_S;
    keyboard->keycodes[SDL_SCANCODE_T] = PTSD_KEY_T;
    keyboard->keycodes[SDL_SCANCODE_U] = PTSD_KEY_U;
    keyboard->keycodes[SDL_SCANCODE_V] = PTSD_KEY_V;
    keyboard->keycodes[SDL_SCANCODE_W] = PTSD_KEY_W;
    keyboard->keycodes[SDL_SCANCODE_X] = PTSD_KEY_X;
    keyboard->keycodes[SDL_SCANCODE_Y] = PTSD_KEY_Y;
    keyboard->keycodes[SDL_SCANCODE_Z] = PTSD_KEY_Z;

    keyboard->keycodes[SDL_SCANCODE_1] = PTSD_KEY_1;
    keyboard->keycodes[SDL_SCANCODE_2] = PTSD_KEY_2;
    keyboard->keycodes[SDL_SCANCODE_3] = PTSD_KEY_3;
    keyboard->keycodes[SDL_SCANCODE_4] = PTSD_KEY_4;
    keyboard->keycodes[SDL_SCANCODE_5] = PTSD_KEY_5;
    keyboard->keycodes[SDL_SCANCODE_6] = PTSD_KEY_6;
    keyboard->keycodes[SDL_SCANCODE_7] = PTSD_KEY_7;
    keyboard->keycodes[SDL_SCANCODE_8] = PTSD_KEY_8;
    keyboard->keycodes[SDL_SCANCODE_9] = PTSD_KEY_9;
    keyboard->keycodes[SDL_SCANCODE_0] = PTSD_KEY_0;

    keyboard->keycodes[SDL_SCANCODE_RETURN] = PTSD_KEY_ENTER;
    keyboard->keycodes[SDL_SCANCODE_ESCAPE] = PTSD_KEY_ESCAPE;
    keyboard->keycodes[SDL_SCANCODE_BACKSPACE] = PTSD_KEY_BACKSPACE;
    keyboard->keycodes[SDL_SCANCODE_TAB] = PTSD_KEY_TAB;
    keyboard->keycodes[SDL_SCANCODE_SPACE] = PTSD_KEY_SPACE;

    keyboard->keycodes[SDL_SCANCODE_MINUS] = PTSD_KEY_MINUS;
    keyboard->keycodes[SDL_SCANCODE_EQUALS] = PTSD_KEY_EQUAL;
    keyboard->keycodes[SDL_SCANCODE_LEFTBRACKET] = PTSD_KEY_LEFT_BRACKET;
    keyboard->keycodes[SDL_SCANCODE_RIGHTBRACKET] = PTSD_KEY_RIGHT_BRACKET;
    keyboard->keycodes[SDL_SCANCODE_BACKSLASH] = PTSD_KEY_BACKSLASH;
    keyboard->keycodes[SDL_SCANCODE_SEMICOLON] = PTSD_KEY_SEMICOLON;
    keyboard->keycodes[SDL_SCANCODE_APOSTROPHE] = PTSD_KEY_APOSTROPHE;
    keyboard->keycodes[SDL_SCANCODE_GRAVE] = PTSD_KEY_GRAVE_ACCENT;
    keyboard->keycodes[SDL_SCANCODE_COMMA] = PTSD_KEY_COMMA;
    keyboard->keycodes[SDL_SCANCODE_PERIOD] = PTSD_KEY_PERIOD;
    keyboard->keycodes[SDL_SCANCODE_SLASH] = PTSD_KEY_SLASH;

    keyboard->keycodes[SDL_SCANCODE_CAPSLOCK] = PTSD_KEY_CAPS_LOCK;

    keyboard->keycodes[SDL_SCANCODE_F1] = PTSD_KEY_F1;
    keyboard->keycodes[SDL_SCANCODE_F2] = PTSD_KEY_F2;
    keyboard->keycodes[SDL_SCANCODE_F3] = PTSD_KEY_F3;
    keyboard->keycodes[SDL_SCANCODE_F4] = PTSD_KEY_F4;
    keyboard->keycodes[SDL_SCANCODE_F5] = PTSD_KEY_F5;
    keyboard->keycodes[SDL_SCANCODE_F6] = PTSD_KEY_F6;
    keyboard->keycodes[SDL_SCANCODE_F7] = PTSD_KEY_F7;
    keyboard->keycodes[SDL_SCANCODE_F8] = PTSD_KEY_F8;
    keyboard->keycodes[SDL_SCANCODE_F9] = PTSD_KEY_F9;
    keyboard->keycodes[SDL_SCANCODE_F10] = PTSD_KEY_F10;
    keyboard->keycodes[SDL_SCANCODE_F11] = PTSD_KEY_F11;
    keyboard->keycodes[SDL_SCANCODE_F12] = PTSD_KEY_F12;

    keyboard->keycodes[SDL_SCANCODE_PRINTSCREEN] = PTSD_KEY_PRINT_SCREEN;
    keyboard->keycodes[SDL_SCANCODE_SCROLLLOCK] = PTSD_KEY_SCROLL_LOCK;
    keyboard->keycodes[SDL_SCANCODE_PAUSE] = PTSD_KEY_PAUSE;
    keyboard->keycodes[SDL_SCANCODE_INSERT] = PTSD_KEY_INSERT;
    keyboard->keycodes[SDL_SCANCODE_HOME] = PTSD_KEY_HOME;
    keyboard->keycodes[SDL_SCANCODE_PAGEUP] = PTSD_KEY_PAGE_UP;
    keyboard->keycodes[SDL_SCANCODE_DELETE] = PTSD_KEY_DELETE;
    keyboard->keycodes[SDL_SCANCODE_END] = PTSD_KEY_END;
    keyboard->keycodes[SDL_SCANCODE_PAGEDOWN] = PTSD_KEY_PAGE_DOWN;
    keyboard->keycodes[SDL_SCANCODE_RIGHT] = PTSD_KEY_RIGHT;
    keyboard->keycodes[SDL_SCANCODE_LEFT] = PTSD_KEY_LEFT;
    keyboard->keycodes[SDL_SCANCODE_DOWN] = PTSD_KEY_DOWN;
    keyboard->keycodes[SDL_SCANCODE_UP] = PTSD_KEY_UP;

    keyboard->keycodes[SDL_SCANCODE_NUMLOCKCLEAR] = PTSD_KEY_NUM_LOCK;
    keyboard->keycodes[SDL_SCANCODE_KP_DIVIDE] = PTSD_KEY_KP_DIVIDE;
    keyboard->keycodes[SDL_SCANCODE_KP_MULTIPLY] = PTSD_KEY_KP_MULTIPLY;
    keyboard->keycodes[SDL_SCANCODE_KP_MINUS] = PTSD_KEY_KP_SUBTRACT;
    keyboard->keycodes[SDL_SCANCODE_KP_PLUS] = PTSD_KEY_KP_ADD;
    keyboard->keycodes[SDL_SCANCODE_KP_ENTER] = PTSD_KEY_KP_ENTER;
    keyboard->keycodes[SDL_SCANCODE_KP_1] = PTSD_KEY_KP_1;
    keyboard->keycodes[SDL_SCANCODE_KP_2] = PTSD_KEY_KP_2;
    keyboard->keycodes[SDL_SCANCODE_KP_3] = PTSD_KEY_KP_3;
    keyboard->keycodes[SDL_SCANCODE_KP_4] = PTSD_KEY_KP_4;
    keyboard->keycodes[SDL_SCANCODE_KP_5] = PTSD_KEY_KP_5;
    keyboard->keycodes[SDL_SCANCODE_KP_6] = PTSD_KEY_KP_6;
    keyboard->keycodes[SDL_SCANCODE_KP_7] = PTSD_KEY_KP_7;
    keyboard->keycodes[SDL_SCANCODE_KP_8] = PTSD_KEY_KP_8;
    keyboard->keycodes[SDL_SCANCODE_KP_9] = PTSD_KEY_KP_9;
    keyboard->keycodes[SDL_SCANCODE_KP_0] = PTSD_KEY_KP_0;
    keyboard->keycodes[SDL_SCANCODE_KP_PERIOD] = PTSD_KEY_KP_DECIMAL;

    keyboard->keycodes[SDL_SCANCODE_NONUSBACKSLASH] = PTSD_KEY_BACKSLASH;
    keyboard->keycodes[SDL_SCANCODE_KP_EQUALS] = PTSD_KEY_KP_EQUAL;
    keyboard->keycodes[SDL_SCANCODE_F13] = PTSD_KEY_F13;
    keyboard->keycodes[SDL_SCANCODE_F14] = PTSD_KEY_F14;
    keyboard->keycodes[SDL_SCANCODE_F15] = PTSD_KEY_F15;
    keyboard->keycodes[SDL_SCANCODE_F16] = PTSD_KEY_F16;
    keyboard->keycodes[SDL_SCANCODE_F17] = PTSD_KEY_F17;
    keyboard->keycodes[SDL_SCANCODE_F18] = PTSD_KEY_F18;
    keyboard->keycodes[SDL_SCANCODE_F19] = PTSD_KEY_F19;
    keyboard->keycodes[SDL_SCANCODE_F20] = PTSD_KEY_F20;
    keyboard->keycodes[SDL_SCANCODE_F21] = PTSD_KEY_F21;
    keyboard->keycodes[SDL_SCANCODE_F22] = PTSD_KEY_F22;
    keyboard->keycodes[SDL_SCANCODE_F23] = PTSD_KEY_F23;
    keyboard->keycodes[SDL_SCANCODE_F24] = PTSD_KEY_F24;
    keyboard->keycodes[SDL_SCANCODE_MENU] = PTSD_KEY_MENU;

    keyboard->keycodes[SDL_SCANCODE_LCTRL] = PTSD_KEY_LEFT_CONTROL;
    keyboard->keycodes[SDL_SCANCODE_LSHIFT] = PTSD_KEY_LEFT_SHIFT;
    keyboard->keycodes[SDL_SCANCODE_LALT] = PTSD_KEY_LEFT_ALT;
    keyboard->keycodes[SDL_SCANCODE_LGUI] = PTSD_KEY_LEFT_SUPER;
    keyboard->keycodes[SDL_SCANCODE_RCTRL] = PTSD_KEY_RIGHT_CONTROL;
    keyboard->keycodes[SDL_SCANCODE_RSHIFT] = PTSD_KEY_RIGHT_SHIFT;
    keyboard->keycodes[SDL_SCANCODE_RALT] = PTSD_KEY_RIGHT_ALT;
    keyboard->keycodes[SDL_SCANCODE_RGUI] = PTSD_KEY_RIGHT_SUPER;

    for (u32 scancode = 0; scancode < 512; scancode++) {
        if (keyboard->keycodes[scancode] != -1)
            keyboard->scancodes[keyboard->keycodes[scancode]] = scancode;
    }
}

pmodifier_t pkb_get_modifiers(const pkeyboard_t *const) {
    pmodifier_t mods = 0;
    SDL_Keymod keymods = SDL_GetModState();
    if (keymods & KMOD_SHIFT)
        mods |= PTSD_MOD_SHIFT;
    if (keymods & KMOD_CTRL)
        mods |= PTSD_MOD_CONTROL;
    if (keymods & KMOD_ALT)
        mods |= PTSD_MOD_ALT;
    if (keymods & KMOD_GUI)
        mods |= PTSD_MOD_SUPER;
    if (keymods & KMOD_CAPS)
        mods |= PTSD_MOD_CAPS_LOCK;
    if (keymods & KMOD_NUM)
        mods |= PTSD_MOD_NUM_LOCK;
    return mods;
}

void pkeyboard_shutdown(pdevice_t *const) { }
void pkeyboard_update(pdevice_t *const device) {
    pkeyboard_t *keyboard = (void*)device;
    static pkey_state_t oldkeystate[512] = {0};
    pmodifier_t mods = pkb_get_modifiers(keyboard);

    pkey_state_t *keystate = keyboard->keys;
    for (register usize i = 0; i < 512; i++) {
        keystate->modifiers = mods;
        if (keystate[i].pressed) {
            if (!oldkeystate[i].pressed) {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = true;
                keystate[i].held     = true;
            }
            else {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = false;
                keystate[i].held     = true;
            }
        }
        else if (keystate[i].released) {
            if (oldkeystate[i].released) {
                oldkeystate[i] = keystate[i];
                keystate[i].released = true;
                keystate[i].pressed  = false;
                keystate[i].held     = false;
            }
            else {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = false;
                keystate[i].held     = false;
            }
        }
        else if (keystate[i].held) {
            oldkeystate[i] = keystate[i];
            keystate[i].released = false;
            keystate[i].pressed  = false;
        }
    }
}

void pkb_update_keystate(pkeyboard_t *const keyboard, pkeycode_t keycode, pkey_action_state_t newstate) {
    if (keycode < 0 || keycode >= PTSD_KEY_COUNT) return;
    pmodifier_t mods = pkb_get_modifiers(keyboard);
    pkey_state_t *key = &keyboard->keys[keycode];

    int combined_states = (key->action << 2) | newstate;
    switch (combined_states) {
    case 0b0000: // RELEASED | RELEASED
        key->action    = PKB_RELEASED;
        key->modifiers = mods;
        key->code      = keycode;
        key->released = true;
        break;
    case 0b0001: // RELEASED | PRESSED
        key->action    = PKB_PRESSED;
        key->modifiers = mods;
        key->code      = keycode;
        key->pressed = true;
        break;
    case 0b0010: // RELEASED | HELD
        key->action    = PKB_HELD;
        key->modifiers = mods;
        key->code      = keycode;
        break;

    case 0b0100: // PRESSED | RELEASED
        key->action    = PKB_RELEASED;
        key->modifiers = mods;
        key->code      = keycode;
        key->released = true;
        break;
    case 0b0101: // PRESSED | PRESSED
        key->action    = PKB_HELD;
        key->modifiers = mods;
        key->code      = keycode;
        break;
    case 0b0110: // PRESSED | HELD
        key->action    = PKB_HELD;
        key->modifiers = mods;
        key->code      = keycode;
        break;
    
    case 0b1000: // HELD    | RELEASED
        key->action    = PKB_RELEASED;
        key->modifiers = mods;
        key->code      = keycode;
        key->released = true;
        break;
    case 0b1001: // HELD    | PRESSED
        key->action    = PKB_HELD;
        key->modifiers = mods;
        key->code      = keycode;
        break;
    case 0b1010: // HELD    | HELD
        key->action    = PKB_HELD;
        key->modifiers = mods;
        key->code      = keycode;
        break;
    }
}

bool pkeyboard_device_proc(pdevice_t *const device, const SDL_Event *const event) {
    pkeyboard_t *const keyboard = (void* const)device;

    switch(event->type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
            int repeats  = event->key.repeat;
            
            pkey_action_state_t action   = event->key.state == SDL_RELEASED ? PKB_RELEASED : PKB_PRESSED;
            if (action && repeats > 1) action = PKB_HELD;

            
            unsigned int scancode = event->key.keysym.scancode;
            if (!scancode) {
                return true;
            }
            pkeycode_t key = keyboard->keycodes[scancode];
            keyboard->last_key = key;
            pkb_update_keystate(keyboard, key, action);
        }
        return true;
    }
    return (pdevice_proc_result_t){0};
}

pkey_state_t pkb_get_key(const pkeyboard_t *const kb, pkeycode_t code) {
    if (code < 0 || code >= PTSD_KEY_COUNT) return (pkey_state_t){.code = PTSD_KEY_UNKNOWN};
    if (code >= 'a' || code <= 'z')
        code = toupper(code);
    return kb->keys[code];
}

bool pkb_is_pressed(const pkeyboard_t *const kb, pkeycode_t code) {
    return pkb_get_key(kb, code).pressed;
}

bool pkb_is_released(const pkeyboard_t *const kb, pkeycode_t code) {
    return pkb_get_key(kb, code).released;
}

bool pkb_is_held(const pkeyboard_t *const kb, pkeycode_t code) {
    return pkb_get_key(kb, code).held;
}

pkey_state_t pkb_get_last_key(const pkeyboard_t *const kb) {
    return pkb_get_key(kb, kb->last_key);
}

bool pkb_any_key_pressed(const pkeyboard_t *kb) {
    if (kb->last_key == PTSD_KEY_UNKNOWN) return false;
    pkey_state_t key = pkb_get_last_key(kb);
    if (key.pressed || key.held || key.action != PKB_RELEASED)
         return true;
    else return false;
}
