
#include "pwindow.h"
#include "pkeyboard.h"
#include "util.h"
#include <windows.h>

void pkeyboard_init(pdevice_t *const device) {
    pkeyboard_t *keyboard = (void*)device;
    pmemexpand(keyboard->keycodes , &((pkeycode_t){-1}), sizeof(keyboard->keycodes), sizeof(pkeycode_t));
    pmemexpand(keyboard->scancodes, &((usize){-1}), sizeof(keyboard->scancodes), sizeof(usize));
    memset(keyboard->keys, 0, sizeof(keyboard->scancodes));


    keyboard->keycodes[0x00B] = PTSD_KEY_0;
    keyboard->keycodes[0x002] = PTSD_KEY_1;
    keyboard->keycodes[0x003] = PTSD_KEY_2;
    keyboard->keycodes[0x004] = PTSD_KEY_3;
    keyboard->keycodes[0x005] = PTSD_KEY_4;
    keyboard->keycodes[0x006] = PTSD_KEY_5;
    keyboard->keycodes[0x007] = PTSD_KEY_6;
    keyboard->keycodes[0x008] = PTSD_KEY_7;
    keyboard->keycodes[0x009] = PTSD_KEY_8;
    keyboard->keycodes[0x00A] = PTSD_KEY_9;
    keyboard->keycodes[0x01E] = PTSD_KEY_A;
    keyboard->keycodes[0x030] = PTSD_KEY_B;
    keyboard->keycodes[0x02E] = PTSD_KEY_C;
    keyboard->keycodes[0x020] = PTSD_KEY_D;
    keyboard->keycodes[0x012] = PTSD_KEY_E;
    keyboard->keycodes[0x021] = PTSD_KEY_F;
    keyboard->keycodes[0x022] = PTSD_KEY_G;
    keyboard->keycodes[0x023] = PTSD_KEY_H;
    keyboard->keycodes[0x017] = PTSD_KEY_I;
    keyboard->keycodes[0x024] = PTSD_KEY_J;
    keyboard->keycodes[0x025] = PTSD_KEY_K;
    keyboard->keycodes[0x026] = PTSD_KEY_L;
    keyboard->keycodes[0x032] = PTSD_KEY_M;
    keyboard->keycodes[0x031] = PTSD_KEY_N;
    keyboard->keycodes[0x018] = PTSD_KEY_O;
    keyboard->keycodes[0x019] = PTSD_KEY_P;
    keyboard->keycodes[0x010] = PTSD_KEY_Q;
    keyboard->keycodes[0x013] = PTSD_KEY_R;
    keyboard->keycodes[0x01F] = PTSD_KEY_S;
    keyboard->keycodes[0x014] = PTSD_KEY_T;
    keyboard->keycodes[0x016] = PTSD_KEY_U;
    keyboard->keycodes[0x02F] = PTSD_KEY_V;
    keyboard->keycodes[0x011] = PTSD_KEY_W;
    keyboard->keycodes[0x02D] = PTSD_KEY_X;
    keyboard->keycodes[0x015] = PTSD_KEY_Y;
    keyboard->keycodes[0x02C] = PTSD_KEY_Z;

    keyboard->keycodes[0x028] = PTSD_KEY_APOSTROPHE;
    keyboard->keycodes[0x02B] = PTSD_KEY_BACKSLASH;
    keyboard->keycodes[0x033] = PTSD_KEY_COMMA;
    keyboard->keycodes[0x00D] = PTSD_KEY_EQUAL;
    keyboard->keycodes[0x029] = PTSD_KEY_GRAVE_ACCENT;
    keyboard->keycodes[0x01A] = PTSD_KEY_LEFT_BRACKET;
    keyboard->keycodes[0x00C] = PTSD_KEY_MINUS;
    keyboard->keycodes[0x034] = PTSD_KEY_PERIOD;
    keyboard->keycodes[0x01B] = PTSD_KEY_RIGHT_BRACKET;
    keyboard->keycodes[0x027] = PTSD_KEY_SEMICOLON;
    keyboard->keycodes[0x035] = PTSD_KEY_SLASH;
    keyboard->keycodes[0x056] = PTSD_KEY_WORLD_2;

    keyboard->keycodes[0x00E] = PTSD_KEY_BACKSPACE;
    keyboard->keycodes[0x153] = PTSD_KEY_DELETE;
    keyboard->keycodes[0x14F] = PTSD_KEY_END;
    keyboard->keycodes[0x01C] = PTSD_KEY_ENTER;
    keyboard->keycodes[0x001] = PTSD_KEY_ESCAPE;
    keyboard->keycodes[0x147] = PTSD_KEY_HOME;
    keyboard->keycodes[0x152] = PTSD_KEY_INSERT;
    keyboard->keycodes[0x15D] = PTSD_KEY_MENU;
    keyboard->keycodes[0x151] = PTSD_KEY_PAGE_DOWN;
    keyboard->keycodes[0x149] = PTSD_KEY_PAGE_UP;
    keyboard->keycodes[0x045] = PTSD_KEY_PAUSE;
    keyboard->keycodes[0x146] = PTSD_KEY_PAUSE;
    keyboard->keycodes[0x039] = PTSD_KEY_SPACE;
    keyboard->keycodes[0x00F] = PTSD_KEY_TAB;
    keyboard->keycodes[0x03A] = PTSD_KEY_CAPS_LOCK;
    keyboard->keycodes[0x145] = PTSD_KEY_NUM_LOCK;
    keyboard->keycodes[0x046] = PTSD_KEY_SCROLL_LOCK;
    keyboard->keycodes[0x03B] = PTSD_KEY_F1;
    keyboard->keycodes[0x03C] = PTSD_KEY_F2;
    keyboard->keycodes[0x03D] = PTSD_KEY_F3;
    keyboard->keycodes[0x03E] = PTSD_KEY_F4;
    keyboard->keycodes[0x03F] = PTSD_KEY_F5;
    keyboard->keycodes[0x040] = PTSD_KEY_F6;
    keyboard->keycodes[0x041] = PTSD_KEY_F7;
    keyboard->keycodes[0x042] = PTSD_KEY_F8;
    keyboard->keycodes[0x043] = PTSD_KEY_F9;
    keyboard->keycodes[0x044] = PTSD_KEY_F10;
    keyboard->keycodes[0x057] = PTSD_KEY_F11;
    keyboard->keycodes[0x058] = PTSD_KEY_F12;
    keyboard->keycodes[0x064] = PTSD_KEY_F13;
    keyboard->keycodes[0x065] = PTSD_KEY_F14;
    keyboard->keycodes[0x066] = PTSD_KEY_F15;
    keyboard->keycodes[0x067] = PTSD_KEY_F16;
    keyboard->keycodes[0x068] = PTSD_KEY_F17;
    keyboard->keycodes[0x069] = PTSD_KEY_F18;
    keyboard->keycodes[0x06A] = PTSD_KEY_F19;
    keyboard->keycodes[0x06B] = PTSD_KEY_F20;
    keyboard->keycodes[0x06C] = PTSD_KEY_F21;
    keyboard->keycodes[0x06D] = PTSD_KEY_F22;
    keyboard->keycodes[0x06E] = PTSD_KEY_F23;
    keyboard->keycodes[0x076] = PTSD_KEY_F24;
    keyboard->keycodes[0x038] = PTSD_KEY_LEFT_ALT;
    keyboard->keycodes[0x01D] = PTSD_KEY_LEFT_CONTROL;
    keyboard->keycodes[0x02A] = PTSD_KEY_LEFT_SHIFT;
    keyboard->keycodes[0x15B] = PTSD_KEY_LEFT_SUPER;
    keyboard->keycodes[0x137] = PTSD_KEY_PRINT_SCREEN;
    keyboard->keycodes[0x138] = PTSD_KEY_RIGHT_ALT;
    keyboard->keycodes[0x11D] = PTSD_KEY_RIGHT_CONTROL;
    keyboard->keycodes[0x036] = PTSD_KEY_RIGHT_SHIFT;
    keyboard->keycodes[0x15C] = PTSD_KEY_RIGHT_SUPER;
    keyboard->keycodes[0x150] = PTSD_KEY_DOWN;
    keyboard->keycodes[0x14B] = PTSD_KEY_LEFT;
    keyboard->keycodes[0x14D] = PTSD_KEY_RIGHT;
    keyboard->keycodes[0x148] = PTSD_KEY_UP;

    keyboard->keycodes[0x052] = PTSD_KEY_KP_0;
    keyboard->keycodes[0x04F] = PTSD_KEY_KP_1;
    keyboard->keycodes[0x050] = PTSD_KEY_KP_2;
    keyboard->keycodes[0x051] = PTSD_KEY_KP_3;
    keyboard->keycodes[0x04B] = PTSD_KEY_KP_4;
    keyboard->keycodes[0x04C] = PTSD_KEY_KP_5;
    keyboard->keycodes[0x04D] = PTSD_KEY_KP_6;
    keyboard->keycodes[0x047] = PTSD_KEY_KP_7;
    keyboard->keycodes[0x048] = PTSD_KEY_KP_8;
    keyboard->keycodes[0x049] = PTSD_KEY_KP_9;
    keyboard->keycodes[0x04E] = PTSD_KEY_KP_ADD;
    keyboard->keycodes[0x053] = PTSD_KEY_KP_DECIMAL;
    keyboard->keycodes[0x135] = PTSD_KEY_KP_DIVIDE;
    keyboard->keycodes[0x11C] = PTSD_KEY_KP_ENTER;
    keyboard->keycodes[0x059] = PTSD_KEY_KP_EQUAL;
    keyboard->keycodes[0x037] = PTSD_KEY_KP_MULTIPLY;
    keyboard->keycodes[0x04A] = PTSD_KEY_KP_SUBTRACT;

    for (u32 scancode = 0; scancode < 512; scancode++) {
        if (keyboard->keycodes[scancode] != -1)
            keyboard->scancodes[keyboard->keycodes[scancode]] = scancode;
    }
}

pmodifier_t pkb_get_modifier(pkeyboard_t *const) {
    pmodifier_t mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= PTSD_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= PTSD_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= PTSD_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= PTSD_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= PTSD_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
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

pdevice_proc_result_t pkeyboard_device_proc(pdevice_t *const device, 
        const pdevice_parameter_pack_t *const parm)
{
    pkeyboard_t *const keyboard = (void* const)device;
    switch(parm->msg) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
            int repeats  = LOWORD(parm->lparam);
            pkey_action_state_t action   = (HIWORD(parm->lparam) & KF_UP) ? PKB_RELEASED : PKB_PRESSED;
            if (action && repeats > 1) action = PKB_HELD;

            unsigned int scancode = (HIWORD(parm->lparam) & (KF_EXTENDED | 0xff));
            if (!scancode) {
                scancode = MapVirtualKeyW((UINT) parm->wparam, MAPVK_VK_TO_VSC);
            }
            pkeycode_t key = keyboard->keycodes[scancode];
            keyboard->last_key = key;
            pkb_update_keystate(keyboard, key, action);
        }
        return (pdevice_proc_result_t){.handled=true, DefWindowProc((HWND)parm->window->handle, parm->msg, parm->wparam, parm->lparam)};
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

pmodifier_t pkb_get_modifiers(const pkeyboard_t *const) {
    pmodifier_t mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= PTSD_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= PTSD_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= PTSD_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= PTSD_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= PTSD_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= PTSD_MOD_NUM_LOCK;
    return mods;
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
