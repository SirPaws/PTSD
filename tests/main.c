#include "deps/munit.h"
#include "tests.c"

static const MunitSuite suites[] = {
#define SUITE(x) x,
    SUITES(SUITE)
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static const MunitSuite test_suite = {
  "", nullptr, (void*)suites, 1,
  MUNIT_SUITE_OPTION_NONE
};



int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
#if PTSD_WINDOWS
    // currently munit assumes windows doesn't support 
    // colour output, it does you just have to turn it on
    // I don't feel like doing a fix because it's trivial
    // and anyone can do it, plus it's intentionally made
    // so that if the version of windows used doesn't 
    // support ansi escapes, it'll just fail silently
    char *extra_args[] = {
        "--color", "always",
    };

    char *argv2[argc + 1 + countof(extra_args)];
    for (usize i = 0; i < (usize)argc; i++)
        argv2[i] = argv[i];

    for (usize i = 0; i < countof(extra_args); i++)
        argv[argc + i] = extra_args[i];
    
    argv2[argc + countof(extra_args)] = argv[argc];
    argc = argc + countof(extra_args);
#endif

    return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#if PTSD_WINDOWS
#define read _read
#define write _write
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#define close _close
#define isatty _isatty
#endif
#include "deps/munit.c"

#pragma clang diagnostic pop

