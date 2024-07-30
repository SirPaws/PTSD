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


// typedef struct pstate_t pstate_t;
// struct pstate_t {
//     usize       code;
//     pmodifier_t modifiers;
//     union {
//         struct { bool held, pressed, released; };
//         struct { f64 x, y; };
//     };
// };

#ifdef PTSD_WINDOWS
typedef struct pdevice_proc_result_t pdevice_proc_result_t;
struct pdevice_proc_result_t {
    bool handled; plong_ptr_t result;
};
#endif

typedef struct pwindow_procedure_parameter_pack_t pwindow_procedure_parameter_pack_t;
#ifdef PTSD_WINDOWS
struct pwindow_procedure_parameter_pack_t {
    struct pwindow_t *const window;
    u32 msg;
    usize wparam;
    plong_ptr_t lparam;
};
#elif defined(PTSD_WASM)
#error device callback has not been defined for this platform
#elif defined(PTSD_LINUX)
#error device callback has not been defined for this platform
#elif defined(PTSD_MACOS)
#error device callback has not been defined for this platform
#endif


typedef struct pdevice_t pdevice_t;
struct pdevice_t {
    void (*init)(pdevice_t *const);
    void (*shutdown)(pdevice_t *const);
    void (*update)(pdevice_t *const);
#ifdef PTSD_WINDOWS
    pdevice_proc_result_t (*wnd_proc)(pdevice_t *const, pwindow_procedure_parameter_pack_t[static const 1]);
#endif
};

#ifdef PTSD_WINDOWS
#endif

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


/*

   we make an action
    action <- list of bindings
    
    1024 keys (should be good enough)

    bindings_t {
        binding_t *binding;
        bindings_t *next;
    }
    
    action {
        usize count;
        binding_t *bindings = nullptr;
    }

    binding_t {
        binding_t *next = nullptr;
        key_t *key;
    }

    some action = {
        .count = 1,
        .bindings = alloc &{ 
            .key = &keys[CTRL],
            .next = alloc &{
                .key = &keys[SHIFT],
                .next = alloc &{
                    .key = &keys['T']
                }
            }
        }
    } 

*/

const pwindow_t *pwindow(const pwindow_info_t *const);

void ppoll_events(const pwindow_t *const);
void pswap_buffers(const pwindow_t *const);
bool prequest_close(const pwindow_t *const);
void pfree_window(const pwindow_t *const);


///////////////////////////////////////////////////////////////////////////////////////
// windows custom window hit handling /////////////////////////////////////////////////

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
PTSD_UNUSED
static phit_device_t phit_device(phit_location_t (*event_handler)(pwindow_t*, isize, isize)) {
    void                  pwindow_hit_device_init(pdevice_t *const);
    void                  pwindow_hit_device_shutdown(pdevice_t *const);
    void                  pwindow_hit_device_update(pdevice_t *const);
    pdevice_proc_result_t pwindow_hit_device_wnd_proc(pdevice_t *const, 
            pwindow_procedure_parameter_pack_t[static const 1]);
   return (phit_device_t){
        .device = {
            .init          = pwindow_hit_device_init,
            .shutdown      = pwindow_hit_device_shutdown,
            .update        = pwindow_hit_device_update,
            .wnd_proc      = pwindow_hit_device_wnd_proc,
        },
        .event_handler = event_handler
    };
}
#endif

#endif // PTSD_WINDOW_HEADER
