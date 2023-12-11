// with clang on windows:
// clang -Iinclude build.c -o ptsd.o && llvm-lib /OUT:ptsd.lib ptsd.o
// 
// with clang on linux:
// clang -Iinclude build.c -o ptsd.o && ar -cr libptsd.a ptsd.o

#include "src/pio.c" //NOLINT
#include "src/pplatform.c" //NOLINT
#include "src/pstring.c" //NOLINT
#include "src/ptime.c" //NOLINT
#include "src/pwindow.c" //NOLINT
#include "src/random.c" //NOLINT
#include "src/stacktrace.c" //NOLINT
#include "src/util.c" //NOLINT

