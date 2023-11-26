#ifndef PTSD_WINDOW_HEADER
#define PTSD_WINDOW_HEADER
#include "general.h"
#include "pplatform.h"

#if defined(PTSD_WINDOW_INCLUDE_VULKAN)
#include <vulkan.h>
#elif defined(PTSD_WINDOW_INCLUDE_GL)
#include <GL/GL.h>
#elif defined(PTSD_WINDOW_INCLUDE_GLAD)
#include <glad/glad.h>
#elif defined(PTSD_WINDOW_INCLUDE_GLAD_GL)
#include <glad/gl.h>
#elif defined(PTSD_WINDOW_INCLUDE_GLAD_VK)
#include <glad/vulkan.h>
#endif

// for setting the window location to 0
// this is needed because setting window_info.x = 0 would be the default location
// this default location may or may not be equal to 0, this macro guarantees it
#define PTSD_ZERO       ((u32)0x80000000)

#ifdef PTSD_WINDOWS
#define phit_device(fn)                                     \
   ((phit_device_t){                                        \
        .device = {                                         \
            .init          = pwindow_hit_device_init,       \
            .shutdown      = pwindow_hit_device_shutdown,   \
            .update        = pwindow_hit_device_update,     \
            .state         = pwindow_hit_state,             \
            .wnd_proc      = pwindow_hit_device_wnd_proc,   \
        },                                                  \
        .event_handler = (fn)                               \
    })
#endif

#define pkeyboard()                             \
   ((pkeyboard_t){                              \
        .device = {                             \
            .init          = pkeyboard_init,    \
            .shutdown      = pkeyboard_shutdown,\
            .update        = pkeyboard_update,  \
            .state         = pkeyboard_state,   \
            .wnd_proc      = pkeyboard_wnd_proc,\
        }                                       \
    })

#define pmouse()                                \
   ((pmouse_t){                                 \
        .device = {                             \
            .init          = pmouse_init,       \
            .shutdown      = pmouse_shutdown,   \
            .update        = pmouse_update,     \
            .state         = pmouse_state,      \
            .wnd_proc      = pmouse_wnd_proc,   \
        }                                       \
    })

#define phints(...) \
    .hint_count = countof(((pwindow_hint_t[]){__VA_ARGS__})),\
    .hints      = (pwindow_hint_t[]){__VA_ARGS__}

#define pdevices(...) \
    .device_count = countof(((pdevice_t*[]){__VA_ARGS__})),\
    .devices      = (pdevice_t*[]){__VA_ARGS__}

struct pwindow_t;

#if PTSD_32 && defined(PTSD_WINDOWS)
typedef long plong_ptr_t;
#elif defined(PTSD_WINDOWS)
typedef isize plong_ptr_t;
#endif

typedef bool pwindow_resize_event_handler_t(struct pwindow_t*, u32 width, u32 height); 
typedef bool pwindow_close_event_handler_t(struct pwindow_t*);

typedef enum pmodifier_t {
    PTSD_MOD_SHIFT     = 0b000001,
    PTSD_MOD_CONTROL   = 0b000010,
    PTSD_MOD_ALT       = 0b000100,
    PTSD_MOD_SUPER     = 0b001000,
    PTSD_MOD_CAPS_LOCK = 0b010000,
    PTSD_MOD_NUM_LOCK  = 0b100000,
} pmodifier_t;

typedef struct pstate_t pstate_t;
struct pstate_t {
    usize       code;
    pmodifier_t modifiers;
    union {
        struct { bool held, pressed, released; };
        struct { f64 x, y; };
    };
};

#ifdef PTSD_WINDOWS
typedef struct pdevice_proc_result_t pdevice_proc_result_t;
struct pdevice_proc_result_t {
    bool handled; plong_ptr_t result;
};
#endif

typedef struct pdevice_t pdevice_t;
struct pdevice_t {
    void (*init)(pdevice_t *const);
    void (*shutdown)(pdevice_t *const);
    void (*update)(pdevice_t *const);
    pstate_t (*state)(pdevice_t *const, usize);

#ifdef PTSD_WINDOWS
    pdevice_proc_result_t (*wnd_proc)(pdevice_t *const,
            struct pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#elif defined(PTSD_WASM)
#error device callback has not been defined for this platform
#elif defined(PTSD_LINUX)
#error device callback has not been defined for this platform
#elif defined(PTSD_MACOS)
#error device callback has not been defined for this platform
#endif
};

#ifdef PTSD_WINDOWS
typedef enum phit_location_t {
    PHIT_NOWHERE,
    PHIT_USERSPACE,
    PHIT_LEFT,
    PHIT_RIGHT,
    PHIT_TOP,
    PHIT_BOTTOM,
    PHIT_TOPLEFT,
    PHIT_TOPRIGHT,
    PHIT_BOTTOMLEFT,
    PHIT_BOTTOMRIGHT,
    PHIT_TITLEBAR,
    PHIT_CLOSE_BUTTON,
    PHIT_MAXIMIZE_BUTTON,
    PHIT_ICONIFY_BUTTON,
} phit_location_t;

typedef struct phit_device_t phit_device_t;
struct phit_device_t {
    pdevice_t device;
    phit_location_t (*event_handler)(struct pwindow_t*, isize x, isize y);
};
#endif

typedef enum pkeycode_t {
    PTSD_KEY_UNKNOWN      = -1,

    PTSD_KEY_SPACE         = ' ',
    PTSD_KEY_APOSTROPHE    = '\'',
    PTSD_KEY_COMMA         = ',',
    PTSD_KEY_MINUS         = '-',
    PTSD_KEY_PERIOD        = '.',
    PTSD_KEY_SLASH         = '/',
    PTSD_KEY_0             = '0',
    PTSD_KEY_1             = '1',
    PTSD_KEY_2             = '2',
    PTSD_KEY_3             = '3',
    PTSD_KEY_4             = '4',
    PTSD_KEY_5             = '5',
    PTSD_KEY_6             = '6',
    PTSD_KEY_7             = '7',
    PTSD_KEY_8             = '8',
    PTSD_KEY_9             = '9',
    PTSD_KEY_SEMICOLON     = ';',
    PTSD_KEY_EQUAL         = '=',
    PTSD_KEY_A             = 'A', /* note: 'a' also works */
    PTSD_KEY_B             = 'B', /* note: 'b' also works */
    PTSD_KEY_C             = 'C', /* note: 'c' also works */
    PTSD_KEY_D             = 'D', /* note: 'd' also works */
    PTSD_KEY_E             = 'E', /* note: 'e' also works */
    PTSD_KEY_F             = 'F', /* note: 'f' also works */
    PTSD_KEY_G             = 'G', /* note: 'g' also works */
    PTSD_KEY_H             = 'H', /* note: 'h' also works */
    PTSD_KEY_I             = 'I', /* note: 'i' also works */
    PTSD_KEY_J             = 'J', /* note: 'j' also works */
    PTSD_KEY_K             = 'K', /* note: 'k' also works */
    PTSD_KEY_L             = 'L', /* note: 'l' also works */
    PTSD_KEY_M             = 'M', /* note: 'm' also works */
    PTSD_KEY_N             = 'N', /* note: 'n' also works */
    PTSD_KEY_O             = 'O', /* note: 'o' also works */
    PTSD_KEY_P             = 'P', /* note: 'p' also works */
    PTSD_KEY_Q             = 'Q', /* note: 'q' also works */
    PTSD_KEY_R             = 'R', /* note: 'r' also works */
    PTSD_KEY_S             = 'S', /* note: 's' also works */
    PTSD_KEY_T             = 'T', /* note: 't' also works */
    PTSD_KEY_U             = 'U', /* note: 'u' also works */
    PTSD_KEY_V             = 'V', /* note: 'v' also works */
    PTSD_KEY_W             = 'W', /* note: 'w' also works */
    PTSD_KEY_X             = 'X', /* note: 'x' also works */
    PTSD_KEY_Y             = 'Y', /* note: 'y' also works */
    PTSD_KEY_Z             = 'Z', /* note: 'z' also works */
    PTSD_KEY_LEFT_BRACKET  = '[',
    PTSD_KEY_BACKSLASH     = '\\',
    PTSD_KEY_RIGHT_BRACKET = ']',
    PTSD_KEY_GRAVE_ACCENT  = '`',
    PTSD_KEY_WORLD_1       = 161, /* non-US #1 */
    PTSD_KEY_WORLD_2       = 162, /* non-US #2 */

    PTSD_KEY_ESCAPE        = 256,
    PTSD_KEY_ENTER         = 257,
    PTSD_KEY_TAB           = 258,
    PTSD_KEY_BACKSPACE     = 259,
    PTSD_KEY_INSERT        = 260,
    PTSD_KEY_DELETE        = 261,
    PTSD_KEY_RIGHT         = 262,
    PTSD_KEY_LEFT          = 263,
    PTSD_KEY_DOWN          = 264,
    PTSD_KEY_UP            = 265,
    PTSD_KEY_PAGE_UP       = 266,
    PTSD_KEY_PAGE_DOWN     = 267,
    PTSD_KEY_HOME          = 268,
    PTSD_KEY_END           = 269,
    PTSD_KEY_CAPS_LOCK     = 280,
    PTSD_KEY_SCROLL_LOCK   = 281,
    PTSD_KEY_NUM_LOCK      = 282,
    PTSD_KEY_PRINT_SCREEN  = 283,
    PTSD_KEY_PAUSE         = 284,
    PTSD_KEY_F1            = 290,
    PTSD_KEY_F2            = 291,
    PTSD_KEY_F3            = 292,
    PTSD_KEY_F4            = 293,
    PTSD_KEY_F5            = 294,
    PTSD_KEY_F6            = 295,
    PTSD_KEY_F7            = 296,
    PTSD_KEY_F8            = 297,
    PTSD_KEY_F9            = 298,
    PTSD_KEY_F10           = 299,
    PTSD_KEY_F11           = 300,
    PTSD_KEY_F12           = 301,
    PTSD_KEY_F13           = 302,
    PTSD_KEY_F14           = 303,
    PTSD_KEY_F15           = 304,
    PTSD_KEY_F16           = 305,
    PTSD_KEY_F17           = 306,
    PTSD_KEY_F18           = 307,
    PTSD_KEY_F19           = 308,
    PTSD_KEY_F20           = 309,
    PTSD_KEY_F21           = 310,
    PTSD_KEY_F22           = 311,
    PTSD_KEY_F23           = 312,
    PTSD_KEY_F24           = 313,
    PTSD_KEY_F25           = 314,
    PTSD_KEY_KP_0          = 320,
    PTSD_KEY_KP_1          = 321,
    PTSD_KEY_KP_2          = 322,
    PTSD_KEY_KP_3          = 323,
    PTSD_KEY_KP_4          = 324,
    PTSD_KEY_KP_5          = 325,
    PTSD_KEY_KP_6          = 326,
    PTSD_KEY_KP_7          = 327,
    PTSD_KEY_KP_8          = 328,
    PTSD_KEY_KP_9          = 329,
    PTSD_KEY_KP_DECIMAL    = 330,
    PTSD_KEY_KP_DIVIDE     = 331,
    PTSD_KEY_KP_MULTIPLY   = 332,
    PTSD_KEY_KP_SUBTRACT   = 333,
    PTSD_KEY_KP_ADD        = 334,
    PTSD_KEY_KP_ENTER      = 335,
    PTSD_KEY_KP_EQUAL      = 336,
    PTSD_KEY_LEFT_SHIFT    = 340,
    PTSD_KEY_LEFT_CONTROL  = 341,
    PTSD_KEY_LEFT_ALT      = 342,
    PTSD_KEY_LEFT_SUPER    = 343,
    PTSD_KEY_RIGHT_SHIFT   = 344,
    PTSD_KEY_RIGHT_CONTROL = 345,
    PTSD_KEY_RIGHT_ALT     = 346,
    PTSD_KEY_RIGHT_SUPER   = 347,
    PTSD_KEY_MENU          = 348,
    PTSD_KEY_COUNT         = 512,
} pkeycode_t;

typedef struct pkeyboard_t pkeyboard_t;
struct pkeyboard_t {
    pdevice_t  device;
    pkeycode_t keycodes[PTSD_KEY_COUNT];
    usize      scancodes[PTSD_KEY_COUNT];
    pstate_t   keys[PTSD_KEY_COUNT];
};

typedef enum pmouse_button_t {
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

    PTSD_MOUSE_LEFT   = PTSD_MOUSE_BUTTON_0,
    PTSD_MOUSE_MIDDLE = PTSD_MOUSE_BUTTON_1,
    PTSD_MOUSE_RIGHT  = PTSD_MOUSE_BUTTON_2,
} pmouse_button_t;

typedef struct pmouse_t pmouse_t;
struct pmouse_t {
    pdevice_t device;
    pstate_t inputs[PTSD_MOUSE_INPUT_COUNT];
};
typedef enum pcontext_kind_t {
    PDEFAULT,
    PNO_CONTEXT,
    PSOFTWARE,
    POPENGL,
    PVULKAN,
#ifdef PTSD_WINDOWS
    PDIRECTX,
#endif
#ifdef PTSD_MACOS
    PMETAL,
#endif
} pcontext_kind_t;

typedef union pcontext_t pcontext_t;
union pcontext_t {
    //TODO: when these have been formalised
    // i want to write a transparent struct in here
    // so that if someone doesn't care about the internals
    // they can just call the functions
    // like say we have a clearscreen function
    // then someone could just do
    // win->context->clear(win->context);
    // instead of doing something like
    // switch (pget_context_kind(win)) {
    // case POPENGL: pget_context(win)->gl.clear(pget_context(win));
    // case PVULKAN: pget_context(win)->vk.clear(pget_context(win));
    // ...
    // }
    const struct pgl_context_t *gl;
    const struct pvk_context_t *vk;
    const struct psw_context_t *sw;
    const struct pgl_context_t *opengl;
    const struct pvk_context_t *vulkan;
    const struct psw_context_t *software;
#ifdef PTSD_WINDOWS
    const struct pdx_context_t *dx;
    const struct pdx_context_t *directx;
#endif
#ifdef PTSD_MACOS
    const struct pmt_context_t *mt;
    const struct pmt_context_t *metal;
#endif
};

typedef struct pwindow_t pwindow_t;
struct pwindow_t {
    u32 width, height;
    u32 x, y;
    volatile bool is_running;
    pcontext_t context;
    usize device_count;
    pdevice_t **devices;
    phandle_t *handle;
};

typedef enum pwindow_hint_t {
    PHINT_NONE,
    PHINT_FULLSCREEN,
    // this will setup a window that is resizable but has no titlebar
    PHINT_UNDECORATED,
    PHINT_NO_RESIZE,
} pwindow_hint_t;

typedef struct pwindow_info_t pwindow_info_t;
struct pwindow_info_t {
    usize           hint_count;
    pwindow_hint_t *hints;
    usize           device_count;
    pdevice_t     **devices;
    char           *title;
    u32             width, height;
    u32             x, y;
    pwindow_t      *parent;
    /* pcontext_info_t *context*/
};

#ifdef PTSD_WINDOWS
void                  pwindow_hit_device_init(pdevice_t *const);
void                  pwindow_hit_device_shutdown(pdevice_t *const);
pstate_t              pwindow_hit_device_update(pdevice_t *const);
pdevice_proc_result_t pwindow_hit_device_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#endif

void                  pkeyboard_init(pdevice_t *const);
void                  pkeyboard_shutdown(pdevice_t *const);
void                  pkeyboard_update(pdevice_t *const);
pstate_t              pkeyboard_state(pdevice_t *const, usize value);
#ifdef PTSD_WINDOWS
pdevice_proc_result_t pkeyboard_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#endif

void                  pmouse_init(pdevice_t *const);
void                  pmouse_shutdown(pdevice_t *const);
void                  pmouse_update(pdevice_t *const);
pstate_t              pmouse_state(pdevice_t *const, usize value);
#ifdef PTSD_WINDOWS
pdevice_proc_result_t pmouse_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#endif

pmodifier_t pmodifier(void);

PTSD_UNUSED
static pstate_t pstate(pdevice_t *device, usize value) { return device->state(device, value); }
#define pstate(d, value) \
    pstate(\
        _Generic((d), \
        pdevice_t*:  (d),\
        default:   &(d)->device\
        ), (value))

const pwindow_t *pwindow(const pwindow_info_t *const);

void ppoll_events(const pwindow_t *const);
void pswap_buffers(const pwindow_t *const);
bool prequest_close(const pwindow_t *const);
void pfree_window(const pwindow_t *const);
#endif // PTSD_WINDOW_HEADER
