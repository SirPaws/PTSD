#pragma once
#ifndef PTSD_PIO_HEADER
#define PTSD_PIO_HEADER

#include <stdarg.h>

#include "general.h"
#include "pstring.h"
#include "pplatform.h"


#define stretchy 

#if defined(__cplusplus)
#define restrict
extern "C" {
#endif



/*
    pPrintf extra format specifiers
    
    |-----------------------------------------------------------|
    |S              | prints a pstring_t (char *, with length)  |
    |-----------------------------------------------------------|
    |Cc             | clear color output                        |
    |-----------------------------------------------------------|
    |Cbg(r, g, b)   | set background color                      |
    |-----------------------------------------------------------|
    |Cfg(r, g, b)   | set foreground color                      |
    |-----------------------------------------------------------|
    |b              | binary. supports '.', '0',                |
    |               | and, length modifiers                     |
    |               | example would be "%hhb", 0b00001010       |
    |               | the example above would print 1010        |
    |-----------------------------------------------------------|
    |B              | boolean. supports '#', length modifiers.  |
    |               | this takes in an integer, if it's zero    |
    |               | it prints 'false', if it's non zero       |
    |               | prints 'true', if the '#' modifier is used|
    |               | it will print them in uppercase           |
    |-----------------------------------------------------------|
    |generic        | this is not a specifier but the pprintf   | 
    |               | functions allows for custom specifiers    |
    |               | this is done through the `pformat_push`   |
    |               | and `pformat_pop` functions.              |
    |               | see examples/formatting.c for usage       |
    |-----------------------------------------------------------|
*/

// not sure what these todos are about, seems more like a pplatform type of thing
// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 
typedef struct pgeneric_stream_t pgeneric_stream_t;
struct pgeneric_stream_t {
    bool is_valid;

    enum pstream_type_t {
        STANDARD_STREAM,
        FILE_STREAM,
        STRING_STREAM,
        CFILE_STREAM,
    } type;
    enum pstream_flags_t {
        STREAM_OUTPUT = 0b01U,
        STREAM_INPUT  = 0b10U,
        STREAM_READ   = STREAM_INPUT,
        STREAM_WRITE  = STREAM_OUTPUT,
        STREAM_READ_WRITE   = STREAM_INPUT|STREAM_OUTPUT,
        STREAM_INOUT        = STREAM_READ_WRITE,
    } flags;
    union {
        struct { // pstring_t stream
            char  *stretchy buffer;
            usize cursor;
        };
        struct { // File stream
            phandle_t *handle;
            usize size;
            pstring_t file_buffer;
        };
        struct { // Standard stream
            phandle_t *stdout_handle;
            phandle_t *stdin_handle;
        };
        struct { // c 'stream'
            FILE *file;
        };
    };
};
typedef pgeneric_stream_t pstring_stream_t; 
typedef pgeneric_stream_t pfile_stream_t;
typedef pgeneric_stream_t pstd_stream_t;
typedef pgeneric_stream_t pcfile_stream_t;

typedef struct pstream_info_t pstream_info_t;
struct pstream_info_t {
    enum pstream_type_t type;  // used by all streams
    u32 flags;                 // used by all streams
    usize buffer_size;         // used by stringstream, ignored by filestream
    char *filename;            // used by filestream
    bool create_buffer;        // used by filestream, ignored by stringstream
    bool create_if_not_exists; // used by filestream
};

// sets the current bound stream
// meaning all calls to functions that don't take an explicit stream parameter
// will be using the new_stream
void pset_stream(pgeneric_stream_t *new_stream, pgeneric_stream_t *old_stream);

// gets the current bound stream
pgeneric_stream_t *pget_stream(void);

// creates a stream
pgeneric_stream_t pcreate_stream(pstream_info_t info);

// frees the stream, note that it does not call free(stream)
void pfree_stream(pgeneric_stream_t *stream);

// creates a string from the contents from a stream with a STREAM_INPUT/STREAM_READ flag
pstring_t pstream_to_buffer_string(pgeneric_stream_t *stream);

u32 pvbprintf(pgeneric_stream_t *stream, const char *restrict fmt, va_list list);

PTSD_UNUSED
static u32 pbprintf(pgeneric_stream_t *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pvbprintf(stream, fmt, list);
    va_end(list);
    return result;
}

static inline u32 pvprintf(const char *restrict fmt, va_list list ) {
    return pvbprintf(pget_stream(), fmt, list);
}

PTSD_UNUSED
static inline u32 pprintf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pvprintf(fmt, list);
    va_end(list);
    return result;
}

void pwrite_stream_s(pgeneric_stream_t *stream, const pstring_t str);
void pwrite_stream_c(pgeneric_stream_t *stream, const char chr);

PTSD_UNUSED
static inline void pputchar(const char c) {
     pwrite_stream_c(pget_stream(), c);
}

PTSD_UNUSED
static inline void pputstring(pstring_t s) {
    pwrite_stream_s(pget_stream(), s);
}

PTSD_UNUSED
static inline void pputc(const char c) {
    pgeneric_stream_t *stream = pget_stream();
    if (c) pwrite_stream_c(stream, c);
    pwrite_stream_c(stream, '\n');
}

PTSD_UNUSED
static inline void pputs(const char *s) {
    pgeneric_stream_t *stream = pget_stream();
    pstring_t str = pstring((char*)s, strlen(s));
    pwrite_stream_s(stream, str);
    pwrite_stream_c(stream, '\n');
}

// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void  pread_stream(pgeneric_stream_t *stream, void *buf, usize size);

PTSD_UNUSED
static inline void pread_into(void *buf, usize size) {
    pread_stream(pget_stream(), buf, size);
}

// line needs to be freed
bool pread_line_stream(pgeneric_stream_t *stream, pstring_t *string);

PTSD_UNUSED
static inline bool pread_line(pstring_t *str) {
    return pread_line_stream(pget_stream(), str);
}


// move the stream pointer forwards or backwards
// size is how many bytes to move the stream pointer
//
// so if the stream pointer points here
// example text that the stream holds
// ----------------^
// 
// and you wan't to move it back 3 bytes
// you would do pmove_stream(stream, -3)
// and now the pointer points here instead
// example text that the stream holds
// -------------^
//
//
void pmove_stream(pgeneric_stream_t *stream, isize size);

// moves the file pointer back to the start of the file
void preset_stream(pgeneric_stream_t *stream);

// psigned_###_to_string appends + or - to the start of buffer then calls punsigned_###_to_string

u32 psigned_int_to_string(char *buf, s64 num, u32 radix,   const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);
u32 punsigned_int_to_string(char *buf, u64 num, u32 radix, const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);

u32 psigned_decimal_to_string(char *buf, s64 num);
u32 punsigned_decimal_to_string(char *buf, u64 num);

u32 psigned_hex_to_string(char *buf, s64 num);
u32 punsigned_hex_to_string(char *buf, u64 num);

u32 psigned_octal_to_string(char *buf, s64 num);
u32 punsigned_octal_to_string(char *buf, u64 num);

// u32 pftoa(char *buf, f32);
// u32 pdtoa(char *buf, f64);


#if PTSD_HAS_VLA || PTSD_C_VERSION >= PTSD_C23
PTSD_UNUSED
static inline bool pchar_anyof(int character, u32 count, const char tests[count]) {//NOLINT
#else
PTSD_UNUSED
static inline bool pchar_anyof(int character, u32 count, const char tests[]) {
#endif
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

PTSD_UNUSED
static inline usize putf8_length(const char *chr) {
#define PTSD_UNICODE_MASK 0xf8 
#define PTSD_UNICODE_4_BYTES 0xf0
#define PTSD_UNICODE_3_BYTES 0xe0
#define PTSD_UNICODE_2_BYTES 0xc0
    if ((chr[0] & PTSD_UNICODE_MASK) == PTSD_UNICODE_4_BYTES)
         return 4;
    else if ((chr[0] & PTSD_UNICODE_4_BYTES) == PTSD_UNICODE_3_BYTES)
         return 3;
    else if ((chr[0] & PTSD_UNICODE_3_BYTES) == PTSD_UNICODE_2_BYTES)
         return 2;
    else return 1;
#undef PTSD_UNICODE_MASK
#undef PTSD_UNICODE_4_BYTES
#undef PTSD_UNICODE_3_BYTES
#undef PTSD_UNICODE_2_BYTES
}

PTSD_UNUSED
static inline usize punicode_to_utf8(u32 codepoint, char result[4]) {
    memset(result, 0, 4);

    if (codepoint <= 0x7f) {
        result[0] = (char)codepoint;
        return 1;
    }
    
    if (codepoint <= 0x7ff) {
        result[0] = (char)(0xc0 | (codepoint >> 6));
        result[1] = (char)(0x80 | (codepoint & 0x3f));
        return 2;
    }
    
    if (codepoint <= 0xffff) {
        result[0] = (char)(0xe0 | ((codepoint >> 12)));
        result[1] = (char)(0x80 | ((codepoint >>  6) & 0x3f));
        result[2] = (char)(0x80 | ((codepoint) & 0x3f));
        return 3;
    }
    
    if (codepoint <= 0x10ffff) {
        result[0] = (char)(0xf0 | ((codepoint >> 18)));
        result[1] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
        result[2] = (char)(0x80 | ((codepoint >>  6) & 0x3f));
        result[3] = (char)(0x80 | ((codepoint) & 0x3f));
        return 4;
    }
    return 0;
}


#if defined(PTSD_MSVC)
void pinitialize_std_stream(void);
void pdestroy_std_stream(void);

static void pmsvc_initialize(void);
#pragma section(".CRT$XCT", read)
__declspec(allocate(".CRT$XCT"))
void (*pmsvc_initialize_var)(void) = pmsvc_initialize;
static void pmsvc_initialize(void) {
    atexit(pdestroy_std_stream);
    pinitialize_std_stream();
}
#else
void pinitialize_std_stream(void) __attribute__((constructor));
void pdestroy_std_stream(void)    __attribute__((destructor));
#endif
// don't know if these need to be in the header file

//TODO: explain what all the vairables mean, maybe write an example
typedef struct pprintf_info_t pprintf_info_t;
struct pprintf_info_t {
    pgeneric_stream_t *stream; 
    const char *restrict fmt;
    va_list *list;
    u32 count; 
    bool *failflag;
};

enum pformatting_length_t {
    PFL_DEFAULT,
    PFL_HH,PFL_H,
    PFL_L, PFL_LL,
    PFL_J, PFL_Z, PFL_T, PFL_128 /* uppercase L */,
};

typedef struct pformatting_specification_t pformatting_specification_t;
struct pformatting_specification_t {
    bool right_justified;
    u32 justification_count;
    u32 zero_justification_count;
    bool prefix_zero;
    bool force_sign;
    enum pformatting_length_t length;
    bool alternative_form;
};

typedef void pformat_callback_t(pprintf_info_t*); 
typedef void pformat_callback_adv_t(pprintf_info_t*, pformatting_specification_t*); 

void pformat_push_impl(pstring_t fmt, pformat_callback_t *callback);
void pformat_push_adv_Impl(pstring_t fmt, pformat_callback_adv_t *callback);

void pformat_pop_impl(pstring_t fmt);
void pformat_pop_adv_impl(pstring_t fmt);

#if PTSD_C_VERSION > PTSD_C11
#define pwrite_stream(stream, ...) _Generic(__VA_ARGS__, int: pwrite_stream_c, \
    char: pwrite_stream_c, pstring_t: pwrite_stream_s)(stream, __VA_ARGS__)
#elif defined(__cplusplus)
}
#endif

#define pformat_push(fmt, callback) pformat_push_impl(pcreate_string(fmt), callback)
#define pformat_push_adv(fmt, callback) pformat_push_impl(pcreate_string(fmt), callback)
#define pformat_pop(fmt) pformat_pop_impl(pcreate_string(fmt))
#define pformat_pop_adv(fmt) pformat_pop_adv_impl(pcreate_string(fmt))

#if defined(PTSD_MSVC)
#   if defined(PTSD_MSVC_MAIN)
#       if PTSD_MSVC_MAIN == 0
            extern int fake_main(void);
            int main(void) { 
                pinitialize_std_stream();
                int value = fake_main(); 
                pdestroy_std_stream();
                return value;
            }
#       elif PTSD_MSVC_MAIN == 3
            extern int fake_main(int argc, char **argv, char **envp);
            int main(int argc, char **argv, char **envp) { 
                pinitialize_std_stream();
                int value = fake_main(argc, argv, envp);
                pdestroy_std_stream();
                return value;
            }
#       elif PTSD_MSVC_MAIN == 2
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                pinitialize_std_stream();
                int value = fake_main(argc, argv);
                pdestroy_std_stream();
                return value;
            }
#       else
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                pinitialize_std_stream();
                int value = fake_main(argc, argv);
                pdestroy_std_stream();
                return value;
            }
#       endif
#   endif
#define main fake_main
#endif



#endif // PTSD_PIO_HEADER
