#pragma once
#ifndef PSTD_STACKTRACE
#define PSTD_STACKTRACE
#include "general.h"
#include "pio.h"

#define PSTD_STACKTRACE_COLOUR(r, g, b)  "%Cfg(" #r "," #g "," #b ")"
#define PSTD_STACKTRACE_ERROR_COLOUR  PSTD_STACKTRACE_COLOUR(255,110,110)
#define PSTD_STACKTRACE_PATH_COLOUR   PSTD_STACKTRACE_COLOUR(121,229,157)
#define PSTD_STACKTRACE_NUMBER_COLOUR PSTD_STACKTRACE_COLOUR(  0,196,255)



pbool_t passert_impl(const char *expr, const char *file, u32 line);

#ifdef NDEBUG
#define passert(expression) ((void)0)
#else
#define passert(expression)                                         \
    (void)(                                                         \
        (!!(expression)) ||                                         \
        (passert_impl(#expression, __FILE__, (unsigned)(__LINE__))) \
    )
#endif

_Noreturn void panic(const char *, ...);

/// registers the signal handlers for stacktrace
void pstacktrace_register_signal_handlers(const char *application_name);
void pstacktrace_unregister_signal_handlers(void);

void pstacktrace_print(void);
#endif // PSTD_STACKTRACE
