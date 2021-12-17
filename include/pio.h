#pragma once
#ifndef PSTD_PIO_HEADER
#define PSTD_PIO_HEADER

#include <stdarg.h>

#include "general.h"
#include "pstring.h"
#include "pplatform.h"

#if defined(PSTD_MSVC)
#pragma message when using pio with msvc compiler you need to define PSTD_MSVC_MAIN and include pio.h in the file where 'main' is defined
#define PSTD_PIO_CONSTRUCTOR
#define PSTD_PIO_DESTRUCTOR
#else
#define PSTD_PIO_CONSTRUCTOR __attribute__(( constructor ))
#define PSTD_PIO_DESTRUCTOR  __attribute__(( destructor ))
#endif

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
    |generic        | this is not a specifier but the pPrintf   | 
    |               | functions allows for custom specifiers    |
    |               | this is done through the `pFormatPush`    |
    |               | and `pFormatPop` functions.               |
    |               | see examples/formatting.c for usage       |
    |-----------------------------------------------------------|
*/


/*
    pScanf extra format specifiers
    
    |-----------------------------------------------------------|
    |S              | same as '%s' but also returns the length  |
    |               | note that this will allocate space        |
    |               | for the string on the heap                |
    |-----------------------------------------------------------|
    |b              | reads a binary number and                 |
    |               | returns the bits,                         |
    |               | note that if length modifier is not       |
    |               | present it will expect a usize.           |
    |               | if length is specified (at most 'll')     |
    |               | then it uses an integer big enough        |
    |               | to store that size                        |
    |-----------------------------------------------------------|
    |S[set]         | pstring_t version of [set]                   |
    |-----------------------------------------------------------|

    note that the uppercase 'S' variants don't support 'l' modifier

    '-' has been implemented as a range modifier in [set]
    if '-' is the last character it is seen as being a character literal
    for example the set '[-]' would only match '-'
*/

/*  IDEA: (not implemented)
    
    pregex()
    
*/



// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 
typedef struct pgeneric_stream_t pgeneric_stream_t;
struct pgeneric_stream_t {
    pbool_t is_valid;
#if defined(PSTD_USE_ALLOCATOR)
    pallocator_t cb;
#endif

    enum pstream_type_t {
        STANDARD_STREAM,
        FILE_STREAM,
        STRING_STREAM,
        CFILE_STREAM,
    } type;
    enum pstream_flags_t {
        STREAM_OUTPUT = 0b01U,
        STREAM_INPUT  = 0b10U,
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
        struct { // c stream
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
    enum pstream_type_t type;
#if defined(PSTD_USE_ALLOCATOR)
    pallocator_t cb;
#else
#endif

    u32 flags;
    // filestream | stringstream
    usize buffersize;
    // filestream
    char *filename;
    pbool_t createbuffer;
    pbool_t createifnonexistent;
    pbool_t append;
};

void pset_stream(pgeneric_stream_t *new_stream, pgeneric_stream_t *old_stream);
pgeneric_stream_t *pget_stream(void);


pgeneric_stream_t pinit_stream(pstream_info_t info);
void pfree_stream(pgeneric_stream_t *stream);

pstring_t pstream_to_buffer_string(pgeneric_stream_t *stream);

u32 pvbprintf(pgeneric_stream_t *stream, const char *restrict fmt, va_list list);

PSTD_UNUSED
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

PSTD_UNUSED
static inline u32 pprintf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pvprintf(fmt, list);
    va_end(list);
    return result;
}

u32 pvbscanf(pgeneric_stream_t *stream, const char *restrict fmt, va_list list);

PSTD_UNUSED
static u32 pBScanf(pgeneric_stream_t *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pvbscanf(stream, fmt, list);
    va_end(list);
    return result;
}

static inline u32 pvscanf(const char *restrict fmt, va_list list ) {
    return pvbscanf(pget_stream(), fmt, list);
}

PSTD_UNUSED
static inline u32 pscanf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pvscanf(fmt, list);
    va_end(list);
    return result;
}

void pstream_write_string(pgeneric_stream_t *stream, const pstring_t str);
void pstream_write_char(pgeneric_stream_t *stream, const char chr);

// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void  pstream_read(pgeneric_stream_t *stream, void *buf, usize size);

#ifndef PSTD_LINUX
PSTD_UNUSED
static inline void pread(void *buf, usize size) {
    pstream_read(pget_stream(), buf, size);
}
#else
static inline void pio_read(void *buf, usize size) {
    pstream_read(pget_stream(), buf, size);
}
#endif

// line needs to be freed
pstring_t pstream_read_line(pgeneric_stream_t *stream);

PSTD_UNUSED
static inline pstring_t pread_line(void) {
    return pstream_read_line(pget_stream());
}


// move the stream pointer forwards or backwards
// size is how many bytes to move the stream pointer
//
// so if the stream pointer points here
// example text that the stream holds
// ----------------^
// 
// and you wan't to move it back 3 bytes
// you would do StreamMove(stream, -3)
// and now the pointer points here instead
// example text that the stream holds
// -------------^
//
//
void pstream_move(pgeneric_stream_t *stream, isize size);

// pSigned**Topstring_t appends + or - to the start of buffer then calls pUnsigned**Topstring_t

u32 psigned_int_to_string(char *buf, s64 num, u32 radix,   const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);
u32 punsigned_int_to_string(char *buf, u64 num, u32 radix, const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);

u32 psigned_decimal_to_string(char *buf, s64 num);
u32 punsigned_decimal_to_string(char *buf, u64 num);

u32 psigned_hex_to_string(char *buf, s64 num);
u32 punsigned_hex_to_string(char *buf, u64 num);

u32 psigned_octal_to_string(char *buf, s64 num);
u32 punsigned_octal_to_string(char *buf, u64 num);

u32 pftoa(char *buf, f32);
u32 pdtoa(char *buf, f64);


#if defined(PSTD_GNU_COMPATIBLE)
PSTD_UNUSED
static inline pbool_t pchar_anyof(int character, u32 count, const char tests[count]) {
#else
PSTD_UNUSED
static inline pbool_t pchar_anyof(int character, u32 count, const char tests[]) {
#endif
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

PSTD_UNUSED
static inline usize pget_utf8_length(const char *chr) {
#define UNICODE_MASK 0xf8 
#define UNICODE_4_BYTES 0xf0
#define UNICODE_3_BYTES 0xe0
#define UNICODE_2_BYTES 0xc0
    if ((chr[0] & UNICODE_MASK) == UNICODE_4_BYTES)
         return 4;
    else if ((chr[0] & UNICODE_4_BYTES) == UNICODE_3_BYTES)
         return 3;
    else if ((chr[0] & UNICODE_3_BYTES) == UNICODE_2_BYTES)
         return 2;
    else return 1;
#undef UNICODE_MASK
#undef UNICODE_4_BYTES
#undef UNICODE_3_BYTES
#undef UNICODE_2_BYTES
}





// don't know if these need to be in the header file
void pinitialize_std_stream(void) PSTD_PIO_CONSTRUCTOR;
void pdestroy_std_stream(void)    PSTD_PIO_DESTRUCTOR;

typedef struct pprintf_info_t pprintf_info_t;
struct pprintf_info_t {
    pgeneric_stream_t *stream; 
    const char *restrict fmt;
    va_list *list;
    u32 count; 
    pbool_t *failflag;
};

enum pformatting_length_t {
    PFL_DEFAULT,
    PFL_HH,PFL_H,
    PFL_L, PFL_LL,
    PFL_J, PFL_Z, PFL_T, PFL_128 /* uppercase L */,
};

typedef struct pformatting_specification_t pformatting_specification_t;
struct pformatting_specification_t {
    pbool_t right_justified;
    u32 justification_count;
    u32 zero_justification_count;
    pbool_t prefix_zero;
    pbool_t force_sign;
    enum pformatting_length_t length;
    pbool_t alternative_form;
};

typedef void pformat_callback_t(pprintf_info_t*); 
typedef void pformat_callback_adv_t(pprintf_info_t*, pformatting_specification_t*); 

void pformat_push_impl(pstring_t fmt, pformat_callback_t *callback);
void pformat_push_adv_Impl(pstring_t fmt, pformat_callback_adv_t *callback);

void pformat_pop_impl(pstring_t fmt);
void pformat_pop_adv_impl(pstring_t fmt);

#if defined(PSTD_C11)
#define pstream_write(stream, ...) _Generic(__VA_ARGS__, int: pstream_write_char, \
    char: pstream_write_char, pstring_t: pstream_write_string)(stream, __VA_ARGS__)

#elif defined(__cplusplus)
}
#endif

#define pformat_push(fmt, callback) pformat_push_impl(pcreate_string(fmt), callback)
#define pformat_push_adv(fmt, callback) pformat_push_impl(pcreate_string(fmt), callback)
#define pformat_pop(fmt) pformat_pop_impl(pcreate_string(fmt))
#define pformat_pop_adv(fmt) pformat_pop_adv_impl(pcreate_string(fmt))

#if defined(PSTD_MSVC)
#   if defined(PSTD_MSVC_MAIN)
#       if PSTD_MSVC_MAIN == 0
            extern int fake_main(void);
            int main(void) { 
                pinitialize_std_stream();
                int value = fake_main(); 
                pdestroy_std_stream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 3
            extern int fake_main(int argc, char **argv, char **envp);
            int main(int argc, char **argv, char **envp) { 
                pinitialize_std_stream();
                int value = fake_main(argc, argv, envp);
                pdestroy_std_stream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 2
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



#endif // PSTD_PIO_HEADER
