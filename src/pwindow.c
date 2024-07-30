#include "general.h"
#if PTSD_WINDOW_GLFW
#include "pwindow.glfw.c" //NOLINT
#elif PTSD_WINDOW_SDL
#include "sdl/pkeyboard.c" //NOLINT
#include "sdl/pwindow.c" //NOLINT
#include "sdl/pmouse.c" //NOLINT
#elif defined(PTSD_WINDOWS)
#include "win32/pkeyboard.c" //NOLINT
#include "win32/pwindow.c" //NOLINT
#include "win32/pmouse.c" //NOLINT
#define platform_folder "win32/"
#elif defined(PTSD_LINUX)
#include "pwindow.linux.c" //NOLINT
#elif defined(PTSD_WASM)
#include "pwindow.wasm.c" //NOLINT
#elif defined(PTSD_MACOS)
#include "pwindow.macos.c" //NOLINT
#endif

