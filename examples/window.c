#include "pwindow.h"
#include "pio.h"

int main(void) {
    pkeyboard_t keyboard = pkeyboard();
    pmouse_t mouse = pmouse();

    const pwindow_t *win = pwindow(&(pwindow_info_t){
        pdevices((void*)&keyboard, (void*)&mouse),
        phints(PHINT_UNDECORATED, PHINT_FULLSCREEN),
        .width = 1024,
    });

    while (win->is_running) {
        ppoll_events(win);

        pprintf("\x1b[s");
        pprintf("(%f, %f)\n", mouse.inputs[PTSD_MOUSE_XY].x, mouse.inputs[PTSD_MOUSE_XY].y);
        pprintf("W: (pressed: %B, released: %B, held: %B)                 \n", keyboard.keys['W'].pressed, keyboard.keys['W'].released, keyboard.keys['W'].held);
        pprintf("A: (pressed: %B, released: %B, held: %B)                 \n", keyboard.keys['A'].pressed, keyboard.keys['A'].released, keyboard.keys['A'].held);
        pprintf("S: (pressed: %B, released: %B, held: %B)                 \n", keyboard.keys['S'].pressed, keyboard.keys['S'].released, keyboard.keys['S'].held);
        pprintf("D: (pressed: %B, released: %B, held: %B)                 \n", keyboard.keys['D'].pressed, keyboard.keys['D'].released, keyboard.keys['D'].held);
        pprintf("\x1b[u");

        pswap_buffers(win);
    }
    pfree_window(win);
}


