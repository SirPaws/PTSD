#include "context_gl.h"
#include "pstacktrace.h"

#include <windows.h>
#include <gl/GL.h>

void pgl_init(pcontext_impl_t *const ctx, pwindow_t *const window) {
    HDC dc = GetDC((void*)window->handle);

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof pfd,
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cDepthBits = 16, //FIXME(Paw): maybe we shouldn't hardcode this?
        .iLayerType = PFD_MAIN_PLANE,
    };

    int format;
    if ((format = ChoosePixelFormat(dc, &pfd)) == 0) {
        panic("cannot create opengl context, ChoosePixelFormat failed");
    }

    if (SetPixelFormat(dc, format, &pfd) == FALSE) {
        panic("cannot create opengl context, SetPixelFormat failed");
    }
    
    DescribePixelFormat(dc, format, sizeof pfd, &pfd);

    HGLRC rc = wglCreateContext(dc);
    if (!rc) {
        panic("cannot create opengl context, wglCreateContext failed");
    }

    if (!wglMakeCurrent(dc, rc)) {
        panic("cannot create opengl context, wglMakeCurrent failed");
    }

    ((pgl_context_t*)ctx)->dc = dc;
    ((pgl_context_t*)ctx)->rc = rc;
    ((pgl_context_t*)ctx)->hwnd = (void*)window->handle;
}

void pgl_shutdown(pcontext_impl_t *const ctx_) {
    pgl_context_t *ctx = (pgl_context_t*)ctx_;

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(ctx->hwnd, ctx->dc);
    wglDeleteContext(ctx->rc);
}

bool pgl_list_devices(pcontext_impl_t *const, u32 *, pcontext_device_id_t[]) { return false; }
bool pgl_get_device_name(pcontext_impl_t *const, pcontext_device_id_t) { return false; }

void pgl_clear(pcontext_impl_t *const, u8 r, u8 g, u8 b, u8 a) {
    glClearColor(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void pgl_prepare_screen(pcontext_impl_t *const) {
}
void pgl_draw_screen(pcontext_impl_t *const ctx_) {
    pgl_context_t *ctx = (pgl_context_t*)ctx_;
    SwapBuffers(ctx->dc);
}
void pgl_teardown_screen(pcontext_impl_t *const) {} 

void pgl_resize_screen(pcontext_impl_t *const, u32, u32) {
    // glViewport(0, 0, width, height);
}

