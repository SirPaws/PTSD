#pragma once
#ifndef PSTD_STACKTRACE
#define PSTD_STACKTRACE
#include "general.h"
#include "pio.h"

#define PSTD_STACKTRACE_COLOUR(r, g, b)  "%Cfg(" #r "," #g "," #b ")"
#define PSTD_STACKTRACE_ERROR_COLOUR  PSTD_STACKTRACE_COLOUR(255,110,110)
#define PSTD_STACKTRACE_PATH_COLOUR   PSTD_STACKTRACE_COLOUR(121,229,157)
#define PSTD_STACKTRACE_NUMBER_COLOUR PSTD_STACKTRACE_COLOUR(  0,196,255)



PSTD_UNUSED static inline int passert(const char *expr, const char *file, u32 line) {
#warning TODO: parse expression text and format it
    pprintf(PSTD_STACKTRACE_ERROR_COLOUR  "Assertion failed%Cc: '%s', file "
            PSTD_STACKTRACE_PATH_COLOUR   "%s%Cc, line "
            PSTD_STACKTRACE_NUMBER_COLOUR "%u%Cc\n", expr, file, line);
    abort();
}

#ifdef NDEBUG
#define passert(expression) ((void)0)
#else
#define passert(expression)                                    \
    (void)(                                                    \
        (!!(expression)) ||                                    \
        (passert(#expression, __FILE__, (unsigned)(__LINE__))) \
    )
#endif

_Noreturn void panic(const char *, ...);

/// registers the signal handlers for stacktrace
void pstacktrace_register_signal_handlers(const char *application_name);
void pstacktrace_unregister_signal_handlers(void);

void pstacktrace_print(void);
#endif // PSTD_STACKTRACE
