
#include "pwindow.h"
#include "plimits.h"
#include "pstacktrace.h"
#include "SDL.h"


const pwindow_t *pwindow(const pwindow_info_t *info) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        return NULL;
    
    bool decorated  = true;
    bool fullscreen = false;
    bool resizeable = true;
    for (usize i = 0; i < info->hint_count; i++) {
        if (info->hints[i] == PHINT_NONE) continue;

        switch (info->hints[i]) { //NOLINT
        case PHINT_FULLSCREEN:  fullscreen = true;  break;
        case PHINT_UNDECORATED: decorated  = false; break;
        case PHINT_NO_RESIZE:   resizeable = false; break;
        }
    }

    u32 flags = SDL_WINDOW_SHOWN;
    flags |= SDL_WINDOW_INPUT_FOCUS;
    flags |= SDL_WINDOW_MOUSE_FOCUS;

    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    else {
        if (resizeable)
            flags |= SDL_WINDOW_RESIZABLE;
        if (!decorated)
            flags |= SDL_WINDOW_BORDERLESS;
    }

    const char *title = info->title ?: "";

    passert(plimits(s32).max >= info->x);
    passert(plimits(s32).max >= info->y);
    s32 x = (s32)(info->x == PTSD_ZERO ? 0 : (info->x ?: SDL_WINDOWPOS_UNDEFINED));
    s32 y = (s32)(info->y == PTSD_ZERO ? 0 : (info->y ?: SDL_WINDOWPOS_UNDEFINED));

    passert(plimits(s32).max >= info->width);
    passert(plimits(s32).max >= info->height);
    s32 width  = (s32)(info->width  ?: 960);
    s32 height = (s32)(info->height ?: 540);

    SDL_Window *window = SDL_CreateWindow(title, x, y, width, height, flags);
    if (!window) return NULL;

    s32 actual_x, actual_y;
    SDL_GetWindowPosition(window, &actual_x, &actual_y);

    pwindow_t *result = pzero_allocate(sizeof *result);
    passert(result);
    *result = (pwindow_t) {
        .handle       = (void*)window,
        .width        = width,
        .height       = height,
        .x            = actual_x,
        .y            = actual_y,
        .is_running   = true,
        .device_count = info->device_count,
        .devices      = info->devices,
    };

    SDL_SetWindowData(window, "PTSD_WINDOW_DATA_", result);
    
    for (usize i = 0; i < result->device_count; i++) {
        result->devices[i]->init(result->devices[i]);
    }

    return result;
}

void ppoll_events(const pwindow_t *const win) {
    for (usize i = 0; i < win->device_count; i++)
        win->devices[i]->update(win->devices[i]);
    
    pwindow_t *window = SDL_GetWindowData((void*)win->handle, "PTSD_WINDOW_DATA_");

    SDL_Event event = {0};
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            window->is_running = false;
            continue;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE: {
                    window->is_running = false;
                } break;
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    const int width = event.window.data1;
                    const int height = event.window.data2;
                    window->width  = width;
                    window->height = height;
                } break;
            case SDL_WINDOWEVENT_MOVED: {
                    const int x = event.window.data1;
                    const int y = event.window.data2;
                    window->x = x;
                    window->y = y;
                }
            case SDL_WINDOWEVENT_LEAVE:
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_SHOWN:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
            default: break;
            }
            continue;
        }

        for (usize i = 0; i < win->device_count; i++) {
            bool result = 
                win->devices[i]->device_proc(win->devices[i], &event);
            if (result) break;
        }
    }
}

void pswap_buffers(const pwindow_t *const) {
    //TODO(paw): this probably requires a graphics context, we don't have that yet
}

bool prequest_close(const pwindow_t *const win) {
    pwindow_t *window = SDL_GetWindowData((void*)win->handle, "PTSD_WINDOW_DATA_");
    window->is_running = false;
    return true;
}

void pfree_window(const pwindow_t *const win) {
    pwindow_t *window = SDL_GetWindowData((void*)win->handle, "PTSD_WINDOW_DATA_");
    for (usize i = 0; i < window->device_count; i++) {
        window->devices[i]->shutdown(window->devices[i]);
    }
    SDL_DestroyWindow((void*)window->handle);
    pfree((void*)window);
}

