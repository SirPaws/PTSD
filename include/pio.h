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
    |S              | prints a String (char *, with length)     |
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
    |S[set]         | String version of [set]                   |
    |-----------------------------------------------------------|

    note that the uppercase 'S' variants don't support 'l' modifier

    '-' has been implemented as a range modifier in [set]
    if '-' is the last character it is seen as being a character literal
    for example the set '[-]' would only match '-'
*/

/*  IDEA: (not implemented)
    
    pRegex()
    
*/



// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 
typedef struct GenericStream GenericStream;
struct GenericStream {
    pbool_t is_valid;
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb;
#endif

    enum StreamType {
        STANDARD_STREAM,
        FILE_STREAM,
        STRING_STREAM,
        CFILE_STREAM,
    } type;
    enum StreamFlags {
        STREAM_OUTPUT = 0b01U,
        STREAM_INPUT  = 0b10U,
    } flags;
    union {
        struct { // String stream
            char  *stretchy buffer;
            usize cursor;
        };
        struct { // File stream
            pHandle *handle;
            usize size;
            String file_buffer;
        };
        struct { // Standard stream
            pHandle *stdout_handle;
            pHandle *stdin_handle;
        };
        struct { // c stream
            FILE *file;
        };
    };
};
typedef GenericStream StringStream; 
typedef GenericStream FileStream;
typedef GenericStream StdStream;
typedef GenericStream cFileStream;

typedef struct StreamInfo StreamInfo;
struct StreamInfo {
    enum StreamType type;
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb;
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

void pSetStream(GenericStream *new_stream, GenericStream *old_stream);
GenericStream *pGetStream(void);


GenericStream pInitStream(StreamInfo info);
void pFreeStream(GenericStream *stream);

String pStreamToBufferString(GenericStream *stream);

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list);

PSTD_UNUSED
static u32 pBPrintf(GenericStream *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf(stream, fmt, list);
    va_end(list);
    return result;
}

static inline u32 pVPrintf(const char *restrict fmt, va_list list ) {
    return pVBPrintf(pGetStream(), fmt, list);
}

PSTD_UNUSED
static inline u32 pPrintf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVPrintf(fmt, list);
    va_end(list);
    return result;
}

u32 pVBScanf(GenericStream *stream, const char *restrict fmt, va_list list);

PSTD_UNUSED
static u32 pBScanf(GenericStream *stream, const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBScanf(stream, fmt, list);
    va_end(list);
    return result;
}

static inline u32 pVScanf(const char *restrict fmt, va_list list ) {
    return pVBScanf(pGetStream(), fmt, list);
}

PSTD_UNUSED
static inline u32 pScanf(const char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVScanf(fmt, list);
    va_end(list);
    return result;
}

void pStreamWriteString(GenericStream *stream, String str);
void pStreamWriteChar(GenericStream *stream, char chr);

// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void  pStreamRead(GenericStream *stream, void *buf, usize size);

PSTD_UNUSED
static inline void pRead(void *buf, usize size) {
    pStreamRead(pGetStream(), buf, size);
}

// line needs to be freed
String pStreamReadLine(GenericStream *stream);

PSTD_UNUSED
static inline String pReadLine(void) {
    return pStreamReadLine(pGetStream());
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
void pStreamMove(GenericStream *stream, isize size);

// pSigned**ToString appends + or - to the start of buffer then calls pUnsigned**ToString

u32 pSignedIntToString(char *buf, s64 num, u32 radix,   const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);
u32 pUnsignedIntToString(char *buf, u64 num, u32 radix, const char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]);

u32 pSignedDecimalToString(char *buf, s64 num);
u32 pUnsignedDecimalToString(char *buf, u64 num);

u32 pSignedHexToString(char *buf, s64 num);
u32 pUnsignedHexToString(char *buf, u64 num);

u32 pSignedOctalToString(char *buf, s64 num);
u32 pUnsignedOctalToString(char *buf, u64 num);



u32 pFtoa(char *buf, f32);
u32 pDtoa(char *buf, f64);


#if defined(PSTD_GNU_COMPATIBLE)
PSTD_UNUSED
static inline pbool_t pCharAnyOf(int character, u32 count, const char tests[count]) {
#else
PSTD_UNUSED
static inline pbool_t pCharAnyOf(int character, u32 count, const char tests[]) {
#endif
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

PSTD_UNUSED
static inline usize pGetUtf8Length(const char *chr) {
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
void InitializeStdStream(void) PSTD_PIO_CONSTRUCTOR;
void DestroyStdStream(void)    PSTD_PIO_DESTRUCTOR;

typedef struct pPrintfInfo pPrintfInfo;
struct pPrintfInfo {
    GenericStream *stream; 
    const char *restrict fmt;
    va_list list;
    u32 count; 
    pbool_t *failflag;
};

enum pFormattingLength {
    PFL_DEFAULT,
    PFL_HH,PFL_H,
    PFL_L, PFL_LL,
    PFL_J, PFL_Z, PFL_T, PFL_128 /* uppercase L */,
};

typedef struct pFormattingSpecification pFormattingSpecification;
struct pFormattingSpecification {
    pbool_t right_justified;
    u32 justification_count;
    u32 zero_justification_count;
    pbool_t prefix_zero;
    pbool_t force_sign;
    enum pFormattingLength length;
    pbool_t alternative_form;
};

typedef void FormatCallback(pPrintfInfo*); 
typedef void FormatCallbackAdv(pPrintfInfo*, pFormattingSpecification*); 

void pFormatPushImpl(String fmt, FormatCallback *callback);
void pFormatPushAdvImpl(String fmt, FormatCallbackAdv *callback);

void pFormatPopImpl(String fmt);
void pFormatPopAdvImpl(String fmt);

#if defined(PSTD_C11)
#define pStreamWrite(stream, ...) _Generic(__VA_ARGS__, int: pStreamWriteChar, \
    char: pStreamWriteChar, String: pStreamWriteString)(stream, __VA_ARGS__)

#elif defined(__cplusplus)
}
#endif

#define pFormatPush(fmt, callback) pFormatPushImpl(pCreateString(fmt), callback)
#define pFormatPushAdv(fmt, callback) pFormatPushAdvImpl(pCreateString(fmt), callback)
#define pFormatPop(fmt) pFormatPopImpl(pCreateString(fmt))
#define pFormatPopAdv(fmt) pFormatPopAdvImpl(pCreateString(fmt))

#if defined(PSTD_MSVC)
#   if defined(PSTD_MSVC_MAIN)
#       if PSTD_MSVC_MAIN == 0
            extern int fake_main(void);
            int main(void) { 
                InitializeStdStream();
                int value = fake_main(); 
                DestroyStdStream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 3
            extern int fake_main(int argc, char **argv, char **envp);
            int main(int argc, char **argv, char **envp) { 
                InitializeStdStream();
                int value = fake_main(argc, argv, envp);
                DestroyStdStream();
                return value;
            }
#       elif PSTD_MSVC_MAIN == 2
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                InitializeStdStream();
                int value = fake_main(argc, argv);
                DestroyStdStream();
                return value;
            }
#       else
            extern int fake_main(int argc, char **argv);
            int main(int argc, char **argv) { 
                InitializeStdStream();
                int value = fake_main(argc, argv);
                DestroyStdStream();
                return value;
            }
#       endif
#   endif
#define main fake_main
#endif



#endif // PSTD_PIO_HEADER
