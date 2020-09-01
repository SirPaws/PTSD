#include "pio.h"
#include "pplatform.h"
#if defined(PSTD_WINDOWS)
#include <Windows.h>
#endif

#include <math.h>

#include "allocator.h"
#include "vector.h"

#define Break ({ break; })

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

typedef pCreateVectorStruct(StringStreamBuffer, char *) StringStreamBuffer;

struct StringStream {
    enum StreamType  type;
    u32 flags;
    StringStreamBuffer *buffer;
    usize cursor;
    // maybe more if not we just expose stringstream
};

struct FileStream {
    enum StreamType  type;
    u32 flags;
    pHandle *handle;
    usize size;
    String buffer;
};

struct StdStream {
    enum StreamType  type;
    u32 flags;
    pHandle *stdout_handle;
    pHandle *stdin_handle;
};



struct GenericStream {
    enum StreamType  type;
    u32 flags;
    void *data;
};

void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

static StdStream *StandardStream = NULL;
static u32 default_code_page = 0;
void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

static GenericStream *pcurrentstream;

GenericStream *pSetStream(GenericStream *stream) {
    assert(stream);
    GenericStream *output = pcurrentstream;
    pcurrentstream = stream;
    return output;
}
GenericStream *pGetStream(void) { return pcurrentstream; }

void InitializeStdStream(void) {
#if defined(PSTD_WINDOWS)
    // this is so we can print unicode characters on windows
    default_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    const StdStream template = { 
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = pGetSTDOutHandle(),
        .stdin_handle  = pGetSTDInHandle()
    };

    StandardStream = malloc(sizeof *StandardStream);
    pcurrentstream = (void *)StandardStream;
    memcpy(StandardStream, &template, sizeof template);

}
void DestroyStdStream(void) {
    free(StandardStream);
#if defined(PSTD_WINDOWS)
    SetConsoleOutputCP(default_code_page);
#endif
}

GenericStream *pInitStream(StreamInfo info) {
    switch(info.type) {
        case STANDARD_STREAM: {
                StdStream *std = pCurrentAllocatorFunc(NULL, sizeof *std, 0, MALLOC, pCurrentAllocatorUserData);
                memcpy(std, StandardStream, sizeof *std);
                std->flags = (u32)info.flags; 

                if ((info.flags & STREAM_INPUT) == 0)
                    std->stdin_handle = pNullHandle();
                if ((info.flags & STREAM_OUTPUT) == 0)
                    std->stdout_handle = pNullHandle();
                return (GenericStream *)std;
            }
        case FILE_STREAM: {
                pFileStat stat = pGetFileStat(info.filename);
                bool result = stat.exists;
                u64 filesize = stat.filesize;

                FileStream *fstream = NULL;
                if (result == false){
                    if (info.createifnonexistent) {
                        fstream = pCurrentAllocatorFunc(NULL, 
                                sizeof *fstream, 0, MALLOC, pCurrentAllocatorUserData);
                        memset(fstream, 0, sizeof *fstream);
                        fstream->type = FILE_STREAM;
                        fstream->flags = info.flags;
                        fstream->handle = pFileCreate(info.filename, info.flags);
                        fstream->size   = 0;
                    }
                    else return NULL;
                } else {
                    fstream = pCurrentAllocatorFunc(NULL, 
                            sizeof *fstream, 0, MALLOC, pCurrentAllocatorUserData);
                    memset(fstream, 0, sizeof * fstream);
                    fstream->type = FILE_STREAM;
                    fstream->flags = info.flags;
                    fstream->size  = filesize;
                    fstream->handle = pFileOpen(info.filename, info.flags);
                }
                if (info.createbuffer) pStreamToBufferString((void *)fstream);
                return (void *)fstream;
            }
        case STRING_STREAM: {
                StringStream *sstream = pCurrentAllocatorFunc(NULL, 
                            sizeof *sstream, 0, MALLOC, pCurrentAllocatorUserData);
                memset(sstream, 0, sizeof *sstream); 
                sstream->type  = STRING_STREAM; 
                sstream->flags = info.flags; 
                VectorInfo vinfo = { .datasize = sizeof(char), .initialsize = info.buffersize };
                sstream->buffer = (void *)pInitVector(vinfo);
                return (void *)sstream;
            }
    }
    return NULL;
}

void pFreeStream(GenericStream *stream) {
    if (!stream) return;
    StdStream    *stdstream;
    FileStream   *fstream;
    StringStream *sstream;
    stdstream = (void *)stream;
    fstream   = (void *)stream;
    sstream   = (void *)stream;
    switch(stream->type) {
        case STANDARD_STREAM: break;
        case FILE_STREAM:
            pFileClose(fstream->handle);
            if (fstream->buffer.length) 
                pCurrentAllocatorFunc(fstream->buffer.c_str, 0, 0, FREE, pCurrentAllocatorUserData);
            break;
        case STRING_STREAM:
            pFreeVector((void *)sstream->buffer);
    }
    pCurrentAllocatorFunc(stream, 0, 0, FREE, pCurrentAllocatorUserData);
}

String pStreamToBufferString(GenericStream *stream) {
    assert(stream->type != STANDARD_STREAM);

    if (stream->type == STRING_STREAM) {
        StringStream *sstream = (void *)stream;
        return (String) { 
            pVectorBegin((GenericVector *)sstream->buffer), 
            pVectorSize((GenericVector *)sstream->buffer)
        };
    } else {
        FileStream *fstream = (void *)stream; 
        if (fstream->buffer.length == 0) {
            u8 *tmp = pCurrentAllocatorFunc(0, fstream->size, 0, MALLOC, pCurrentAllocatorUserData);
            String buf = pString(tmp, fstream->size);
            pFileRead(fstream->handle, buf);
            fstream->buffer = buf;
        }
        return fstream->buffer;
    }
}

#define expect(x, value) __builtin_expect(x, value)

void StreamRead(GenericStream *stream, void *buf, usize size) {
    assert(stream);
    assert(stream->flags & STREAM_INPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        StdStream *stdstream = (void *)stream;
        // this should probably have a comment explaning why we do this
        pHandle *handle = stream->type == STANDARD_STREAM ? stdstream->stdin_handle : stdstream->stdout_handle;  
        bool result = pFileRead(handle, (String){ buf, size });
        if (result == false) memset(buf, 0, size);
    } else {
        StringStream *sstream = (void *)stream;
        if (sstream->cursor + size >= pVectorSize((void *)sstream->buffer)) {
            memset(buf, 0, size); return;
        }
        StringStreamBuffer *buffer = sstream->buffer;
        memcpy(buf, buffer->data + sstream->cursor, size);
    }
}

void StreamWriteString(GenericStream *stream, String str) {
    assert(stream);
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        pFileWrite(fstream->handle, str);
    } else {
        StringStream *sstream = (StringStream *)stream;
        StringStreamBuffer *buffer = sstream->buffer;
        buffer->datasize = str.length;
        pVectorInsert((GenericVector **)&sstream->buffer, buffer->data + sstream->cursor, str.c_str);
        buffer->datasize = 1;
    }
}

void StreamWriteChar(GenericStream *stream, char str) {
    assert(stream);
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        pFileWrite(fstream->handle, (String){ (u8*)&str, 1 });
    } else {
        StringStream *sstream = (StringStream *)stream;
        StringStreamBuffer *buffer = sstream->buffer;
        pVectorInsert((GenericVector **)&sstream->buffer, buffer->data + sstream->cursor, &str);
    }
}

static bool IsCharacters(int character, u32 count, char tests[count]){
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count);

void GetRGB(char *restrict* fmt, String RGB[3]);
u32  GetUnicodeLength(const char *chr);

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
    u8 justificiation_chars[2];
    bool justification_char_is_zero;
    enum pFormattingLength length;
};

struct BinaryStringReturn {
    u8 *buffer;
    String str;
    bool iszero;
};

struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num);
struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num) {
    struct BinaryStringReturn ret = { 0 };

    u64 bit = 1ULL << (bitcount - 1);
    ret.buffer = pCurrentAllocatorFunc(NULL, bitcount, 0, MALLOC, pCurrentAllocatorUserData);
    for (u64 i = 0; i < bitcount; i++) {
       ret.buffer[i] = (u8)'0' + ((num & bit) ? 1 : 0); 
       bit >>= 1;
    }

    ret.str.c_str = ret.buffer;
    s32 count = (s32)bitcount;
    while(*ret.str.c_str == '0') { ret.str.c_str++; count--; }
    if (count <= 0) { 
        ret.iszero = true;
        ret.str.c_str -= 1; 
        count = 1; 
    }
    ret.str.length = (usize)count; 
    return ret;
}


typedef struct pPrintfInfo pPrintfInfo;
struct pPrintfInfo {
    GenericStream *stream; 
    char *restrict fmt;
    va_list list;
    u32 count; 
    bool *failflag;
};


pPrintfInfo pHandleLength(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandlePlus(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleMinus(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleSpace(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleZero(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleHash(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleDot(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleNumber(pPrintfInfo,pFormattingSpecification);

pPrintfInfo pHandleChar(pPrintfInfo,   bool wide);
pPrintfInfo pHandleString(pPrintfInfo, pFormattingSpecification, bool cstr);
pPrintfInfo pHandleInt(pPrintfInfo,    pFormattingSpecification, char printtype, bool always_print_sign);
pPrintfInfo pHandleFloat(pPrintfInfo,  pFormattingSpecification);
pPrintfInfo pHandlePointer(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleCharatersWritten(pPrintfInfo, pFormattingSpecification);



pPrintfInfo pHandleBinary(pPrintfInfo,pFormattingSpecification);
pPrintfInfo pHandleBackgroundColor(pPrintfInfo);
pPrintfInfo pHandleForegroundColor(pPrintfInfo);
pPrintfInfo pHandleColorClear(pPrintfInfo);

u32 pVBPrintf(GenericStream *stream, char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (expect(*fmt != '%', 1)) {
            char *restrict fmt_next = fmt;
            while (IsCharacters(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) fmt_next++;
            StreamWrite(pcurrentstream, (String){ (u8 *)fmt, (usize)(fmt_next - fmt)});
            fmt = fmt_next;
        } else {
            bool failed = false;
            struct pFormattingSpecification jinfo = {
                .right_justified    = false,
                .justification_count = 0,
                .justificiation_chars = { u' ', u'0' },
                .justification_char_is_zero  = false,
                .length = PFL_DEFAULT,
            };
            char *restrict fmt_next = fmt + 1;
            pPrintfInfo pinfo = {
                .stream = stream,
                .fmt = fmt_next,
                .list = list,
                .count = printcount,
                .failflag = &failed, 
            };

            #define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            pPrintfInfo tmp;
            switch(*fmt_next) {
                case 'n': tmp = pHandleCharatersWritten(pinfo, jinfo); break;
                case '-': tmp = pHandleMinus(pinfo, jinfo); break; 
                case '+': tmp = pHandlePlus(pinfo, jinfo); break;
                case '0': tmp = pHandleZero(pinfo, jinfo); break;
                case '#': tmp = pHandleHash(pinfo, jinfo); break;
                case '.': tmp = pHandleDot(pinfo, jinfo); break;
                case ' ': tmp = pHandleSpace(pinfo, jinfo); break;

                case '1': case '2':
                case '3': case '4':
                case '5': case '6':
                case '7': case '8':
                case '9': tmp = pHandleNumber(pinfo, jinfo); break;
                         
                case 'h': case 'l':
                case 'j': case 'z': 
                case 't': case 'L': pHandleLength(pinfo, jinfo);
                
                case 's': case 'S': tmp = pHandleString(pinfo, jinfo, *fmt_next == 's' ? true : false); break;
                
                case 'u': case 'o': 
                case 'x': case 'X':
                case 'i': case 'd':  tmp = pHandleInt(pinfo, jinfo, *fmt_next, false); break;
                case 'b': tmp = pHandleBinary(pinfo, jinfo); break;
                case 'c': tmp = pHandleChar(pinfo, false);   break;
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0)       tmp = pHandleColorClear(pinfo), Break;
                        else if (memcmp(fmt_next, "Cbg", 3) == 0) tmp = pHandleBackgroundColor(pinfo), Break;
                        else if (memcmp(fmt_next, "Cfg", 3) == 0) tmp = pHandleForegroundColor(pinfo), Break;
                    }
                default: { /*TODO: add user functions*/ 
                    }
            }
            if (failed) {
                StreamWrite(stream, pString((u8*)fmt, tmp.fmt - fmt));
                fmt = tmp.fmt + 1;
            }
            fmt        = tmp.fmt + 1;
            printcount = tmp.count;
            list       = tmp.list;
        }
    }
    return printcount;
}

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count) {
    s64 test = (s64)count - (s64)string.length;
    if (test > 0) {
        char *arr = pCurrentAllocatorFunc(NULL, (u64)test, 0, MALLOC, pCurrentAllocatorUserData);
        void *tmp = memset(arr, character, (u64)test); arr = tmp;

        if (expect(!right_justified, 1)) {
            StreamWrite(stream, (String){ tmp, (u64)test });
            StreamWrite(stream, string);
        } else {
            StreamWrite(stream, string);
            StreamWrite(stream, (String){ tmp, (u64)test });
        }
        pCurrentAllocatorFunc(arr, 0, 0, FREE, pCurrentAllocatorUserData);
        return string.length + (u64)test;
    } else {
        StreamWrite(stream, string);
        return string.length;
    }
}

void GetRGB(char *restrict* fmtptr, String RGB[3]) {
    char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        u8 *begin = (u8*)fmt + 1;
        u8 *end   = (u8*)fmt + 1;
        // we check if there is any whitespace that needs to be skipped
        // this allows %Cfg( 255 , 255 , 255 )
        while( *end == ' ' || *end == '\t'  ) end++;
        while( *end >= '0' && *end <= '9') end++;
        RGB[n++] = (String){ begin, (usize)(end - begin) }; 
        while( *end == ' ' || *end == '\t'  ) end++;

        fmt = (char *)end;
    }
    *fmtptr = fmt;
}

pPrintfInfo pHandleBinary(pPrintfInfo info, pFormattingSpecification spec) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];

    u32 numbits[] = {
        [PFL_DEFAULT] = sizeof(u32),
        [PFL_HH]      = sizeof(u8),
        [PFL_H]       = sizeof(u16),
        [PFL_L]       = sizeof(u32),
        [PFL_LL]      = sizeof(u64),
        [PFL_J]       = sizeof(intmax_t),
        [PFL_Z]       = sizeof(size_t),
        [PFL_T]       = sizeof(ptrdiff_t),
    };

    if (spec.length == PFL_128) {
        union {
            struct { u64 high, low; };
            long double ld;
        } conv;
        long double ld = va_arg(info.list, long double);
        conv.ld = ld;

        struct BinaryStringReturn high = MakeBinaryString(64, conv.high);
        struct BinaryStringReturn low  = MakeBinaryString(64, conv.low);
        if (spec.justification_count == 0 && spec.justification_char_is_zero) {
            info.count += PrintJustified(info.stream, high.str, spec.right_justified, 
                character, 64);
            StreamWriteString(info.stream, low.str);
            info.count += low.str.length; 
        } else {
            PrintJustified(info.stream, high.str, spec.right_justified, 
                character, spec.justification_count);
            StreamWriteString(info.stream, low.str);
        }

        pCurrentAllocatorFunc(high.buffer, 0, 0, FREE, pCurrentAllocatorUserData);
        pCurrentAllocatorFunc(low.buffer, 0, 0, FREE, pCurrentAllocatorUserData);
    } else {
        u64 num = va_arg(info.list, u64);
        struct BinaryStringReturn high = MakeBinaryString(numbits[ spec.length ], num);
        if (spec.justification_count == 0 && spec.justification_char_is_zero) {
            info.count += PrintJustified(info.stream, high.str, spec.right_justified, 
                character, 64);
        } else {
            info.count += PrintJustified(info.stream, high.str, spec.right_justified, 
                character, spec.justification_count);
        }
    }
    return info;
}

pPrintfInfo pHandleBackgroundColor(pPrintfInfo info) {
    info.fmt += 3;
    if (*info.fmt != '(') return info;
    String header = pCreateString("\x1b[48;2;");
    StreamWrite(info.stream, header);
    String RGB[3];
    GetRGB(&info.fmt, RGB);
    StreamWrite(info.stream, RGB[0]);
    StreamWrite(info.stream, ';');
    StreamWrite(info.stream, RGB[1]);
    StreamWrite(info.stream, ';');
    StreamWrite(info.stream, RGB[2]);
    StreamWrite(info.stream, 'm');
    info.count += RGB[0].length 
               +  RGB[1].length 
               +  RGB[2].length
               +  header.length 
               +  3;

    return info;
}

pPrintfInfo pHandleChar(pPrintfInfo info, bool wide) {
    if (expect(wide, 0)){
        int character = va_arg(info.list, int);
        StreamWrite(info.stream, (char)character);
        info.count++;
    } else {
        char *character = va_arg(info.list, char *);
        u32 len = GetUnicodeLength(character);
        StreamWrite(info.stream, (String){ (u8 *)character, len});
        info.count += len;
    }
    return info;
}

pPrintfInfo pHandleString(pPrintfInfo info, pFormattingSpecification spec, bool cstring) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];
    String str;
    if (cstring){
        char *c_str = va_arg(info.list, char *);
        str = (String){ .c_str = (u8 *)c_str, .length = strlen(c_str) };
    } else { 
        str = va_arg(info.list, String); 
    }
    info.count += PrintJustified(info.stream, str, spec.right_justified, 
            character, spec.justification_count);

    return info;
}


pPrintfInfo pHandleSignedInt(pPrintfInfo,pFormattingSpecification,s64, bool always_print_sign);
pPrintfInfo pHandleOctalInt(pPrintfInfo,pFormattingSpecification,s64, bool always_print_sign);
pPrintfInfo pHandleHexadecimalInt(pPrintfInfo,pFormattingSpecification,u64, bool always_print_sign, bool uppercase);
pPrintfInfo pHandleUnsignedInt(pPrintfInfo,pFormattingSpecification,u64, bool always_print_sign);

pPrintfInfo pHandleInt(pPrintfInfo info, pFormattingSpecification spec, char printtype, bool always_print_sign) {
    u64 num = 0;

    if ( expect( spec.length == PFL_DEFAULT, 1) ) {
        num = va_arg(info.list, s32);
    } else {
        switch(spec.length) {
            case PFL_HH: 
            case PFL_H: 
            case PFL_L:  num = va_arg(info.list, u32);       break;
            case PFL_LL: num = va_arg(info.list, u64);       break;
            case PFL_J:  num = va_arg(info.list, intmax_t);  break;
            case PFL_Z:  num = va_arg(info.list, usize);    break;
            case PFL_T:  num = va_arg(info.list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return info;
        }
    }

    switch (printtype) {
    case 'd': case 'i': return pHandleSignedInt(info, spec, (s64)num, always_print_sign);
    case 'o':           return pHandleOctalInt(info, spec, (s64)num, always_print_sign);
    case 'x': case 'X': return pHandleHexadecimalInt(info, spec, (s64)num, always_print_sign, printtype == 'X');
    case 'u':           return pHandleUnsignedInt(info, spec, (s64)num, always_print_sign);
    default: return info;
    }

}

pPrintfInfo pHandleSignedInt(pPrintfInfo info, pFormattingSpecification spec, s64 num, bool always_print_sign) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];
    u32 count;
    char buf[20];

    count = pSignedDecimalToString(buf, num);
    char *printbuf = buf;
    if (num > 0 && !always_print_sign) { printbuf++; count--; }
    info.count += PrintJustified(info.stream, (String){ (u8 *)printbuf, count }, 
           spec.right_justified, character, spec.justification_count);
    return info;
}

pPrintfInfo pHandleUnsignedInt(pPrintfInfo info, pFormattingSpecification spec, u64 num, bool always_print_sign) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];
    u32 count;
    char buf[20];
    count = pUnsignedDecimalToString(buf, num);
    
    if (always_print_sign)
        StreamWrite(info.stream, '+');

    info.count += PrintJustified(info.stream, (String){ (u8 *)buf, count }, 
           spec.right_justified, character, spec.justification_count) + 1;
    return info;
}
pPrintfInfo pHandleOctalInt(pPrintfInfo info,pFormattingSpecification spec, s64 num, bool always_print_sign) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];
    u32 count;
    char buf[20];
    count = pSignedOctalToString(buf, num);
    
    if (always_print_sign && num > 0)
        StreamWrite(info.stream, '+');

    info.count += PrintJustified(info.stream, (String){ (u8 *)buf, count }, 
           spec.right_justified, character, spec.justification_count) + 1;
    return info;

}

pPrintfInfo pHandleHexadecimalInt(pPrintfInfo info,pFormattingSpecification spec, u64 num, bool always_print_sign, bool uppercase) {
    const u8 character = spec.justificiation_chars[spec.justification_char_is_zero];
    u32 count;
#warning currently buffers for string to int are pretty small (20 bytes) so this might cause an issue
    char buf[20];
    count = pUnsignedHexToString(buf, num);
   
    if (uppercase) {
        for (u32 i = 0; i < count + 1; i++) {
            buf[i] = toupper(buf[i]);
        }
    }

    if (always_print_sign)
        StreamWrite(info.stream, '+');

    info.count += PrintJustified(info.stream, (String){ (u8 *)buf, count }, 
           spec.right_justified, character, spec.justification_count) + 1;
    return info;
    
}



pPrintfInfo pHandleFloat(pPrintfInfo info, pFormattingSpecification spec) {
    (void)info;
    (void)spec;
    assert(false);
    abort();
}

pPrintfInfo pHandlePointer(pPrintfInfo info, pFormattingSpecification spec) {
#if defined(PSTD_32)
    spec.length = PFL_DEFAULT;
#elif defined(PSTD_64)
    spec.length = PFL_LL;
#else
#error neither 32 or 64 bit!
#endif

    void *ptr = va_arg(info.list, void *);
    return pHandleHexadecimalInt(info, spec, (u64)ptr, false, false);
}

pPrintfInfo pHandleCharatersWritten(pPrintfInfo info, pFormattingSpecification spec) {
    if ( expect( spec.length == PFL_DEFAULT, 1) ) {
        s32 *count = va_arg(info.list, s32*);
        *count = info.count;
    } else {
        switch(spec.length) {
            case PFL_HH: {  u8  *count = va_arg(info.list, u8 *); *count = info.count; } break;  
            case PFL_H:  {  u16 *count = va_arg(info.list, u16*); *count = info.count; } break; 
            case PFL_L:  {  u32 *count = va_arg(info.list, u32*); *count = info.count; } break; 
            case PFL_LL: {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_J:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_Z:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_T:  {  u64 *count = va_arg(info.list, u64*); *count = info.count; } break; 
            case PFL_DEFAULT: case PFL_128:
            default: break;
        }
    }
    return info;
}

pPrintfInfo pHandleForegroundColor(pPrintfInfo info) {
    info.fmt += 3;
    if (*info.fmt != '(') return info;
    String header = pCreateString("\x1b[38;2;");
    StreamWrite(info.stream, header);
    String RGB[3];
    GetRGB(&info.fmt, RGB);
    StreamWrite(info.stream, RGB[0]);
    StreamWrite(info.stream, ';');
    StreamWrite(info.stream, RGB[1]);
    StreamWrite(info.stream, ';');
    StreamWrite(info.stream, RGB[2]);
    StreamWrite(info.stream, 'm');
    info.count += RGB[0].length 
               +  RGB[1].length 
               +  RGB[2].length
               +  header.length 
               +  3;

    return info;
}
pPrintfInfo pHandleColorClear(pPrintfInfo info) {
    String reset = pCreateString("\x1b[0m");
    StreamWrite(info.stream, reset);
    info.fmt++;
    info.count += reset.length; 
    return info;
}

u32 GetUnicodeLength(const char *chr) {
    /**/ if ((chr[0] & 0xf8) == 0xf0)
         return 4;
    else if ((chr[0] & 0xf0) == 0xe0)
         return 3;
    else if ((chr[0] & 0xe0) == 0xc0)
         return 2;
    else return 1;
}




#include "table.c"

///////////////////////////////////////////////////////////////////////////////////////////////////
//                            ||                                   ||                            //
//                            ||                                   ||                            //
//                            ||            IntToString            ||                            //
//                            ||                                   ||                            //
//                            ||                                   ||                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

u32 pSignedIntToString(char *buf, s64 num, u32 radix, char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3]) {
    if (num < 0) {
        *buf++ = '-';
        num = llabs(num);
    } else *buf++ = '+';
    return pUnsignedIntToString(buf, (u64)num, radix, radixarray, pow2array, pow3array) + 1; 
}

u32 pUnsignedIntToString(char *buf, u64 num, u32 radix, char radixarray[], const char (*pow2array)[2], const char (*pow3array)[3])  {
    u64 pow3 = radix * radix * radix;
    u64 pow2 = radix * radix * radix;
    u32 printnum = 0;
    register char *ptr = buf;
    if (num >= pow3) {
        while(num) {
            register u32 mod = num % pow3;
            num = num / pow3;
            if (mod < radix){
                *ptr++ = radixarray[mod];
                printnum++;
            }
            else if (mod < pow2) {
                register const char (*s)[2] = pow2array + mod;
                ptr[0] = (*s)[1];
                ptr[1] = (*s)[0];
                ptr += 2;
                printnum += 2;
            } else {
                register const char (*s)[3] = pow3array + mod;
                ptr[0] = (*s)[2];
                ptr[1] = (*s)[1];
                ptr[2] = (*s)[0];
                ptr += 3;
                printnum += 3;
            }
        }
    } else if (num < radix) {
        *ptr++ = radixarray[num];
        return 1;
    } else if (num < pow2) {
        register const char (*s)[2] = pow2array + num;
        ptr[0] = (*s)[1];
        ptr[1] = (*s)[0];
        return 2;
    } else {
        register const char (*s)[3] = pow3array + num;
        ptr[0] = (*s)[2];
        ptr[1] = (*s)[1];
        ptr[2] = (*s)[0];
        return 3;
    }

    ptr--;
    while(ptr > buf) {
        const char tmp = *buf;
        *buf++ = *ptr;
        *ptr-- = tmp;
    }
    return printnum;
}

u32 pSignedDecimalToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, 10, (char *)pMod10, pMod100, pMod1000);
}
u32 pUnsignedDecimalToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, 10, (char *)pMod10, pMod100, pMod1000);
}
u32 pSignedHexToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, 16, (char *)pMod16, pMod256, pMod4096);
}
u32 pUnsignedHexToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, 16, (char *)pMod16, pMod256, pMod4096);
}
u32 pSignedOctalToString(char *buf, s64 num) {
    return pSignedIntToString(buf, num, 8, (char *)pMod8, pMod64, pMod512);
}
u32 pUnsignedOctalToString(char *buf, u64 num) {
    return pUnsignedIntToString(buf, num, 8, (char *)pMod8, pMod64, pMod512);
}
