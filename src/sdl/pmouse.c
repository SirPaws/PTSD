#include "pwindow.h"
#include "plimits.h"
#include "pstacktrace.h"
#include "SDL.h"
#include "pmouse.h"

void pmouse_init(pdevice_t *const device) {
    pmouse_t *mouse = (void*)device;
    memset(mouse->inputs, 0, sizeof mouse->inputs);
    for (int i = 0; i < PTSD_MOUSE_INPUT_COUNT; i++) {
        mouse->inputs[i].code = i;
    }
    mouse->last_button = PTSD_MOUSE_BUTTON_UNKNOWN;
}
void pmouse_shutdown(pdevice_t *const) {}

void pmouse_update(pdevice_t *const device) {
    static pmouse_state_t oldmousestate[PTSD_MOUSE_BUTTON_COUNT] = {0};
    pmouse_t *mouse = (void*)device;
    
    pmouse_state_t *mousestate = mouse->inputs;
    for (int i = 0; i < PTSD_MOUSE_BUTTON_COUNT; i++) {
        if (mousestate[i].pressed){
            if (oldmousestate[i].pressed) {
                oldmousestate[i] = mousestate[i];
                mousestate[i].released = false;
                mousestate[i].pressed  = true;
                mousestate[i].held     = true;
            }
            else {
                oldmousestate[i] = mousestate[i];
                mousestate[i].released = false;
                mousestate[i].pressed  = false;
                mousestate[i].held     = true;
            }
        }
        else if (mousestate[i].released) {
            if (oldmousestate[i].released) {
                oldmousestate[i] = mousestate[i];
                mousestate[i].released = true;
                mousestate[i].pressed  = false;
                mousestate[i].held     = false;
            }
            else {
                oldmousestate[i] = mousestate[i];
                mousestate[i].released = false;
                mousestate[i].pressed  = false;
                mousestate[i].held     = false;
            }
        }
        else if (mousestate[i].held) {
            oldmousestate[i] = mousestate[i];
            mousestate[i].released = false;
            mousestate[i].pressed  = false;
        }
    }
}

void pmo_update_buttonstate(pmouse_t *const mouse, pmouse_button_t buttoncode, pmouse_button_action_t newstate) {
    if (buttoncode < 0 || buttoncode >= PTSD_MOUSE_BUTTON_COUNT) return;
    pmouse_state_t *button = &mouse->inputs[buttoncode];

    int combined_states = (button->action << 2) | newstate;
    switch (combined_states) {
    case 0b0000: // RELEASED | RELEASED
        button->action    = PTSD_MOUSE_RELEASED;
        break;
    case 0b0001: // RELEASED | PRESSED
        button->action    = PTSD_MOUSE_PRESSED;
        button->held      = false;
        button->pressed   = true;
        button->released  = false;
        break;
    case 0b0010: // RELEASED | HELD
        button->action    = PTSD_MOUSE_HELD;
        break;

    case 0b0100: // PRESSED | RELEASED
        button->action    = PTSD_MOUSE_RELEASED;
        button->held      = false;
        button->pressed   = false;
        button->released  = true;
        break;
    case 0b0101: // PRESSED | PRESSED
        button->action    = PTSD_MOUSE_HELD;
        break;
    case 0b0110: // PRESSED | HELD
        button->action    = PTSD_MOUSE_HELD;
        break;
    
    case 0b1000: // HELD    | RELEASED
        button->action    = PTSD_MOUSE_RELEASED;
        break;
    case 0b1001: // HELD    | PRESSED
        button->action    = PTSD_MOUSE_HELD;
        break;
    case 0b1010: // HELD    | HELD
        button->action    = PTSD_MOUSE_HELD;
        break;
    }
}



bool pmouse_device_proc(pdevice_t *const device, const SDL_Event *const event) {
    pmouse_t *mouse = (void*)device;

    switch (event->type) {
    case SDL_MOUSEMOTION: {
            mouse->inputs[PTSD_MOUSE_XY].x = event->motion.x;
            mouse->inputs[PTSD_MOUSE_XY].y = event->motion.y;
            return true;
        }
    case SDL_MOUSEWHEEL:{
            mouse->inputs[PTSD_MOUSE_WHEEL_XY].x = event->wheel.preciseX;
            mouse->inputs[PTSD_MOUSE_WHEEL_XY].y = event->wheel.preciseY;
            mouse->inputs[PTSD_MOUSE_XY].x = event->wheel.mouseX;
            mouse->inputs[PTSD_MOUSE_XY].y = event->wheel.mouseY;
            return true;
        }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
            int button = event->button.button;
            if (button < 1 || button > PTSD_MOUSE_BUTTON_COUNT)
                break;
            button -= 1; // SDL starts mouse buttons at one, while ours start at 0

            int state;
            if (event->button.state == SDL_PRESSED)
                 state = PTSD_MOUSE_PRESSED;
            else state = PTSD_MOUSE_RELEASED;

            mouse->last_button = button;
            pmo_update_buttonstate(mouse, button, state);
            mouse->inputs[PTSD_MOUSE_XY].x = event->button.x;
            mouse->inputs[PTSD_MOUSE_XY].y = event->button.y;
            return true;
        }
    }

    return false;
}

bool pmo_is_pressed(pmouse_t *mo, pmouse_button_t button) {
    if (button < 0 || button >= PTSD_MOUSE_BUTTON_COUNT) return false;
    return mo->inputs[button].pressed;
}

bool pmo_is_released(pmouse_t *mo, pmouse_button_t button) {
    if (button < 0 || button >= PTSD_MOUSE_BUTTON_COUNT) return false;
    return mo->inputs[button].released;
}

bool pmo_is_held(pmouse_t *mo, pmouse_button_t button) {
    if (button < 0 || button >= PTSD_MOUSE_BUTTON_COUNT) return false;
    return mo->inputs[button].held;
}

pmouse_state_t pmo_get_input(pmouse_t *mo, pmouse_input_t input) {
    if (input < 0 || input >= PTSD_MOUSE_INPUT_COUNT) return (pmouse_state_t){.code = PTSD_MOUSE_BUTTON_UNKNOWN};
    return mo->inputs[input];
}
pmouse_state_t pmo_get_last_button(pmouse_t *mo) {
    return pmo_get_input(mo, mo->last_button);
}
bool pmo_any_button_pressed(pmouse_t *mo) {
    if (mo->last_button == PTSD_MOUSE_BUTTON_UNKNOWN || mo->last_button == PTSD_MOUSE_BUTTON_NONE) return false;
    pmouse_state_t key = pmo_get_last_button(mo);
    if (key.pressed || key.held || key.action != PTSD_MOUSE_RELEASED)
         return true;
    else return false;
}

pmouse_state_t pmo_get_position(pmouse_t *mo) {
    return mo->inputs[PTSD_MOUSE_XY];
}
pmouse_state_t pmo_get_scroll(pmouse_t *mo) {
    return mo->inputs[PTSD_MOUSE_WHEEL_XY];
}


