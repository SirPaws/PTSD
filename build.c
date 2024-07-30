// with clang on windows:
// clang -Iinclude build.c -o ptsd.o && llvm-lib /OUT:ptsd.lib ptsd.o
// 
// with clang on linux:
// clang -Iinclude build.c -o ptsd.o && ar -cr libptsd.a ptsd.o
//
// if you want to use glfw or sdl as a windowing backend, add `-DPTSD_WINDOW_GLFW` or `-DPTSD_WINDOW_SDL` to the command
// so something like:
// clang -DPTSD_WINDOW_SDL -Iinclude build.c ... 

#include "src/pio.c" //NOLINT
#include "src/pplatform.c" //NOLINT
#include "src/pstring.c" //NOLINT
#include "src/ptime.c" //NOLINT
#include "src/pwindow.c" //NOLINT
#include "src/random.c" //NOLINT
#include "src/stacktrace.c" //NOLINT
#include "src/util.c" //NOLINT

