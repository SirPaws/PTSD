#ifndef PTSD_MOUSE_HEADER
#define PTSD_MOUSE_HEADER

#include "general.h"
#include "pplatform.h"
#include "pwindow.h"

enum pmouse_input_t PTSD_IN_VERSION(PTSD_C23, : s16){
    PTSD_MOUSE_BUTTON_UNKNOWN = -1,
    PTSD_MOUSE_BUTTON_0,
    PTSD_MOUSE_BUTTON_1,
    PTSD_MOUSE_BUTTON_2,
    PTSD_MOUSE_BUTTON_3,
    PTSD_MOUSE_BUTTON_4,
    PTSD_MOUSE_BUTTON_5,
    PTSD_MOUSE_BUTTON_6,
    PTSD_MOUSE_BUTTON_7,
    PTSD_MOUSE_BUTTON_8,
    PTSD_MOUSE_BUTTON_9,
    PTSD_MOUSE_BUTTON_10,
    PTSD_MOUSE_BUTTON_11,
    PTSD_MOUSE_BUTTON_12,
    PTSD_MOUSE_BUTTON_13,
    PTSD_MOUSE_BUTTON_14,
    PTSD_MOUSE_BUTTON_15,
    PTSD_MOUSE_BUTTON_16,
    PTSD_MOUSE_BUTTON_17,
    PTSD_MOUSE_BUTTON_18,
    PTSD_MOUSE_BUTTON_19,
    PTSD_MOUSE_BUTTON_20,
    PTSD_MOUSE_BUTTON_21,
    PTSD_MOUSE_BUTTON_22,
    PTSD_MOUSE_BUTTON_23,
    PTSD_MOUSE_BUTTON_24,
    PTSD_MOUSE_BUTTON_25,
    PTSD_MOUSE_BUTTON_26,
    PTSD_MOUSE_BUTTON_27,
    PTSD_MOUSE_BUTTON_28,
    PTSD_MOUSE_BUTTON_29,
    PTSD_MOUSE_BUTTON_30,
    PTSD_MOUSE_BUTTON_31,

    PTSD_MOUSE_XY,
    PTSD_MOUSE_WHEEL_XY,

    PTSD_MOUSE_INPUT_COUNT,
    PTSD_MOUSE_BUTTON_COUNT = PTSD_MOUSE_XY,

    PTSD_MOUSE_BUTTON_NONE,

    PTSD_MOUSE_LEFT   = PTSD_MOUSE_BUTTON_0,
    PTSD_MOUSE_MIDDLE = PTSD_MOUSE_BUTTON_1,
    PTSD_MOUSE_RIGHT  = PTSD_MOUSE_BUTTON_2,
};
PTSD_BEFORE_VERSION(PTSD_C23, typedef s16 pmouse_button_t);
PTSD_IN_VERSION(    PTSD_C23, typedef enum pmouse_input_t pmouse_input_t);

typedef pmouse_input_t pmouse_button_t;

typedef struct pmouse_state_t pmouse_state_t;
struct pmouse_state_t {
    pmouse_input_t code;
    union {
        struct {
            enum pmouse_button_action_t PTSD_IN_VERSION(PTSD_C23, : u8) {
                PTSD_MOUSE_RELEASED, PTSD_MOUSE_PRESSED, PTSD_MOUSE_HELD
            } action;
            bool held, pressed, released;
        };
        struct { f64 x, y; };
        struct pmouse_button_state_t {
            enum pmouse_button_action_t action;
            bool held, pressed, released;
        } button;
        struct pmouse_position_state_t { f64 x, y; } pos;
        struct pmouse_scroll_state_t { f64 x, y; } offset;
    };
};
PTSD_BEFORE_VERSION(PTSD_C23, typedef u8 pmouse_button_action_t);
PTSD_IN_VERSION(    PTSD_C23, typedef enum pmouse_button_action_t pmouse_button_action_t);

typedef struct pmouse_t pmouse_t;
struct pmouse_t {
    pdevice_t device;
    pmouse_state_t inputs[PTSD_MOUSE_INPUT_COUNT];
    pmouse_button_t last_button;
};

PTSD_UNUSED
static inline pmouse_t pmouse(void) {
    void                  pmouse_init(pdevice_t *const);
    void                  pmouse_shutdown(pdevice_t *const);
    void                  pmouse_update(pdevice_t *const);
    #ifdef PTSD_WINDOWS
    pdevice_proc_result_t pmouse_wnd_proc(pdevice_t *const,
            pwindow_procedure_parameter_pack_t[static const 1]);
    #endif
    return (pmouse_t){
        .device = {
            .init       = pmouse_init,
            .shutdown   = pmouse_shutdown,
            .update     = pmouse_update,
    #ifdef PTSD_WINDOWS
            .wnd_proc   = pmouse_wnd_proc,
    #endif
        },
        .last_button = PTSD_MOUSE_BUTTON_NONE
    };
}

bool         pmo_is_pressed(pmouse_t *mo, pmouse_button_t);
bool         pmo_is_released(pmouse_t *mo, pmouse_button_t);
bool         pmo_is_held(pmouse_t *mo, pmouse_button_t);
pmouse_state_t pmo_get_input(pmouse_t *mo, pmouse_button_t);
pmouse_state_t pmo_get_last_button(pmouse_t *mo);
bool           pmo_any_button_pressed(pmouse_t *mo);

pmouse_state_t pmo_get_position(pmouse_t *mo);
pmouse_state_t pmo_get_scroll(pmouse_t *mo);

#endif // PTSD_MOUSE_HEADER
