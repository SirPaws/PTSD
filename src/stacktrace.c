#include "pstacktrace.h"

#include <signal.h>
#include <stdio.h>
#include <limits.h>

#if defined(PSTD_WINDOWS)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#else
#endif

static struct {
    const char *program_name;
} PSTD_STACKTRACE_CTX = {0};


void pstacktrace_signal_handler(int sig);
void pstacktrace_register_signal_handlers(const char *application_name) {
    PSTD_STACKTRACE_CTX.program_name = application_name;
    signal(SIGABRT, pstacktrace_signal_handler);
    signal(SIGFPE , pstacktrace_signal_handler);
    signal(SIGILL , pstacktrace_signal_handler);
    signal(SIGINT , pstacktrace_signal_handler);
    signal(SIGSEGV, pstacktrace_signal_handler);
    signal(SIGTERM, pstacktrace_signal_handler);
}

void pstacktrace_unregister_signal_handlers(void) {
    PSTD_STACKTRACE_CTX.program_name = NULL;
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE , SIG_DFL);
    signal(SIGILL , SIG_DFL);
    signal(SIGINT , SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

void pstacktrace_signal_handler(int sig) {
    pstacktrace_print();
    switch (sig) {
    case SIGABRT: pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGABRT%Cc: " "Abort was called\n");                break; //NOLINT
    case SIGFPE : pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGFPE%Cc: "  "Floating point exception\n");        break;
    case SIGILL : pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGILL%Cc: "  "Illegal instruction\n");             break;
    case SIGINT : pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGINT%Cc: "  "Interrupt signal\n");                break;
    case SIGSEGV: pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGSEGV%Cc: " "Read or Write to invalid memory\n"); break;
    case SIGTERM: pprintf(PSTD_STACKTRACE_ERROR_COLOUR "SIGTERM%Cc: " "Termination signal\n");              break;
    default:      pprintf(PSTD_STACKTRACE_ERROR_COLOUR "UNKNOWN%Cc: " "unknown signal\n");
    }
}

void pstacktrace_pretty_print(pbool_t named, const char *symbol_name, usize offset, void *address) {
#define PROGRAM_NAME(x)  PSTD_STACKTRACE_PATH_COLOUR x "%Cc"
#define FUNCTION_NAME(x) "%Cfg(255,  34, 243)" x "%Cc"
#define NUMBER(x)        PSTD_STACKTRACE_NUMBER_COLOUR x "%Cc"

    pprintf(PROGRAM_NAME("%s") ": ", PSTD_STACKTRACE_CTX.program_name);
    if (named)
        pprintf(FUNCTION_NAME("%s") "+" NUMBER("%#zX") " ", symbol_name, offset);
    pprintf("["NUMBER("%zX")"]\n", (usize)address);
}

void pstacktrace_print(void) {
#if defined(PSTD_WINDOWS)
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
