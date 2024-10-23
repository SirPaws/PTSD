#include "pwindow.h"
#include "plimits.h"
#include "pstacktrace.h"
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <dwmapi.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "Dwmapi.lib")

static LRESULT CALLBACK pwin32_winproc(HWND,UINT,WPARAM,LPARAM);

const pwindow_t *pwindow(const pwindow_info_t *info) {
    void *module = GetModuleHandle(NULL);
    WNDCLASS wndclass = {
        .lpfnWndProc   = pwin32_winproc,
        .lpszClassName = TEXT("PTSD_WINDOW_CLASS_"),
        .hInstance     = module,
    };
    if (!RegisterClass(&wndclass)) return NULL;

    bool decorated  = true;
    // bool fullscreen = false;
    bool resizeable = true;
    for (usize i = 0; i < info->hint_count; i++) {
        if (info->hints[i] == PHINT_NONE) continue;

        switch (info->hints[i]) { //NOLINT
        case PHINT_FULLSCREEN:  panic("fullscreen is unimplemented!"); break;
        case PHINT_UNDECORATED: decorated  = false; break;
        case PHINT_NO_RESIZE:   resizeable = false; break;
        }
    }

    passert(plimits(s32).max >= info->x);
    passert(plimits(s32).max >= info->y);
    s32 x = (s32)(info->x == PTSD_ZERO ? 0 : (info->x ?: CW_USEDEFAULT));
    s32 y = (s32)(info->y == PTSD_ZERO ? 0 : (info->y ?: CW_USEDEFAULT));
    
    passert(plimits(s32).max >= info->width);
    passert(plimits(s32).max >= info->height);
    s32 width  = (s32)(info->width  ?: 960);
    s32 height = (s32)(info->height ?: 540);

    DWORD style    = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if (decorated) {
        style |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        if (resizeable) style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
    } else {
        style |= WS_POPUP | WS_MINIMIZEBOX;
        if (resizeable) style |= WS_BORDER | WS_MAXIMIZEBOX;
    }

    DWORD style_ex = WS_EX_APPWINDOW;
    
    RECT rect = { 0, 0, (s32)width, (s32)height };
    AdjustWindowRectEx(&rect, style, FALSE, style_ex);
    width  = rect.right - rect.left;
    height = rect.bottom - rect.top;

    HWND hwnd = CreateWindowEx(style_ex, TEXT("PTSD_WINDOW_CLASS_"), 
            TEXT(""), style, (s32)x, (s32)y, (s32)width, (s32)height, 
            NULL, NULL, module, NULL);
    ShowWindow(hwnd, SW_SHOW);
    if (!GetWindowRect(hwnd, &rect))
        panic("unable to get window location");

    //has_app = true;
    phandle_t *handle = (void*)hwnd;

    pwindow_t *result = pzero_allocate(sizeof *result);
    passert(result);
    *result = (pwindow_t) {
        .handle       = handle,
        .width        = width,
        .height       = height,
        .x            = rect.left,
        .y            = rect.top,
        .is_running   = true,
        .device_count = info->device_count,
        .devices      = info->devices,
    };

    SetProp(hwnd, "PTSD_WINDOW_DATA_", result);

    if (info->context.any) {
        result->context = info->context.any;
        result->context->init(result->context, result);
    } else result->context = nullptr;
    
    for (usize i = 0; i < result->device_count; i++) {
        result->devices[i]->init(result->devices[i]);
    }

    return result;
}

void ppoll_events(const pwindow_t *const win) {
    for (usize i = 0; i < win->device_count; i++)
        win->devices[i]->update(win->devices[i]);

    MSG msg = {0};
    while (PeekMessage(&msg, (HWND)win->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void pswap_buffers(const pwindow_t *const win) {
    if (win->context) {
        win->context->draw_screen(win->context);
        // SwapBuffers(GetDC((HWND)win->handle));
    }
}

bool prequest_close(const pwindow_t *const win) {
    ((pwindow_t*const)win)->is_running = false;
    return true;
}

void pfree_window(const pwindow_t *const win) {
    for (usize i = 0; i < win->device_count; i++) {
        win->devices[i]->shutdown(win->devices[i]);
    }
    if (win->context) {
        win->context->shutdown(win->context);
    }
    DestroyWindow((HWND)win->handle);
    pfree((void*)win);
}

static LRESULT CALLBACK pwin32_winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    pwindow_t *win = GetProp(hwnd, "PTSD_WINDOW_DATA_");
    if (!win)
        return DefWindowProc(hwnd, msg, wparam, lparam);

    switch(msg) {
    case WM_DESTROY: 
        PostQuitMessage(0);
        win->is_running = false;
        return 0;
    case WM_MOVE: 
        win->x = LOWORD(lparam);
        win->y = HIWORD(lparam);
        return 0;
    case WM_SIZE:
        win->width  = LOWORD(lparam);
        win->height = HIWORD(lparam);
        if (win->context) {
            win->context->resize_screen(win->context, win->width, win->height);
        }
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(lparam) == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;
        }
    }
    
    for (usize i = 0; i < win->device_count; i++) {
        pdevice_proc_result_t result = 
            win->devices[i]->device_proc(win->devices[i], &(pdevice_parameter_pack_t){win, msg, wparam, lparam});
        if (result.handled) return result.result;
    }

    //FIXME(Paw): add this info to the window itself so we don't have to check manually if it's actually undecorated and resizable
    constexpr DWORD resizeable_and_undecorated = WS_POPUP | WS_BORDER;
    int style = GetWindowLong(hwnd, GWL_STYLE);
    if ((style & resizeable_and_undecorated) && msg == WM_NCHITTEST) {
        isize xpos = GET_X_LPARAM(lparam); 
        isize ypos = GET_Y_LPARAM(lparam);

        isize target_minimum = 44;

        bool is_left   = xpos > win->x - target_minimum/2                 && xpos < win->x + target_minimum/2;
        bool is_right  = xpos > (win->x + win->width) - target_minimum/2  && xpos < (win->x + win->width) + target_minimum/2;
        bool is_top    = ypos > win->y - target_minimum/2                 && ypos < win->y + target_minimum/2;
        bool is_bottom = ypos > (win->y + win->height) - target_minimum/2 && ypos < (win->y + win->height) + target_minimum/2;

        int area = is_left << 3 | is_right << 2 | is_top << 1 | is_bottom;
        switch (area) {
        case 0b1000: return HTLEFT;
        case 0b1010: return HTTOPLEFT;
        case 0b1001: return HTBOTTOMLEFT;
        case 0b0100: return HTRIGHT;
        case 0b0110: return HTTOPRIGHT;
        case 0b0101: return HTBOTTOMRIGHT;
        case 0b0010: return HTTOP;
        case 0b0001: return HTBOTTOM;
        default: break;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void pwindow_hit_device_init(pdevice_t *const) { /* INTENTIONAL STUB */ }
void pwindow_hit_device_shutdown(pdevice_t *const) { /* INTENTIONAL STUB */ }
void pwindow_hit_device_update(pdevice_t *const) { /* INTENTIONAL STUB */ }
pdevice_proc_result_t pwindow_hit_device_wnd_proc(pdevice_t *const device, 
        const pdevice_parameter_pack_t *const parm) 
{
    if (parm->msg != WM_NCHITTEST) 
        return (pdevice_proc_result_t){.handled = false};
    
    if (!((phit_device_t*)device)->event_handler)
        return (pdevice_proc_result_t){.handled = false};

    isize xpos = GET_X_LPARAM(parm->lparam); 
    isize ypos = GET_Y_LPARAM(parm->lparam);
    phit_location_t loc = ((phit_device_t*)device)->event_handler((phit_device_t*)device, parm->window, xpos, ypos);
    switch(loc) {
    case PHIT_DEFAULT        : 
        return (pdevice_proc_result_t){0};
    case PHIT_NOWHERE        : return (pdevice_proc_result_t){.handled=true, HTNOWHERE     };
    case PHIT_USERSPACE      : return (pdevice_proc_result_t){.handled=true, HTCLIENT      };
    case PHIT_LEFT           : return (pdevice_proc_result_t){.handled=true, HTLEFT        };
    case PHIT_RIGHT          : return (pdevice_proc_result_t){.handled=true, HTRIGHT       };
    case PHIT_TOP            : return (pdevice_proc_result_t){.handled=true, HTTOP         };
    case PHIT_BOTTOM         : return (pdevice_proc_result_t){.handled=true, HTBOTTOM      };
    case PHIT_TOPLEFT        : return (pdevice_proc_result_t){.handled=true, HTTOPLEFT     };
    case PHIT_TOPRIGHT       : return (pdevice_proc_result_t){.handled=true, HTTOPRIGHT    };
    case PHIT_BOTTOMLEFT     : return (pdevice_proc_result_t){.handled=true, HTBOTTOMLEFT  };
    case PHIT_BOTTOMRIGHT    : return (pdevice_proc_result_t){.handled=true, HTBOTTOMRIGHT };
    case PHIT_TITLEBAR       : return (pdevice_proc_result_t){.handled=true, HTCAPTION     };
    case PHIT_CLOSE_BUTTON   : return (pdevice_proc_result_t){.handled=true, HTCLOSE       };
    case PHIT_MAXIMIZE_BUTTON: return (pdevice_proc_result_t){.handled=true, HTMAXBUTTON   };
    case PHIT_ICONIFY_BUTTON : return (pdevice_proc_result_t){.handled=true, HTMINBUTTON   };

                               // assume the user knowns what they're doing
    default                  : return (pdevice_proc_result_t){.handled=true, loc   };
    }
}



