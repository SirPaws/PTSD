#ifndef PSTD_WINDOW_HEADER
#define PSTD_WINDOW_HEADER
#include "general.h"
#include "pplatform.h"

#if defined(PSTD_WINDOW_INCLUDE_VULKAN)
#include <vulkan.h>
#elif defined(PSTD_WINDOW_INCLUDE_GL)
#include <GL/GL.h>
#elif defined(PSTD_WINDOW_INCLUDE_GLAD)
#include <glad/glad.h>
#elif defined(PSTD_WINDOW_INCLUDE_GLAD_GL)
#include <glad/gl.h>
#elif defined(PSTD_WINDOW_INCLUDE_GLAD_VK)
#include <glad/vulkan.h>
#endif

// for setting the window location to 0
// this is needed because setting window_info.x = 0 would be the default location
// this default location may or may not be equal to 0, this macro guarantees it
#define PSTD_ZERO       ((u32)0x80000000)

#ifdef PSTD_WINDOWS
#define phit_device(fn)                                     \
   ((phit_device_t){                                        \
        .device = {                                         \
            .init          = pwindow_hit_device_init,       \
            .shutdown      = pwindow_hit_device_shutdown,   \
            .update        = pwindow_hit_device_update,     \
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
            .wnd_proc      = pkeyboard_wnd_proc,\
        }                                       \
    })

#define pmouse()                                \
   ((pmouse_t){                                 \
        .device = {                             \
            .init          = pmouse_init,       \
            .shutdown      = pmouse_shutdown,   \
            .update        = pmouse_update,     \
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

#if PSTD_32 && defined(PSTD_WINDOWS)
typedef long plong_ptr_t;
#elif defined(PSTD_WINDOWS)
typedef isize plong_ptr_t;
#endif

typedef bool pwindow_resize_event_handler_t(struct pwindow_t*, u32 width, u32 height); 
typedef bool pwindow_close_event_handler_t(struct pwindow_t*);

typedef enum pmodifier_t {
    PSTD_MOD_SHIFT     = 0b000001,
    PSTD_MOD_CONTROL   = 0b000010,
    PSTD_MOD_ALT       = 0b000100,
    PSTD_MOD_SUPER     = 0b001000,
    PSTD_MOD_CAPS_LOCK = 0b010000,
    PSTD_MOD_NUM_LOCK  = 0b100000,
} pmodifier_t;

typedef struct pstate_t pstate_t;
struct pstate_t {
    usize       code;
    pmodifier_t modifiers;
    union {
        struct { pbool_t held, pressed, released; };
        struct { f64 x, y; };
    };
};

#ifdef PSTD_WINDOWS
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

#ifdef PSTD_WINDOWS
    pdevice_proc_result_t (*wnd_proc)(pdevice_t *const,
            struct pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#elif defined(PSTD_WASM)
#error device callback has not been defined for this platform
#elif defined(PSTD_LINUX)
#error device callback has not been defined for this platform
#elif defined(PSTD_MACOS)
#error device callback has not been defined for this platform
#endif
};

#ifdef PSTD_WINDOWS
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
    PSTD_KEY_UNKNOWN      = -1,

    PSTD_KEY_SPACE         = ' ',
    PSTD_KEY_APOSTROPHE    = '\'',
    PSTD_KEY_COMMA         = ',',
    PSTD_KEY_MINUS         = '-',
    PSTD_KEY_PERIOD        = '.',
    PSTD_KEY_SLASH         = '/',
    PSTD_KEY_0             = '0',
    PSTD_KEY_1             = '1',
    PSTD_KEY_2             = '2',
    PSTD_KEY_3             = '3',
    PSTD_KEY_4             = '4',
    PSTD_KEY_5             = '5',
    PSTD_KEY_6             = '6',
    PSTD_KEY_7             = '7',
    PSTD_KEY_8             = '8',
    PSTD_KEY_9             = '9',
    PSTD_KEY_SEMICOLON     = ';',
    PSTD_KEY_EQUAL         = '=',
    PSTD_KEY_A             = 'A', /* note: 'a' also works */
    PSTD_KEY_B             = 'B', /* note: 'b' also works */
    PSTD_KEY_C             = 'C', /* note: 'c' also works */
    PSTD_KEY_D             = 'D', /* note: 'd' also works */
    PSTD_KEY_E             = 'E', /* note: 'e' also works */
    PSTD_KEY_F             = 'F', /* note: 'f' also works */
    PSTD_KEY_G             = 'G', /* note: 'g' also works */
    PSTD_KEY_H             = 'H', /* note: 'h' also works */
    PSTD_KEY_I             = 'I', /* note: 'i' also works */
    PSTD_KEY_J             = 'J', /* note: 'j' also works */
    PSTD_KEY_K             = 'K', /* note: 'k' also works */
    PSTD_KEY_L             = 'L', /* note: 'l' also works */
    PSTD_KEY_M             = 'M', /* note: 'm' also works */
    PSTD_KEY_N             = 'N', /* note: 'n' also works */
    PSTD_KEY_O             = 'O', /* note: 'o' also works */
    PSTD_KEY_P             = 'P', /* note: 'p' also works */
    PSTD_KEY_Q             = 'Q', /* note: 'q' also works */
    PSTD_KEY_R             = 'R', /* note: 'r' also works */
    PSTD_KEY_S             = 'S', /* note: 's' also works */
    PSTD_KEY_T             = 'T', /* note: 't' also works */
    PSTD_KEY_U             = 'U', /* note: 'u' also works */
    PSTD_KEY_V             = 'V', /* note: 'v' also works */
    PSTD_KEY_W             = 'W', /* note: 'w' also works */
    PSTD_KEY_X             = 'X', /* note: 'x' also works */
    PSTD_KEY_Y             = 'Y', /* note: 'y' also works */
    PSTD_KEY_Z             = 'Z', /* note: 'z' also works */
    PSTD_KEY_LEFT_BRACKET  = '[',
    PSTD_KEY_BACKSLASH     = '\\',
    PSTD_KEY_RIGHT_BRACKET = ']',
    PSTD_KEY_GRAVE_ACCENT  = '`',
    PSTD_KEY_WORLD_1       = 161, /* non-US #1 */
    PSTD_KEY_WORLD_2       = 162, /* non-US #2 */

    PSTD_KEY_ESCAPE        = 256,
    PSTD_KEY_ENTER         = 257,
    PSTD_KEY_TAB           = 258,
    PSTD_KEY_BACKSPACE     = 259,
    PSTD_KEY_INSERT        = 260,
    PSTD_KEY_DELETE        = 261,
    PSTD_KEY_RIGHT         = 262,
    PSTD_KEY_LEFT          = 263,
    PSTD_KEY_DOWN          = 264,
    PSTD_KEY_UP            = 265,
    PSTD_KEY_PAGE_UP       = 266,
    PSTD_KEY_PAGE_DOWN     = 267,
    PSTD_KEY_HOME          = 268,
    PSTD_KEY_END           = 269,
    PSTD_KEY_CAPS_LOCK     = 280,
    PSTD_KEY_SCROLL_LOCK   = 281,
    PSTD_KEY_NUM_LOCK      = 282,
    PSTD_KEY_PRINT_SCREEN  = 283,
    PSTD_KEY_PAUSE         = 284,
    PSTD_KEY_F1            = 290,
    PSTD_KEY_F2            = 291,
    PSTD_KEY_F3            = 292,
    PSTD_KEY_F4            = 293,
    PSTD_KEY_F5            = 294,
    PSTD_KEY_F6            = 295,
    PSTD_KEY_F7            = 296,
    PSTD_KEY_F8            = 297,
    PSTD_KEY_F9            = 298,
    PSTD_KEY_F10           = 299,
    PSTD_KEY_F11           = 300,
    PSTD_KEY_F12           = 301,
    PSTD_KEY_F13           = 302,
    PSTD_KEY_F14           = 303,
    PSTD_KEY_F15           = 304,
    PSTD_KEY_F16           = 305,
    PSTD_KEY_F17           = 306,
    PSTD_KEY_F18           = 307,
    PSTD_KEY_F19           = 308,
    PSTD_KEY_F20           = 309,
    PSTD_KEY_F21           = 310,
    PSTD_KEY_F22           = 311,
    PSTD_KEY_F23           = 312,
    PSTD_KEY_F24           = 313,
    PSTD_KEY_F25           = 314,
    PSTD_KEY_KP_0          = 320,
    PSTD_KEY_KP_1          = 321,
    PSTD_KEY_KP_2          = 322,
    PSTD_KEY_KP_3          = 323,
    PSTD_KEY_KP_4          = 324,
    PSTD_KEY_KP_5          = 325,
    PSTD_KEY_KP_6          = 326,
    PSTD_KEY_KP_7          = 327,
    PSTD_KEY_KP_8          = 328,
    PSTD_KEY_KP_9          = 329,
    PSTD_KEY_KP_DECIMAL    = 330,
    PSTD_KEY_KP_DIVIDE     = 331,
    PSTD_KEY_KP_MULTIPLY   = 332,
    PSTD_KEY_KP_SUBTRACT   = 333,
    PSTD_KEY_KP_ADD        = 334,
    PSTD_KEY_KP_ENTER      = 335,
    PSTD_KEY_KP_EQUAL      = 336,
    PSTD_KEY_LEFT_SHIFT    = 340,
    PSTD_KEY_LEFT_CONTROL  = 341,
    PSTD_KEY_LEFT_ALT      = 342,
    PSTD_KEY_LEFT_SUPER    = 343,
    PSTD_KEY_RIGHT_SHIFT   = 344,
    PSTD_KEY_RIGHT_CONTROL = 345,
    PSTD_KEY_RIGHT_ALT     = 346,
    PSTD_KEY_RIGHT_SUPER   = 347,
    PSTD_KEY_MENU          = 348,
} pkeycode_t;

typedef struct pkeyboard_t pkeyboard_t;
struct pkeyboard_t {
    pdevice_t  device;
    pkeycode_t keycodes[512];
    usize      scancodes[512];
    pstate_t   keys[512];
};

typedef enum pmouse_button_t {
    PSTD_MOUSE_BUTTON_0,
    PSTD_MOUSE_BUTTON_1,
    PSTD_MOUSE_BUTTON_2,
    PSTD_MOUSE_BUTTON_3,
    PSTD_MOUSE_BUTTON_4,
    PSTD_MOUSE_BUTTON_5,
    PSTD_MOUSE_BUTTON_6,
    PSTD_MOUSE_BUTTON_7,
    PSTD_MOUSE_BUTTON_8,
    PSTD_MOUSE_BUTTON_9,
    PSTD_MOUSE_BUTTON_10,
    PSTD_MOUSE_BUTTON_11,
    PSTD_MOUSE_BUTTON_12,
    PSTD_MOUSE_BUTTON_13,
    PSTD_MOUSE_BUTTON_14,
    PSTD_MOUSE_BUTTON_15,
    PSTD_MOUSE_BUTTON_16,
    PSTD_MOUSE_BUTTON_17,
    PSTD_MOUSE_BUTTON_18,
    PSTD_MOUSE_BUTTON_19,
    PSTD_MOUSE_BUTTON_20,
    PSTD_MOUSE_BUTTON_21,
    PSTD_MOUSE_BUTTON_22,
    PSTD_MOUSE_BUTTON_23,
    PSTD_MOUSE_BUTTON_24,
    PSTD_MOUSE_BUTTON_25,
    PSTD_MOUSE_BUTTON_26,
    PSTD_MOUSE_BUTTON_27,
    PSTD_MOUSE_BUTTON_28,
    PSTD_MOUSE_BUTTON_29,
    PSTD_MOUSE_BUTTON_30,
    PSTD_MOUSE_BUTTON_31,

    PSTD_MOUSE_XY,
    PSTD_MOUSE_WHEEL_XY,

    PSTD_MOUSE_INPUT_COUNT,

    PSTD_MOUSE_LEFT   = PSTD_MOUSE_BUTTON_0,
    PSTD_MOUSE_MIDDLE = PSTD_MOUSE_BUTTON_1,
    PSTD_MOUSE_RIGHT  = PSTD_MOUSE_BUTTON_2,
} pmouse_button_t;

typedef struct pmouse_t pmouse_t;
struct pmouse_t {
    pdevice_t device;
    pstate_t inputs[PSTD_MOUSE_INPUT_COUNT];
};
typedef enum pcontext_kind_t {
    PDEFAULT,
    PSOFTWARE,
    POPENGL,
    PVULKAN,
#ifdef PSTD_WINDOWS
    PDIRECTX,
#endif
#ifdef PSTD_MACOS
    PMETAL,
#endif
} pcontext_kind_t;

typedef union pcontext_t pcontext_t;
union pcontext_t {
    const struct pgl_context_t *gl;
    const struct pvk_context_t *vk;
    const struct psw_context_t *sw;
    const struct pgl_context_t *opengl;
    const struct pvk_context_t *vulkan;
    const struct psw_context_t *software;
#ifdef PSTD_WINDOWS
    const struct pdx_context_t *dx;
    const struct pdx_context_t *directx;
#endif
#ifdef PSTD_MACOS
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
    PHINT_UNDECORATION,
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
};

#ifdef PSTD_WINDOWS
void                  pwindow_hit_device_init(pdevice_t *const);
void                  pwindow_hit_device_shutdown(pdevice_t *const);
void                  pwindow_hit_device_update(pdevice_t *const);
pdevice_proc_result_t pwindow_hit_device_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);
#endif

void                  pkeyboard_init(pdevice_t *const);
void                  pkeyboard_shutdown(pdevice_t *const);
void                  pkeyboard_update(pdevice_t *const);
pdevice_proc_result_t pkeyboard_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);

void                  pmouse_init(pdevice_t *const);
void                  pmouse_shutdown(pdevice_t *const);
void                  pmouse_update(pdevice_t *const);
pdevice_proc_result_t pmouse_wnd_proc(pdevice_t *const, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam);

const pwindow_t *pwindow(const pwindow_info_t *const);

void ppoll_events(const pwindow_t *const);
void pswap_buffers(const pwindow_t *const);
bool prequest_close(const pwindow_t *const);
void pfree_window(const pwindow_t *const);
#endif // PSTD_WINDOW_HEADER
