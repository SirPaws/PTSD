#pragma once
#ifndef PSTD_PIO_HEADER
#define PSTD_PIO_HEADER

#include "pstring.h"
#include "dynarray.h"
#include "pplatform.h"
#pragma clang diagnostic ignored "-Wclass-varargs"

#define CAST_STREAM(stream)                      \
    _Generic((stream),                           \
        GenericStream *:(GenericStream*)(stream),\
        StdStream *:    (GenericStream*)(stream),\
        FileStream *:   (GenericStream*)(stream),\
        StringStream *: (GenericStream*)(stream) \
    )


enum StreamFlags {
    STREAM_OUTPUT = 0b01,
    STREAM_INPUT  = 0b10,
};

enum StreamType {
    STANDARD_STREAM,
    FILE_STREAM,
    STRING_STREAM
};

// TODO: CreateFile dwShareMode
// TODO: CreateFile lpSecurityAttributes 

typedef struct StringStream  StringStream;
struct StringStream {
    enum StreamType  type;
    u32 flags;
    pCreateDynArray(StringStreamBuffer, char) buffer;
    usize cursor;
    // maybe more if not we just expose stringstream
};

typedef struct FileStream FileStream;
struct FileStream {
    enum StreamType type;
    u32 flags;
    pHandle *handle;
    usize size;
    String buffer;
};

typedef struct StdStream StdStream;
struct StdStream {
    enum StreamType type;
    u32 flags;
    pHandle *stdout_handle;
    pHandle *stdin_handle;
};

typedef struct GenericStream GenericStream;
struct GenericStream {
    enum StreamType type;
    u32 flags;
    void *data;
};

typedef struct StreamInfo StreamInfo;
struct StreamInfo {
    enum StreamType type;
    u32 flags;
    // filestream | stringstream
    usize buffersize;
    // filestream
    char *filename;
    bool createbuffer;
    bool createifnonexistent;
    bool append;
};

GenericStream *pSetStream(GenericStream *stream);
#define pSetStream(stream) pSetStream(CAST_STREAM(stream))
GenericStream *pGetStream(void);


GenericStream *pInitStream(StreamInfo info);
void pFreeStream(GenericStream *stream);
#define pFreeStream(stream) pFreeStream(CAST_STREAM(stream))

String pStreamToBufferString(GenericStream *stream);
#define pStreamToBufferString(stream) pStreamToBufferString(CAST_STREAM(stream))

u32 pVBPrintf(GenericStream *stream, char *restrict fmt, va_list list);
#define pVBPrintf(stream, fmt, list) pVBPrintf(CAST_STREAM(stream), fmt, list)

[[maybe_unused]]
static u32 pBPrintf(GenericStream *stream, char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVBPrintf(stream, fmt, list);
    va_end(list);
    return result;
}
#define pBPrintf(stream, fmt, ...) pBPrintf(CAST_STREAM(stream), fmt, ## __VA_ARGS__)

static inline u32 pVPrintf(char *restrict fmt, va_list list ) {
    return pVBPrintf(pGetStream(), fmt, list);
}

PSTD_MAYBE_UNUSED
static inline u32 pPrintf(char *restrict fmt, ...) {
    va_list list;
    va_start(list, fmt);
    u32 result = pVPrintf(fmt, list);
    va_end(list);
    return result;
}

void StreamWriteString(GenericStream *stream, String str);
void StreamWriteChar(GenericStream *stream, char chr);
#define StreamWrite(stream, ...) _Generic(__VA_ARGS__, int: StreamWriteChar, \
    char: StreamWriteChar, String: StreamWriteString)(CAST_STREAM(stream), __VA_ARGS__)


#define StreamWriteString(stream, str) StreamWriteString(CAST_STREAM(stream), str)
#define StreamWriteChar(stream, chr) StreamWriteChar(CAST_STREAM(stream), chr)

// size: how many bytes to read from stream
// eof: (if null it's ignored) set to true if the stream is at the end 
void StreamRead(GenericStream *stream, void *buf, usize size);
#define StreamRead(stream, buf, str) StreamRead(CAST_STREAM(stream), buf, str)

PSTD_MAYBE_UNUSED
static inline void pRead(void *buf, usize size) {
    StreamRead(pGetStream(), buf, size);
}

// line needs to be freed
String StreamReadLine(GenericStream *stream);
#define StreamReadLine(stream) StreamReadLine(CAST_STREAM(stream))

PSTD_MAYBE_UNUSED
static inline String pReadLine(void) {
    return StreamReadLine(pGetStream());
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
void StreamMove(GenericStream *stream, isize size);
#define StreamMove(stream, size) StreamMove(CAST_STREAM(stream), size)

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








// don't know if these need to be in the header file
void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

typedef struct pPrintfInfo pPrintfInfo;
struct pPrintfInfo {
    GenericStream *stream; 
    char *restrict fmt;
    va_list list;
    u32 count; 
    bool *failflag;
};

enum pFormattingLength {
    PFL_DEFAULT,
    PFL_HH,PFL_H,
    PFL_L, PFL_LL,
    PFL_J, PFL_Z, PFL_T, PFL_128 /* uppercase L */,
};

typedef struct pFormattingSpecification pFormattingSpecification;
struct pFormattingSpecification {
    bool right_justified;
    u32 justification_count;
    u32 zero_justification_count;
    bool prefix_zero;
    bool force_sign;
    enum pFormattingLength length;
    bool alternative_form;
};

typedef pPrintfInfo FormatCallback(pPrintfInfo); 
typedef pPrintfInfo FormatCallbackAdv(pPrintfInfo, pFormattingSpecification); 

void pFormatPushImpl(String fmt, FormatCallback *callback);
void pFormatPushAdvImpl(String fmt, FormatCallbackAdv *callback);
#define pFormatPush(fmt, callback) pFormatPushImpl(pCreateString(fmt), callback)
#define pFormatPushAdv(fmt, callback) pFormatPushAdvImpl(pCreateString(fmt), callback)

void pFormatPopImpl(String fmt);
void pFormatPopAdvImpl(String fmt);
#define pFormatPop(fmt) pFormatPopImpl(pCreateString(fmt))
#define pFormatPopAdv(fmt) pFormatPopAdvImpl(pCreateString(fmt))




/*
size = snprintf(NULL, 0, FORMAT, ...);
char buf[size + 1];
sprintf(buf, FORMAT, ...);

MY LIB:

StringStream *ss   = CreateStringStream(0);
GenericStrema *old = SetStream(ss);
pPrintf(FORMAT, ...);
SetStream(old);

--- OR ---

StringStream *ss   = CreateStringStream(0);
pBPrintf(ss, FORMAT, ...);






*/

#endif // PSTD_PIO_HEADER
