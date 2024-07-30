#include "pwindow.h"
#include "pkeyboard.h"
#include "pmouse.h"
#include "pio.h"


int main(void) {
    pkeyboard_t keyboard = pkeyboard();
    pmouse_t mouse = pmouse();

    const pwindow_t *win = pwindow(&(pwindow_info_t) {
        pdevices((void*)&keyboard, (void*)&mouse),
        .width = 1024,
    });

    static constexpr pstring_t states[] = {
        pcreate_const_string("RELEASED"),
        pcreate_const_string("PRESSED"),
        pcreate_const_string("HELD"),
    };

    while (win->is_running) {
        ppoll_events(win);

        pprintf("\x1b[s");
        pmouse_state_t pos = pmo_get_position(&mouse);
        pprintf("(%f, %f)\n", pos.x, pos.y);
        pkey_state_t w = pkb_get_key(&keyboard, 'w'); pprintf("W: (pressed: %B, released: %B, held: %B, raw: %S)                     \n", w.pressed, w.released, w.held, states[w.action]);
        pkey_state_t a = pkb_get_key(&keyboard, 'a'); pprintf("A: (pressed: %B, released: %B, held: %B, raw: %S)                     \n", a.pressed, a.released, a.held, states[a.action]);
        pkey_state_t s = pkb_get_key(&keyboard, 's'); pprintf("S: (pressed: %B, released: %B, held: %B, raw: %S)                     \n", s.pressed, s.released, s.held, states[s.action]);
        pkey_state_t d = pkb_get_key(&keyboard, 'd'); pprintf("D: (pressed: %B, released: %B, held: %B, raw: %S)                     \n", d.pressed, d.released, d.held, states[d.action]);

        pmouse_state_t left = pmo_get_input(&mouse, PTSD_MOUSE_LEFT); pprintf("mouse left: (pressed: %B, released: %B, held: %B, raw: %S)                     \n",
                left.pressed, left.released, left.held, states[left.action]);

        if (pmo_any_button_pressed(&mouse)) {
            pprintf("last mouse button id: %u   \n", mouse.last_button);
        }
        else pputc(0);
        if (pkb_any_key_pressed(&keyboard)) {
            pprintf("last key id: %u   \n", keyboard.last_key);
        }
        else pputc(0);

        pprintf("\x1b[u");

        pswap_buffers(win);
    }
    pfree_window(win);
}


