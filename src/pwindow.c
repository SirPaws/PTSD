#include "pwindow.h"
#include "plimits.h"
#include "pstacktrace.h"
#include "util.h"
#include <stddef.h>
#ifdef PSTD_WINDOWS
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Opengl32.lib")
#endif

static LRESULT CALLBACK pwin32_winproc(HWND,UINT,WPARAM,LPARAM);

const pwindow_t *pwindow(const pwindow_info_t *info) {
    void *module = GetModuleHandle(NULL);
    WNDCLASS wndclass = {
        .lpfnWndProc   = pwin32_winproc,
        .lpszClassName = TEXT("PSTD_WINDOW_CLASS_"),
        .hInstance     = module,
    };
    if (!RegisterClass(&wndclass)) return NULL;

    passert(plimits(s32).max >= info->x);
    passert(plimits(s32).max >= info->y);
    s32 x = (s32)(info->x == PSTD_ZERO ? 0 : (info->x ?: CW_USEDEFAULT));
    s32 y = (s32)(info->y == PSTD_ZERO ? 0 : (info->y ?: CW_USEDEFAULT));
    
    passert(plimits(s32).max >= info->width);
    passert(plimits(s32).max >= info->height);
    s32 width  = (s32)(info->width  ?: 960);
    s32 height = (s32)(info->height ?: 540);

    DWORD style    = WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_POPUP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN;
    DWORD style_ex = WS_EX_APPWINDOW;
    
    RECT rect = { 0, 0, (s32)width, (s32)height };
    AdjustWindowRectEx(&rect, style, FALSE, style_ex);
    width  = rect.right - rect.left;
    height = rect.bottom - rect.top;

    HWND hwnd = CreateWindowEx(style_ex, TEXT("PSTD_WINDOW_CLASS_"), 
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

    SetProp(hwnd, "PSTD_WINDOW_DATA_", result);
    
    for (usize i = 0; i < result->device_count; i++) {
        result->devices[i]->init(result->devices[i]);
    }

    return result;
}

void ppoll_events(const pwindow_t *const win) {
    MSG msg = {0};
    while (PeekMessage(&msg, (HWND)win->handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    for (usize i = 0; i < win->device_count; i++)
        win->devices[i]->update(win->devices[i]);
}

void pswap_buffers(const pwindow_t *const win) {
    SwapBuffers(GetDC((HWND)win->handle));
}

bool prequest_close(const pwindow_t *const win) {
    ((pwindow_t*const)win)->is_running = false;
    return true;
}

void pfree_window(const pwindow_t *const win) {
    for (usize i = 0; i < win->device_count; i++) {
        win->devices[i]->shutdown(win->devices[i]);
    }
    DestroyWindow((HWND)win->handle);
    pfree((void*)win);
}

static LRESULT CALLBACK pwin32_winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    pwindow_t *win = GetProp(hwnd, "PSTD_WINDOW_DATA_");
    if (!win)
        return DefWindowProc(hwnd, msg, wparam, lparam);

    switch(msg) {
    case WM_DESTROY: 
        PostQuitMessage(0);
        win->is_running = false;
        return 0;
    }
    for (usize i = 0; i < win->device_count; i++) {
        pdevice_proc_result_t result = 
            win->devices[i]->wnd_proc(win->devices[i], win, msg, wparam, lparam);
        if (result.handled) return result.result;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void pwindow_hit_device_init(pdevice_t *const) { /* INTENTIONAL STUB */ }
void pwindow_hit_device_shutdown(pdevice_t *const) { /* INTENTIONAL STUB */ }
void pwindow_hit_device_update(pdevice_t *const) { /* INTENTIONAL STUB */ }
pdevice_proc_result_t pwindow_hit_device_wnd_proc(pdevice_t *const device, 
        pwindow_t *const win, u32 msg, usize, plong_ptr_t lparam) 
{
    if (msg != WM_NCHITTEST) 
        return (pdevice_proc_result_t){.handled = false};
    
    if (!((phit_device_t*)device)->event_handler)
        return (pdevice_proc_result_t){.handled = false};

    isize xpos = GET_X_LPARAM(lparam); 
    isize ypos = GET_Y_LPARAM(lparam);
    phit_location_t loc = ((phit_device_t*)device)->event_handler(win, xpos, ypos);
    switch(loc) {
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

void pkeyboard_init(pdevice_t *const device) {
    pkeyboard_t *keyboard = (void*)device;
    pmemexpand(keyboard->keycodes , &((pkeycode_t){-1}), sizeof(keyboard->keycodes), sizeof(pkeycode_t));
    pmemexpand(keyboard->scancodes, &((usize){-1}), sizeof(keyboard->scancodes), sizeof(usize));
    memset(keyboard->keys, 0, sizeof(keyboard->scancodes));


    keyboard->keycodes[0x00B] = PSTD_KEY_0;
    keyboard->keycodes[0x002] = PSTD_KEY_1;
    keyboard->keycodes[0x003] = PSTD_KEY_2;
    keyboard->keycodes[0x004] = PSTD_KEY_3;
    keyboard->keycodes[0x005] = PSTD_KEY_4;
    keyboard->keycodes[0x006] = PSTD_KEY_5;
    keyboard->keycodes[0x007] = PSTD_KEY_6;
    keyboard->keycodes[0x008] = PSTD_KEY_7;
    keyboard->keycodes[0x009] = PSTD_KEY_8;
    keyboard->keycodes[0x00A] = PSTD_KEY_9;
    keyboard->keycodes[0x01E] = PSTD_KEY_A;
    keyboard->keycodes[0x030] = PSTD_KEY_B;
    keyboard->keycodes[0x02E] = PSTD_KEY_C;
    keyboard->keycodes[0x020] = PSTD_KEY_D;
    keyboard->keycodes[0x012] = PSTD_KEY_E;
    keyboard->keycodes[0x021] = PSTD_KEY_F;
    keyboard->keycodes[0x022] = PSTD_KEY_G;
    keyboard->keycodes[0x023] = PSTD_KEY_H;
    keyboard->keycodes[0x017] = PSTD_KEY_I;
    keyboard->keycodes[0x024] = PSTD_KEY_J;
    keyboard->keycodes[0x025] = PSTD_KEY_K;
    keyboard->keycodes[0x026] = PSTD_KEY_L;
    keyboard->keycodes[0x032] = PSTD_KEY_M;
    keyboard->keycodes[0x031] = PSTD_KEY_N;
    keyboard->keycodes[0x018] = PSTD_KEY_O;
    keyboard->keycodes[0x019] = PSTD_KEY_P;
    keyboard->keycodes[0x010] = PSTD_KEY_Q;
    keyboard->keycodes[0x013] = PSTD_KEY_R;
    keyboard->keycodes[0x01F] = PSTD_KEY_S;
    keyboard->keycodes[0x014] = PSTD_KEY_T;
    keyboard->keycodes[0x016] = PSTD_KEY_U;
    keyboard->keycodes[0x02F] = PSTD_KEY_V;
    keyboard->keycodes[0x011] = PSTD_KEY_W;
    keyboard->keycodes[0x02D] = PSTD_KEY_X;
    keyboard->keycodes[0x015] = PSTD_KEY_Y;
    keyboard->keycodes[0x02C] = PSTD_KEY_Z;

    keyboard->keycodes[0x028] = PSTD_KEY_APOSTROPHE;
    keyboard->keycodes[0x02B] = PSTD_KEY_BACKSLASH;
    keyboard->keycodes[0x033] = PSTD_KEY_COMMA;
    keyboard->keycodes[0x00D] = PSTD_KEY_EQUAL;
    keyboard->keycodes[0x029] = PSTD_KEY_GRAVE_ACCENT;
    keyboard->keycodes[0x01A] = PSTD_KEY_LEFT_BRACKET;
    keyboard->keycodes[0x00C] = PSTD_KEY_MINUS;
    keyboard->keycodes[0x034] = PSTD_KEY_PERIOD;
    keyboard->keycodes[0x01B] = PSTD_KEY_RIGHT_BRACKET;
    keyboard->keycodes[0x027] = PSTD_KEY_SEMICOLON;
    keyboard->keycodes[0x035] = PSTD_KEY_SLASH;
    keyboard->keycodes[0x056] = PSTD_KEY_WORLD_2;

    keyboard->keycodes[0x00E] = PSTD_KEY_BACKSPACE;
    keyboard->keycodes[0x153] = PSTD_KEY_DELETE;
    keyboard->keycodes[0x14F] = PSTD_KEY_END;
    keyboard->keycodes[0x01C] = PSTD_KEY_ENTER;
    keyboard->keycodes[0x001] = PSTD_KEY_ESCAPE;
    keyboard->keycodes[0x147] = PSTD_KEY_HOME;
    keyboard->keycodes[0x152] = PSTD_KEY_INSERT;
    keyboard->keycodes[0x15D] = PSTD_KEY_MENU;
    keyboard->keycodes[0x151] = PSTD_KEY_PAGE_DOWN;
    keyboard->keycodes[0x149] = PSTD_KEY_PAGE_UP;
    keyboard->keycodes[0x045] = PSTD_KEY_PAUSE;
    keyboard->keycodes[0x146] = PSTD_KEY_PAUSE;
    keyboard->keycodes[0x039] = PSTD_KEY_SPACE;
    keyboard->keycodes[0x00F] = PSTD_KEY_TAB;
    keyboard->keycodes[0x03A] = PSTD_KEY_CAPS_LOCK;
    keyboard->keycodes[0x145] = PSTD_KEY_NUM_LOCK;
    keyboard->keycodes[0x046] = PSTD_KEY_SCROLL_LOCK;
    keyboard->keycodes[0x03B] = PSTD_KEY_F1;
    keyboard->keycodes[0x03C] = PSTD_KEY_F2;
    keyboard->keycodes[0x03D] = PSTD_KEY_F3;
    keyboard->keycodes[0x03E] = PSTD_KEY_F4;
    keyboard->keycodes[0x03F] = PSTD_KEY_F5;
    keyboard->keycodes[0x040] = PSTD_KEY_F6;
    keyboard->keycodes[0x041] = PSTD_KEY_F7;
    keyboard->keycodes[0x042] = PSTD_KEY_F8;
    keyboard->keycodes[0x043] = PSTD_KEY_F9;
    keyboard->keycodes[0x044] = PSTD_KEY_F10;
    keyboard->keycodes[0x057] = PSTD_KEY_F11;
    keyboard->keycodes[0x058] = PSTD_KEY_F12;
    keyboard->keycodes[0x064] = PSTD_KEY_F13;
    keyboard->keycodes[0x065] = PSTD_KEY_F14;
    keyboard->keycodes[0x066] = PSTD_KEY_F15;
    keyboard->keycodes[0x067] = PSTD_KEY_F16;
    keyboard->keycodes[0x068] = PSTD_KEY_F17;
    keyboard->keycodes[0x069] = PSTD_KEY_F18;
    keyboard->keycodes[0x06A] = PSTD_KEY_F19;
    keyboard->keycodes[0x06B] = PSTD_KEY_F20;
    keyboard->keycodes[0x06C] = PSTD_KEY_F21;
    keyboard->keycodes[0x06D] = PSTD_KEY_F22;
    keyboard->keycodes[0x06E] = PSTD_KEY_F23;
    keyboard->keycodes[0x076] = PSTD_KEY_F24;
    keyboard->keycodes[0x038] = PSTD_KEY_LEFT_ALT;
    keyboard->keycodes[0x01D] = PSTD_KEY_LEFT_CONTROL;
    keyboard->keycodes[0x02A] = PSTD_KEY_LEFT_SHIFT;
    keyboard->keycodes[0x15B] = PSTD_KEY_LEFT_SUPER;
    keyboard->keycodes[0x137] = PSTD_KEY_PRINT_SCREEN;
    keyboard->keycodes[0x138] = PSTD_KEY_RIGHT_ALT;
    keyboard->keycodes[0x11D] = PSTD_KEY_RIGHT_CONTROL;
    keyboard->keycodes[0x036] = PSTD_KEY_RIGHT_SHIFT;
    keyboard->keycodes[0x15C] = PSTD_KEY_RIGHT_SUPER;
    keyboard->keycodes[0x150] = PSTD_KEY_DOWN;
    keyboard->keycodes[0x14B] = PSTD_KEY_LEFT;
    keyboard->keycodes[0x14D] = PSTD_KEY_RIGHT;
    keyboard->keycodes[0x148] = PSTD_KEY_UP;

    keyboard->keycodes[0x052] = PSTD_KEY_KP_0;
    keyboard->keycodes[0x04F] = PSTD_KEY_KP_1;
    keyboard->keycodes[0x050] = PSTD_KEY_KP_2;
    keyboard->keycodes[0x051] = PSTD_KEY_KP_3;
    keyboard->keycodes[0x04B] = PSTD_KEY_KP_4;
    keyboard->keycodes[0x04C] = PSTD_KEY_KP_5;
    keyboard->keycodes[0x04D] = PSTD_KEY_KP_6;
    keyboard->keycodes[0x047] = PSTD_KEY_KP_7;
    keyboard->keycodes[0x048] = PSTD_KEY_KP_8;
    keyboard->keycodes[0x049] = PSTD_KEY_KP_9;
    keyboard->keycodes[0x04E] = PSTD_KEY_KP_ADD;
    keyboard->keycodes[0x053] = PSTD_KEY_KP_DECIMAL;
    keyboard->keycodes[0x135] = PSTD_KEY_KP_DIVIDE;
    keyboard->keycodes[0x11C] = PSTD_KEY_KP_ENTER;
    keyboard->keycodes[0x059] = PSTD_KEY_KP_EQUAL;
    keyboard->keycodes[0x037] = PSTD_KEY_KP_MULTIPLY;
    keyboard->keycodes[0x04A] = PSTD_KEY_KP_SUBTRACT;

    for (u32 scancode = 0; scancode < 512; scancode++) {
        if (keyboard->keycodes[scancode] != -1)
            keyboard->scancodes[keyboard->keycodes[scancode]] = scancode;
    }
}

void pkeyboard_shutdown(pdevice_t *const) { }
void pkeyboard_update(pdevice_t *const device) {
    pkeyboard_t *keyboard = (void*)device;
    static pstate_t oldkeystate[512] = {0};

    pmodifier_t mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= PSTD_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= PSTD_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= PSTD_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= PSTD_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= PSTD_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= PSTD_MOD_NUM_LOCK;

    pstate_t *keystate = keyboard->keys;
    for (register usize i = 0; i < 512; i++) {
        keystate->modifiers = mods;
        if (keystate[i].pressed) {
            if (!oldkeystate[i].pressed) {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = true;
                keystate[i].held     = true;
            }
            else {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = false;
                keystate[i].held     = true;
            }
        }
        else if (keystate[i].released) {
            if (oldkeystate[i].released) {
                oldkeystate[i] = keystate[i];
                keystate[i].released = true;
                keystate[i].pressed  = false;
                keystate[i].held     = false;
            }
            else {
                oldkeystate[i] = keystate[i];
                keystate[i].released = false;
                keystate[i].pressed  = false;
                keystate[i].held     = false;
            }
        }
        else if (keystate[i].held) {
            oldkeystate[i] = keystate[i];
            keystate[i].released = false;
            keystate[i].pressed  = false;
        }
    }
}

pdevice_proc_result_t pkeyboard_wnd_proc(pdevice_t *const device, 
        pwindow_t *const win, u32 msg, usize wparam, plong_ptr_t lparam)
{
    pkeyboard_t *keyboard = (void*)device;
    switch(msg) {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
            int action   = (HIWORD(lparam) & KF_UP) ? 0 : 1;
            unsigned int scancode = (HIWORD(lparam) & (KF_EXTENDED | 0xff));
            if (!scancode) {
                scancode = MapVirtualKeyW((UINT) wparam, MAPVK_VK_TO_VSC);
            }
            pkeycode_t key = keyboard->keycodes[scancode];
            if (action == 1 && !keyboard->keys[key].held) {
                keyboard->keys[key].pressed = true;
            }
            else if (action == 0) keyboard->keys[key].released = true;
        }
        return (pdevice_proc_result_t){.handled=true, DefWindowProc((HWND)win->handle, msg, wparam, lparam)};
    }
    return (pdevice_proc_result_t){0};
}

void pmouse_init(pdevice_t *const device) {
    pmouse_t *mouse = (void*)device;
    memset(mouse->inputs, 0, sizeof mouse->inputs);
}
void pmouse_shutdown(pdevice_t *const) {}

void pmouse_update(pdevice_t *const device) {
    static pstate_t oldmousestate[PSTD_MOUSE_INPUT_COUNT] = {0};
    pmouse_t *mouse = (void*)device;
    
    pmodifier_t mods = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= PSTD_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= PSTD_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= PSTD_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= PSTD_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= PSTD_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= PSTD_MOD_NUM_LOCK;
    
    pstate_t *mousestate = mouse->inputs;
    for (int i = 0; i < PSTD_MOUSE_BUTTON_31 + 1; i++) {
        mousestate[i].modifiers = mods;
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

pdevice_proc_result_t pmouse_wnd_proc(pdevice_t *const device, 
        pwindow_t *const, u32 msg, usize wparam, plong_ptr_t lparam)
{
    pmouse_t *mouse = (void*)device;

    switch(msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP: {
            pmouse_button_t button; int action; 
            /**/ if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
                 button = PSTD_MOUSE_LEFT;
            else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) //NOLINT
                 button = PSTD_MOUSE_RIGHT;
            else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP) //NOLINT
                 button = PSTD_MOUSE_MIDDLE;
            else if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)//NOLINT
                 button = PSTD_MOUSE_BUTTON_4;
            else button = PSTD_MOUSE_BUTTON_5;

            if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN 
             || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN) {
                action = 1;
            } else {
                action = 0;
            }
            
            if      (action == 1) mouse->inputs[button].pressed  = true;
            else if (action == 0) mouse->inputs[button].released = true;
            mouse->inputs[PSTD_MOUSE_XY].x = GET_X_LPARAM(lparam);
            mouse->inputs[PSTD_MOUSE_XY].y = GET_Y_LPARAM(lparam);
            return (pdevice_proc_result_t){.handled=true, 0};
        }
    case WM_MOUSEWHEEL:
        mouse->inputs[PSTD_MOUSE_WHEEL_XY].y = (SHORT) HIWORD(wparam) / (double) WHEEL_DELTA;
        return (pdevice_proc_result_t){.handled=true, 0};
    case WM_MOUSEHWHEEL:
        mouse->inputs[PSTD_MOUSE_WHEEL_XY].x = (SHORT) HIWORD(wparam) / (double) WHEEL_DELTA;
        return (pdevice_proc_result_t){.handled=true, 0};
    case WM_MOUSEMOVE: 
        mouse->inputs[PSTD_MOUSE_XY].x = GET_X_LPARAM(lparam);
        mouse->inputs[PSTD_MOUSE_XY].y = GET_Y_LPARAM(lparam);
        return (pdevice_proc_result_t){.handled=true, 0};
    }
    return (pdevice_proc_result_t){0};
}



















