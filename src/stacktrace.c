#include "pstacktrace.h"

#include <signal.h>
#include <stdio.h>
#include <limits.h>

#if defined(PTSD_WINDOWS)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#else
#endif

static struct {
    const char *program_name;
    bool handled;
} PTSD_STACKTRACE_CTX = {0};

#if PTSD_WINDOWS
BOOL WINAPI pcrtl_c_handler_routine(_In_ DWORD dwCtrlType) {
    (void)dwCtrlType;
    exit(EXIT_FAILURE);
    return TRUE;
}

void pwin32_print_error(DWORD error) {
    if (error != ERROR_SUCCESS) {
        void *message_buffer;
        FormatMessage(
           FORMAT_MESSAGE_ALLOCATE_BUFFER |
           FORMAT_MESSAGE_FROM_SYSTEM |
           FORMAT_MESSAGE_IGNORE_INSERTS,
           NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
           (LPSTR)&message_buffer, 
           0, NULL);
        pprintf("last error was %d: %s\n", error, message_buffer);
    }
}

#endif

void pstacktrace_signal_handler(int sig);
void pstacktrace_register_signal_handlers(const char *application_name) {
    PTSD_STACKTRACE_CTX.program_name = application_name;
    signal(SIGABRT, pstacktrace_signal_handler);
    signal(SIGFPE , pstacktrace_signal_handler);
    signal(SIGILL , pstacktrace_signal_handler);
    signal(SIGINT , pstacktrace_signal_handler);
    signal(SIGSEGV, pstacktrace_signal_handler);
    signal(SIGTERM, pstacktrace_signal_handler);
#if PTSD_WINDOWS
SetConsoleCtrlHandler(pcrtl_c_handler_routine, TRUE);
#endif
}

void pstacktrace_unregister_signal_handlers(void) {
    PTSD_STACKTRACE_CTX.program_name = NULL;
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE , SIG_DFL);
    signal(SIGILL , SIG_DFL);
    signal(SIGINT , SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
#if PTSD_WINDOWS
SetConsoleCtrlHandler(pcrtl_c_handler_routine, FALSE);
#endif
}

void pstacktrace_signal_handler(int sig) {
    if (PTSD_STACKTRACE_CTX.handled)
        return;
#if PTSD_WINDOWS
    u32 error = GetLastError();
#endif

    pstacktrace_print();
    switch (sig) {
    case SIGABRT: pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGABRT%Cc: " "Abort was called\n");                break; //NOLINT
    case SIGFPE : pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGFPE%Cc: "  "Floating point exception\n");        break;
    case SIGILL : pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGILL%Cc: "  "Illegal instruction\n");             break;
    case SIGINT : pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGINT%Cc: "  "Interrupt signal\n");                break;
    case SIGSEGV: pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGSEGV%Cc: " "Read or Write to invalid memory\n"); break;
    case SIGTERM: pprintf(PTSD_STACKTRACE_ERROR_COLOUR "SIGTERM%Cc: " "Termination signal\n");              break;
    default:      pprintf(PTSD_STACKTRACE_ERROR_COLOUR "UNKNOWN%Cc: " "unknown signal\n");
    }
#if PTSD_WINDOWS
    pwin32_print_error(error);
#endif
}

void pstacktrace_pretty_print(bool named, const char *symbol_name, usize offset, void *address) {
#define PROGRAM_NAME(x)  PTSD_STACKTRACE_PATH_COLOUR x "%Cc"
#define FUNCTION_NAME(x) "%Cfg(255,  34, 243)" x "%Cc"
#define NUMBER(x)        PTSD_STACKTRACE_NUMBER_COLOUR x "%Cc"

    pprintf(PROGRAM_NAME("%s") ": ", PTSD_STACKTRACE_CTX.program_name);
    if (named)
        pprintf(FUNCTION_NAME("%s") "+" NUMBER("%#zX") " ", symbol_name, offset);
    pprintf("["NUMBER("%zX")"]\n", (usize)address);
}

void pstacktrace_print(void) {
#if defined(PTSD_WINDOWS)
    void *trace[1024];
    u16 num_captured = CaptureStackBackTrace(0, 1024, trace, 0);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, true);

    IMAGEHLP_SYMBOL *symbol = pzero_allocate(sizeof *symbol + 1024);
    symbol->SizeOfStruct = sizeof *symbol;
    symbol->MaxNameLength = 1024;

    for (u16 i = 0; i < num_captured; i++) {
        usize offset;
        BOOL result = SymGetSymFromAddr(process, (usize)trace[i], &offset, symbol);
        pstacktrace_pretty_print(result, symbol->Name, offset, trace[i]);
    }

    pfree(symbol);
    SymCleanup(process);
#else
#endif
}

void panic(const char *fmt, ...) {
    static const pstream_info_t info = {
        .type  = STRING_STREAM,
        .flags = STREAM_INOUT, 
    };
#if PTSD_WINDOWS
    u32 error = GetLastError();
#endif

    pstring_stream_t out = pcreate_stream(info);
    
    va_list list;
    va_start(list, fmt);
    pvbprintf(&out, fmt, list);
    va_end(list);

    pstring_t str = pstream_to_buffer_string(&out);

    pstacktrace_print();
    pprintf(PTSD_STACKTRACE_ERROR_COLOUR "PANIC%Cc: " "%S\n", str);
#if PTSD_WINDOWS
    pwin32_print_error(error);
#endif
    PTSD_STACKTRACE_CTX.handled = true;
    abort();
}

bool passert_impl(const char *expr, const char *file, u32 line) {
    pstacktrace_print();
    pprintf(PTSD_STACKTRACE_ERROR_COLOUR  "Assertion failed%Cc: '%s', file "
            PTSD_STACKTRACE_PATH_COLOUR   "%s%Cc, line "
            PTSD_STACKTRACE_NUMBER_COLOUR "%u%Cc\n", expr, file, line);
    PTSD_STACKTRACE_CTX.handled = true;
    abort();
    return 0;
}

