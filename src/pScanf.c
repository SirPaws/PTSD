#error pScanf.c is a work in progress
#include "pio.h"
#include "stretchy_buffer.h"
#include <ctype.h>

#define PIO_STATIC PSTD_UNUSED static inline 

#define BASE_10 10
#define BASE_8   8
#define BASE_16 16

typedef struct pScanfInfo pScanfInfo;
struct pScanfInfo {
    GenericStream *stream; 
    const char *restrict fmt;
    va_list list;
    u32 count; 
    pBool *failflag;
    pBool is_supressed;
};

PIO_STATIC void pScanf_handle_length(pScanfInfo *info);
PIO_STATIC void pScanf_handle_number(pScanfInfo *info);
PIO_STATIC void pScanf_handle_supress(pScanfInfo *info);
                                                 
PIO_STATIC void pScanf_handle_set(pScanfInfo *info);
                                                 
PIO_STATIC void pScanf_handle_char(pScanfInfo *info, pBool wide);
PIO_STATIC void pScanf_handle_string(pScanfInfo *info, pBool cstr, pBool is_wide);
PIO_STATIC void pScanf_handle_int(pScanfInfo *info, char printtype);
PIO_STATIC void pScanf_handle_float(pScanfInfo *info);
PIO_STATIC void pScanf_handle_pointer(pScanfInfo *info);
PIO_STATIC void pScanf_handle_characters_written(pScanfInfo *info);

PIO_STATIC void pScanf_handle_binary(pScanfInfo *info);

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list) {
    u32 read_count = 0;
    if (!stream || !stream->is_valid || !(stream->flags & STREAM_INPUT))
        return 0;

    while(*fmt) {
        if (PSTD_EXPECT(*fmt != '%', 1)) {
            const char *restrict fmt_next = fmt;
            while (pCharAnyOf(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) {
                if (isspace(*fmt_next)) {
                    while (isspace(*fmt_next)) fmt_next++;
                } else {
                    char buf[5] = {0}; // pGetUtf8Length return 1-4
                    u32 length = pGetUtf8Length(fmt_next);
                    pStreamRead(stream, buf, length);
                    if (memcmp(buf, fmt_next, length) == 0) {
                        return read_count;
                    }
                }
            }
            read_count += fmt_next - fmt;
            fmt = fmt_next;
        }
        else {
            pBool failed = false;
            const char* restrict fmt_next = fmt + 1;
            pScanfInfo info = {
                .stream = stream,
                .fmt = fmt_next,
                .count = read_count,
                .failflag = &failed,
            };
            va_copy(info.list, list);

            switch (*fmt_next) {
            case '[': pScanf_handle_set(&info); break;
            case 'n': pScanf_handle_characters_written(&info); break;
            case '1': case '2':
            case '3': case '4':
            case '5': case '6':
            case '7': case '8':
            case '9': pScanf_handle_number(&info); break; 
            case '*': pScanf_handle_supress(&info); break;

            case 'h': case 'l':
            case 'j': case 'z':
            case 't': case 'L': pScanf_handle_length(&info); break;

            case 's': case 'S': 
                pScanf_handle_string(&info, *fmt_next == 's' ? true : false); break;

            case 'f': case 'F':
            case 'e': case 'E':
            case 'a': case 'A':
            case 'g': case 'G': pScanf_handle_float(&info); break;

            case 'u':
            case 'x': case 'X':
            case 'i': case 'd':  pScanf_handle_int(&info, *fmt_next); break;
            case 'p': pScanf_handle_pointer(&info); break;
            case 'b': pScanf_handle_binary(&info); break;
            case 'c': pScanf_handle_char(&info, false);   break;
            default: {
                    failed = true;
                }
            }
            if (failed) {
                fmt = info.fmt + 1;
            }
            fmt = info.fmt + 1;
            read_count = info.count;
            list = info.list;
        }
    }
    return read_count;
}

PIO_STATIC void pScanf_handle_length(pScanfInfo *info) {
}

PIO_STATIC void pScanf_handle_number(pScanfInfo *info) {
}

PIO_STATIC void pScanf_handle_supress(pScanfInfo *info) {
}
                                                 
PIO_STATIC void pScanf_handle_set(pScanfInfo *info) {
}
                                                 
PIO_STATIC void pScanf_handle_char(pScanfInfo *info, pBool wide) {
}

PIO_STATIC void pScanf_handle_string(pScanfInfo *info, pBool cstr, pBool is_wide) {
}

PIO_STATIC void pScanf_handle_int(pScanfInfo *info, char printtype) {
}

PIO_STATIC void pScanf_handle_float(pScanfInfo *info) {
}

PIO_STATIC void pScanf_handle_pointer(pScanfInfo *info) {
}

PIO_STATIC void pScanf_handle_characters_written(pScanfInfo *info) {
}
