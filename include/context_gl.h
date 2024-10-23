#pragma once
#ifndef PTSD_CONTEXT_GL_HEADER
#define PTSD_CONTEXT_GL_HEADER
#include "pwindow.h"

#define PGL_VERSION(major, minor) ((((u32)(major)) << 16) | ((u32)(minor)))

typedef struct pgl_context_t pgl_context_t;
struct pgl_context_t {
    pcontext_impl_t impl;
    u32 version;
#if PTSD_WINDOWS
    struct HDC__ *dc;
    struct HGLRC__ *rc;
    struct HWND__ *hwnd;
#endif
};

PTSD_UNUSED
static inline pgl_context_t pgl_context(u32 version) {
    void pgl_init(pcontext_impl_t *const, pwindow_t *const);
    void pgl_shutdown(pcontext_impl_t *const);

    bool pgl_list_devices(pcontext_impl_t *const, u32 *num_devices, pcontext_device_id_t buffer[*num_devices]);
    bool pgl_get_device_name(pcontext_impl_t *const, pcontext_device_id_t id);

    void pgl_resize_screen(pcontext_impl_t *const, u32 width, u32 height);

    void pgl_clear(pcontext_impl_t *const, u8 r, u8 g, u8 b, u8 a);
    void pgl_prepare_screen(pcontext_impl_t *const);
    void pgl_draw_screen(pcontext_impl_t *const);
    void pgl_teardown_screen(pcontext_impl_t *const);

    return (pgl_context_t) {
        .impl = {
            pgl_init,
            pgl_shutdown,

            pgl_list_devices,
            pgl_get_device_name,
    
            pgl_resize_screen,

            pgl_clear,
            pgl_prepare_screen,
            pgl_draw_screen,
            pgl_teardown_screen,
        },
        .version = version ?: PGL_VERSION(3, 3)
    };
}

#endif // PTSD_CONTEXT_GL_HEADER

