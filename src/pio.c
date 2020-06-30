#include "pio.h"
#include <Windows.h>

#include "allocator.h"
extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

enum StreamType {
    STANDARD_STREAM,
    FILE_STREAM,
    STRING_STREAM
};

struct GenericStream {
    enum StreamType  type;
    u32 flags;
    void *buffer;
};

struct StringStream {
    const enum StreamType  type;
    u32 flags;
    usize buffersize;
    usize actualsize;
    u8 *c_str;
    // maybe more if not we just expose stringstream
};

struct FileStream {
    const enum StreamType  type;
    const u32 flags;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE filehandle;
#endif
};

struct StdStream {
    const enum StreamType  type;
    const enum StreamFlags flags;
#if defined(_WIN32) || defined(_WIN64)
    HANDLE stdout_handle;
    HANDLE stdin_handle;
#endif
};

void InitializeStdStream(void) __attribute__(( constructor ));
void DestroyStdStream(void) __attribute__(( destructor ));

static StdStream *StandardStream = NULL;
u32 default_code_page = 0;
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
#if defined(_WIN32) || defined(_WIN64)
    default_code_page = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    const StdStream template = { 
        .type          = STANDARD_STREAM,
        .flags         = STREAM_INPUT|STREAM_OUTPUT,
        .stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE),
        .stdin_handle  = GetStdHandle(STD_INPUT_HANDLE)
    };

    StandardStream = malloc(sizeof *StandardStream);
    pcurrentstream = (void *)StandardStream;
    memcpy(StandardStream, &template, sizeof template);

}
void DestroyStdStream(void) {
    free(StandardStream);
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(default_code_page);
#endif
}


StdStream *pGetStandardStream(enum StreamFlags flags) {
    union {
        const enum StreamFlags *flags;
        int *mutable_flags;
    } conv;

    StdStream *std = pCurrentAllocatorFunc(NULL, sizeof *std, MALLOC, pCurrentAllocatorUserData);
    memcpy(std, StandardStream, sizeof *std);
    conv.flags = &std->flags; *conv.mutable_flags = flags; 

    if ((flags & STREAM_INPUT) == 0)
        std->stdin_handle = NULL;
    if ((flags & STREAM_OUTPUT) == 0)
        std->stdout_handle = NULL;
    return std;
}

#define expect(x, value) __builtin_expect(x, value)

void StreamWriteString(GenericStream *stream, String str) {
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        WriteFile(fstream->filehandle, str.c_str, (u32)str.length, NULL, NULL);
    } else {
        // type == STRING_STREAM
        StringStream *sstream = (StringStream *)stream;
        usize diff = (sstream->buffersize - sstream->actualsize);
        if (diff <= str.length) {
            void *tmp = pCurrentAllocatorFunc(sstream->c_str, 
                    sstream->buffersize + str.length, REALLOC, pCurrentAllocatorUserData);
            sstream->c_str = tmp; 
            sstream->buffersize += str.length;
        }
        memcpy(sstream->c_str + sstream->actualsize, str.c_str, str.length);
        sstream->actualsize += str.length;
    }
}

void StreamWriteChar(GenericStream *stream, char str) {
    assert(stream->flags & STREAM_OUTPUT);

    if (expect(stream->type == STANDARD_STREAM, 1) || stream->type == FILE_STREAM) {
        FileStream *fstream = (FileStream *)stream;
        WriteFile(fstream->filehandle, &str, 1, NULL, NULL);
    } else {
        // type == STRING_STREAM
        StringStream *sstream = (StringStream *)stream;
        usize diff = (sstream->buffersize - sstream->actualsize);
        if (diff <= 1) {
            void *tmp = pCurrentAllocatorFunc(sstream->c_str, 
                    sstream->buffersize + 1, REALLOC, pCurrentAllocatorUserData);
            sstream->c_str = tmp; 
            sstream->buffersize += 1;
        }
        sstream->c_str[sstream->actualsize] = (u8)str;
        sstream->actualsize += 1;
    }
}

static bool IsCharacters(int character, u32 count, char tests[count]){
    for (u32 i = 0; i < count; i++) {
        if (character == tests[i]) return true;
    }
    return false;
}

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count);

void GetRGB(const char *restrict* fmt, String RGB[3]);
u32  GetUnicodeLength(const char *chr);

struct JusticifationInfo {
    bool right_justified;
    u32 justification_count;
    u8 justificiation_chars[2];
    bool justification_char_is_zero;
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
    u64 i = 0;
    ret.buffer = pCurrentAllocatorFunc(NULL, bitcount, MALLOC, pCurrentAllocatorUserData);
    do {
       ret.buffer[i] = (u8)'0' + ((num & bit) ? 1 : 0); 
       bit >>= 1;
    } while(i++ <= bitcount);

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

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (*fmt == '%'){
            bool always_print_sign  = false; 
            u32 bitcount = 32;
            bool bitcountset = false;
            struct JusticifationInfo info = {
                .right_justified    = false,
                .justification_count = 0,
                .justificiation_chars = { u' ', u'0' },
                .justification_char_is_zero  = false,
            };
            const char *restrict fmt_next = fmt + 1;
        repeat:
            #define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            switch(*fmt_next) {
                case '-': info.right_justified             = true; fmt_next++; goto repeat; 
                case '+': always_print_sign                = true; fmt_next++; goto repeat;
                case '0': info.justification_char_is_zero  = true; fmt_next++; goto repeat;

                case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    {
                        const char *restrict begin = fmt_next;
                        char *end;
                        info.justification_count = strtoul(begin, &end, 10);
                        // maybe fmt_next = ++end; not sure tbh
                        fmt_next = end; 
                        goto repeat;
                    }
                case 'h': {
                        if (*(fmt_next + 1) == 'h') { 
                            bitcount = sizeof(char) * 8;
                            fmt_next += 2;
                        }
                        else {
                            bitcount = sizeof(short)  * 8;
                            fmt_next++;
                        }
                        goto repeat;
                    }
                case 'l': {
                        if (*(fmt_next + 1) == 'l'){ 
                            bitcount = sizeof(long long) * 8;
                            fmt_next += 2;
                        }
                        else {
                            bitcount = sizeof(long)  * 8;
                            fmt_next++;
                        }
                        goto repeat;
                    }
                case 'j': SetBitCount(sizeof(intmax_t)    * 8, 1); goto repeat;
                case 'z': SetBitCount(sizeof(size_t)      * 8, 1); goto repeat;
                case 't': SetBitCount(sizeof(ptrdiff_t)   * 8, 1); goto repeat; 
                case 'L': SetBitCount(sizeof(long double) * 8, 1); goto repeat;
                
                case 's': case 'S': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        String str;
                        if (*fmt_next == 's'){
                            const char *c_str = va_arg(list, const char *);
                            str = (String){ .c_str = (const u8 *)c_str, .length = strlen(c_str) };
                        } else { 
                            str = va_arg(list, String); 
                        }
                        PrintJustified(stream, str, info.right_justified, 
                                character, info.justification_count);
                        break;
                    }
                case 'i':case 'd': {

                    } break;
                case 'u':
                case 'b': {
                        const u8 character = info.justificiation_chars[info.justification_char_is_zero];
                        u64 num = va_arg(list, u64);
                        struct BinaryStringReturn ret = MakeBinaryString(bitcount > 64 ? 64 : bitcount, num);
                        u32 old_justification_count = info.justification_count;
                        if (info.justification_count == 0 && info.justification_char_is_zero) {
                            info.justification_count = bitcount; 
                        }
                        PrintJustified(stream, ret.str, info.right_justified, 
                                character, info.justification_count);
                        info.justification_count = old_justification_count;
                        pCurrentAllocatorFunc(ret.buffer, 0, FREE, pCurrentAllocatorUserData);
                        break;
                    }
                case 'c': {
                    if (expect(bitcountset == false, 1)){
                        int character = va_arg(list, int);
                        StreamWrite(stream, (char)character);
                    }
                    else {
                        const char *character = va_arg(list, const char *);
                        u32 len = GetUnicodeLength(character);
                        StreamWrite(stream, (String){ (const u8 *)character, len});
                    }
                    break;
                }
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0) {
                            StreamWrite(stream, pCreateString("\x1b[0m"));
                            fmt_next++;
                            break;
                        }
                        else if (memcmp(fmt_next, "CB", 2) == 0) {
                            FormatCallBack *callback = va_arg(list, FormatCallBack *);
                             
                            fmt_next += 2;
                            FormatCallbackTuple ret = callback(stream, fmt_next, list);
                            fmt_next = ret.end_pos - 1;
                            list = ret.list;
                            break;
                        }
                        else if (memcmp(fmt_next, "Cbg", 3) == 0) {
                            fmt_next += 3;
                            if (*fmt_next != '(') break;
                            StreamWrite(stream, pCreateString("\x1b[48;2;"));
                        }
                        else if (memcmp(fmt_next, "Cfg", 3) == 0) {
                            fmt_next += 3;
                            if (*fmt_next != '(') break;
                            StreamWrite(stream, pCreateString("\x1b[38;2;"));
                        }
                        String RGB[3];
                        GetRGB(&fmt_next, RGB);
                        StreamWrite(stream, RGB[0]);
                        StreamWrite(stream, ';');
                        StreamWrite(stream, RGB[1]);
                        StreamWrite(stream, ';');
                        StreamWrite(stream, RGB[2]);
                        StreamWrite(stream, pCreateString("m"));
                    }
                    break;
                default: break;
            }
            fmt = ++fmt_next;
        }
        else {
            StreamWrite(pcurrentstream, (String){ (const u8 *)fmt++, 1 });
        }
    }
    return printcount;
}
u32 pBPrintf(GenericStream *stream, const char *restrict fmt, ...);

u64 PrintJustified(GenericStream *stream, String string, bool right_justified, const u8 character, u32 count) {
    s64 test = (s64)count - (s64)string.length;
    if (test > 0) {
        char *arr = pCurrentAllocatorFunc(NULL, (u64)test, MALLOC, pCurrentAllocatorUserData);
        void *tmp = memset(arr, character, (u64)test); arr = tmp;

        if (expect(!right_justified, 1)) {
            StreamWrite(stream, (String){ tmp, (u64)test });
            StreamWrite(stream, string);
        } else {
            StreamWrite(stream, string);
            StreamWrite(stream, (String){ tmp, (u64)test });
        }
        pCurrentAllocatorFunc(arr, 0, FREE, pCurrentAllocatorUserData);
        return string.length + (u64)test;
    } else {
        StreamWrite(stream, string);
        return string.length;
    }
}

void GetRGB(const char *restrict* fmtptr, String RGB[3]) {
    const char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        const u8 *begin = (const u8*)fmt + 1;
        const u8 *end   = (const u8*)fmt + 1;
        while(IsCharacters(*end, 2, (char[2]){ ',', ')'}) == false) end++;
        RGB[n++] = (String){ begin, (usize)(end - begin) }; 
        fmt = (const char *)end;
    }
    *fmtptr = fmt;
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



