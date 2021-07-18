#include "pio.h"
#include "stretchy_buffer.h"
#include <ctype.h>

#define PIO_STATIC PSTD_UNUSED static inline 

#define BASE_10 10
#define BASE_8   8
#define BASE_16 16


typedef struct AdvUserCallback AdvUserCallback; 
struct AdvUserCallback {
    String format;
    FormatCallbackAdv *callback;
};

typedef struct UserCallback UserCallback; 
struct UserCallback {
    String format;
    FormatCallback *callback;
};

typedef struct BinaryStringReturn BinaryStringReturn;
struct BinaryStringReturn {
    u8 *buffer;
    String str;
    pBool iszero;
};

static AdvUserCallback *stretchy advcallbacks = NULL;
static UserCallback    *stretchy callbacks    = NULL;

// forward declaration
static pBool color_output;

PIO_STATIC void pPrintf_handle_length(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_plus(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_minus(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_space(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_zero(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_hash(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_dot(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_number(pPrintfInfo *info, pFormattingSpecification *spec);

PIO_STATIC void pPrintf_handle_char(pPrintfInfo *info, pBool wide);
PIO_STATIC void pPrintf_handle_string(pPrintfInfo *info, pFormattingSpecification *spec, pBool cstr);
PIO_STATIC void pPrintf_handle_int(pPrintfInfo *info, pFormattingSpecification *spec, char printtype);
PIO_STATIC void pPrintf_handle_float(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_pointer(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_characters_written(pPrintfInfo *info, pFormattingSpecification *spec);

PIO_STATIC void pPrintf_handle_binary(pPrintfInfo *info, pFormattingSpecification *spec);
PIO_STATIC void pPrintf_handle_background_color(pPrintfInfo *info);
PIO_STATIC void pPrintf_handle_foreground_color(pPrintfInfo *info);
PIO_STATIC void pPrintf_handle_color_clear(pPrintfInfo *info);

PIO_STATIC u64 pPrintf_print_justified(GenericStream *stream, pFormattingSpecification *spec, String str);
PIO_STATIC void pPrintf_get_rgb(const char *restrict* fmt, String RGB[3]);

#if defined(PSTD_USE_ALLOCATOR)
struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num, Allocator *cb);
#else
struct BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num);
#endif

u32 pVBPrintf(GenericStream *stream, const char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (PSTD_EXPECT(*fmt != '%', 1)) {
            const char *restrict fmt_next = fmt;
            while (pCharAnyOf(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) fmt_next++;
            pStreamWriteString(stream, pString( (u8 *)fmt, (usize)(fmt_next - fmt)));
            printcount += fmt_next - fmt;
            fmt = fmt_next;
        }
        else {
            pBool failed = false;
            struct pFormattingSpecification jinfo = {
                .right_justified = false,
                .justification_count = 0,
                .prefix_zero = false,
                .length = PFL_DEFAULT,
            };
            const char* restrict fmt_next = fmt + 1;
            pPrintfInfo pinfo = {
                .stream = stream,
                .fmt = fmt_next,
                .count = printcount,
                .failflag = &failed,
            };
            va_copy(pinfo.list, list);

#define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            pBool found_format = false;
            for (usize i = 0; i < pSize(callbacks); i++) {
                if ((u8)*fmt_next == callbacks[i].format.c_str[0]) {
                    found_format = true;
                    String format = callbacks[i].format;
                    for (usize j = 1; j < format.length; j++)
                        if (format.c_str[j] != (u8)fmt_next[j]) { found_format = false; break; }

                    if (found_format) {
                        callbacks[i].callback(&pinfo); break;
                    }
                }
            }
            if (!found_format) {
                switch (*fmt_next) {
                case 'n': pPrintf_handle_characters_written(&pinfo, &jinfo); break;
                case '-': pPrintf_handle_minus(&pinfo, &jinfo); break;
                case '+': pPrintf_handle_plus( &pinfo, &jinfo); break;
                case '0': pPrintf_handle_zero( &pinfo, &jinfo); break;
                case '#': pPrintf_handle_hash( &pinfo, &jinfo); break;
                case '.': pPrintf_handle_dot(  &pinfo, &jinfo); break;
                case ' ': pPrintf_handle_space(&pinfo, &jinfo); break;

                case '1': case '2':
                case '3': case '4':
                case '5': case '6':
                case '7': case '8':
                case '9': case '*': pPrintf_handle_number(&pinfo, &jinfo); break;

                case 'h': case 'l':
                case 'j': case 'z':
                case 't': case 'L': pPrintf_handle_length(&pinfo, &jinfo); break;

                case 's': case 'S': 
                    pPrintf_handle_string(&pinfo, &jinfo, *fmt_next == 's' ? true : false); break;

                case 'f': case 'F':
                case 'e': case 'E':
                case 'a': case 'A':
                case 'g': case 'G': pPrintf_handle_float(&pinfo, &jinfo); break;

                case 'u':
                case 'x': case 'X':
                case 'i': case 'd':  pPrintf_handle_int(&pinfo, &jinfo, *fmt_next); break;
                case 'p': pPrintf_handle_pointer(&pinfo, &jinfo); break;
                case 'b': pPrintf_handle_binary(&pinfo, &jinfo); break;
                case 'c': pPrintf_handle_char(&pinfo, false);   break;
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0) { 
                            pPrintf_handle_color_clear(&pinfo); 
                            break; 
                        }
                        else if (memcmp(fmt_next, "Cbg", 3) == 0) { 
                            pPrintf_handle_background_color(&pinfo);
                            break;
                        }
                        else if (memcmp(fmt_next, "Cfg", 3) == 0) { 
                            pPrintf_handle_foreground_color(&pinfo); 
                            break; 
                        }
                    }
                default: {
                        failed = true;
                    }
                }
            }
            if (failed) {
                pStreamWriteString(stream, pString((u8*)fmt, pinfo.fmt - fmt));
                fmt = pinfo.fmt + 1;
            }
            fmt = pinfo.fmt + 1;
            printcount = pinfo.count;
            list = pinfo.list;
        }
    }
    return printcount;
}

#if defined(PSTD_USE_ALLOCATOR)
PIO_STATIC BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num, Allocator *cb) {
#else
PIO_STATIC BinaryStringReturn MakeBinaryString(u64 bitcount, u64 num) {
#endif
    struct BinaryStringReturn ret = { 0 };

    u64 bit = 1ULL << (bitcount - 1);
#if defined(PSTD_USE_ALLOCATOR)
    ret.buffer = cb->allocator(cb, ALLOCATE, bitcount + 1, NULL);
#else
    ret.buffer = pAllocate(bitcount + 1);
#endif
    for (u64 i = 0; i < bitcount; i++) {
       ret.buffer[i] = (u8)'0' + ((num & bit) ? 1 : 0); 
       bit >>= 1;
    }

    ret.str.c_str = ret.buffer;
    s32 count = (s32)bitcount;
    while(true) { 
        if (*ret.str.c_str != '0') break;
        ret.str.c_str++;
        count--;
    }
    if (count <= 0) { 
        ret.iszero = true;
        ret.str.c_str -= 1; 
        count = 1; 
    }
    ret.str.length = (usize)count; 
    return ret;
}



PIO_STATIC u64 pPrintf_print_justified(GenericStream *stream, pFormattingSpecification *spec, String string) {
    s64 space_count = spec->justification_count; 
    s64 zero_count  = spec->zero_justification_count; 
    s64 count = space_count + zero_count;
#if defined(PSTD_USE_ALLOCATOR)
    Allocator cb = stream->cb;
#endif

    s64 test = count - (s64)string.length;
    if (test > 0) {
        space_count -= (s64)string.length;
        zero_count  -= (s64)string.length;
        usize space_size = space_count > 0 ? space_count : 1;
        usize zero_size  = zero_count  > 0 ? zero_count  : 1;
#if defined(PSTD_USE_ALLOCATOR)
        u8 *spaces = cb.allocator(&cb, ALLOCATE, space_size, NULL);
        u8 *zeros  = cb.allocator(&cb, ALLOCATE, zero_size, NULL);
#else
        u8 *spaces = pAllocate(space_size);
        u8 *zeros  = pAllocate(zero_size);
#endif
        if (space_count > 0)  memset(spaces, ' ', space_count);
        if (zero_count  > 0)  memset(zeros,  '0', zero_count);

        if (PSTD_EXPECT(!spec->right_justified, 1)) {
            if (space_count > 0) pStreamWriteString(stream, pString( spaces, space_count ));
            if (zero_count > 0)  pStreamWriteString(stream, pString( zeros, zero_count ));
            pStreamWriteString(stream, string);
        } else {
            if (zero_count > 0)  pStreamWriteString(stream, pString( zeros, zero_count ));
            pStreamWriteString(stream, string);
            if (space_count > 0) pStreamWriteString(stream, pString( spaces, space_count ));
        }
#if defined(PSTD_USE_ALLOCATOR)
        cb.allocator(&cb, SIZED_FREE, space_size, spaces);
        cb.allocator(&cb, SIZED_FREE, zero_size, zeros);
#else
        pSizedFree(space_size, spaces);
        pSizedFree(zero_size, zeros);
#endif
        return string.length + (u64)test;
    } else {
        pStreamWriteString(stream, string);
        return string.length;
    }
}

PIO_STATIC pBool IsRGBWhitespace(char chr) {
     return chr == ' ' 
         || chr == '\t' 
         || chr == '\r' 
         || chr == '\n';
}

PIO_STATIC void pPrintf_get_rgb(const char *restrict* fmtptr, String RGB[3]) {
    const char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        u8 *begin = (u8*)fmt + 1;
        u8 *end   = (u8*)fmt + 1;
        // we check if there is any whitespace that needs to be skipped
        // this allows %Cfg( 255 , 255 , 255 )
        while( IsRGBWhitespace(*end) ) begin++, end++;
        while( *end >= '0' && *end <= '9') end++;
        RGB[n++] = pString( begin, (usize)(end - begin) ); 
        while( IsRGBWhitespace(*end) ) end++;

        fmt = (char *)end;
    }
    *fmtptr = fmt;
}

PIO_STATIC void pPrintf_handle_binary(pPrintfInfo *info, pFormattingSpecification *spec) {
#define NUM_BITS_IN_BYTE 8
    u32 numbits[] = {
        [PFL_DEFAULT] = sizeof(u32)       * NUM_BITS_IN_BYTE,
        [PFL_HH]      = sizeof(u8)        * NUM_BITS_IN_BYTE,
        [PFL_H]       = sizeof(u16)       * NUM_BITS_IN_BYTE,
        [PFL_L]       = sizeof(u32)       * NUM_BITS_IN_BYTE,
        [PFL_LL]      = sizeof(u64)       * NUM_BITS_IN_BYTE,
        [PFL_J]       = sizeof(intmax_t)  * NUM_BITS_IN_BYTE,
        [PFL_Z]       = sizeof(size_t)    * NUM_BITS_IN_BYTE,
        [PFL_T]       = sizeof(ptrdiff_t) * NUM_BITS_IN_BYTE,
    };
#undef NUM_BITS_IN_BYTE

#if defined(PSTD_USE_ALLOCATOR)
    Allocator *cb = &info->stream->cb;
#endif

    if (spec->length == PFL_128) {
        union {
            struct { u64 high, low; };
            long double ld;
        } conv;
        long double ld = va_arg(info->list, long double);
        conv.high = 0;
        conv.low  = 0;
        conv.ld = ld;

#if defined(PSTD_USE_ALLOCATOR)
        BinaryStringReturn high = MakeBinaryString(numbits[PFL_LL], conv.high, cb);
        BinaryStringReturn low  = MakeBinaryString(numbits[PFL_LL], conv.low,  cb);
#else
        BinaryStringReturn high = MakeBinaryString(numbits[PFL_LL], conv.high);
        BinaryStringReturn low  = MakeBinaryString(numbits[PFL_LL], conv.low);
#endif
        if (spec->zero_justification_count == 0 && spec->prefix_zero) {
            spec->zero_justification_count = numbits[PFL_LL];
            info->count += pPrintf_print_justified(info->stream, spec, high.str);
            pStreamWriteString(info->stream, low.str);
        } else {
            pPrintf_print_justified(info->stream, spec, high.str);
            pStreamWriteString(info->stream, low.str);
        }

        info->count += low.str.length; 
#if defined(PSTD_USE_ALLOCATOR)
        cb->allocator(cb, SIZED_FREE, numbits[PFL_LL], high.buffer);
        cb->allocator(cb, SIZED_FREE, numbits[PFL_LL], low.buffer);
#else
        pSizedFree(numbits[PFL_LL], high.buffer);
        pSizedFree(numbits[PFL_LL], low.buffer);
#endif
    } else {
        u64 num;
        if (PSTD_EXPECT(spec->length == PFL_DEFAULT, 1)) {
            num = va_arg(info->list, s32);
        }
        else {
            switch (spec->length) {
            case PFL_HH:
            case PFL_H:
            case PFL_L:  num = va_arg(info->list, u32);       break;
            case PFL_LL: num = va_arg(info->list, u64);       break;
            case PFL_J:  num = va_arg(info->list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(info->list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return;
            }
        }
        
#if defined(PSTD_USE_ALLOCATOR)
        BinaryStringReturn binary = MakeBinaryString(numbits[ spec->length ], num, cb);
#else
        BinaryStringReturn binary = MakeBinaryString(numbits[ spec->length ], num);
#endif
        if (spec->zero_justification_count == 0 && spec->prefix_zero) {
            spec->zero_justification_count = binary.iszero ? 0 : numbits[ spec->length ]; 
            info->count += pPrintf_print_justified(info->stream, spec, binary.str);
        } else {
            info->count += pPrintf_print_justified(info->stream, spec, binary.str);
        }
#if defined(PSTD_USE_ALLOCATOR)
        cb->allocator(cb, SIZED_FREE, numbits[ spec->length ], binary.buffer);
#else
        pSizedFree(numbits[ spec->length ], binary.buffer);
#endif
    }
}

PIO_STATIC void pPrintf_handle_background_color(pPrintfInfo *info) {
        info->fmt += 3;
        if (*info->fmt != '(') return;
        
        String RGB[3];
        pPrintf_get_rgb(&info->fmt, RGB);
    if (PSTD_EXPECT(color_output, 1)) {
        String header = pCreateString("\x1b[48;2;");
        pStreamWriteString(info->stream, header);
        pStreamWriteString(info->stream, RGB[0]);
        pStreamWriteChar(info->stream, ';');
        pStreamWriteString(info->stream, RGB[1]);
        pStreamWriteChar(info->stream, ';');
        pStreamWriteString(info->stream, RGB[2]);
        pStreamWriteChar(info->stream, 'm');
        info->count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }
}

PIO_STATIC void pPrintf_handle_foreground_color(pPrintfInfo *info) {
    info->fmt += 3;
    if (*info->fmt != '(') return;
    String RGB[3];
    pPrintf_get_rgb(&info->fmt, RGB);

    if (PSTD_EXPECT(color_output, 1)) {
        String header = pCreateString("\x1b[38;2;");
        pStreamWriteString(info->stream, header);
        pStreamWriteString(info->stream, RGB[0]);
        pStreamWriteChar(info->stream, ';');
        pStreamWriteString(info->stream, RGB[1]);
        pStreamWriteChar(info->stream, ';');
        pStreamWriteString(info->stream, RGB[2]);
        pStreamWriteChar(info->stream, 'm');
        info->count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }
}

PIO_STATIC void pPrintf_handle_char(pPrintfInfo *info, pBool wide) {
    if (PSTD_EXPECT(wide == false, 1)){
        int character = va_arg(info->list, int);
        pStreamWriteChar(info->stream, (char)character);
        info->count++;
    } else {
        char *character = va_arg(info->list, char *);
        u32 len = pGetUtf8Length(character);
        pStreamWriteString(info->stream, pString( (u8 *)character, len));
        info->count += len;
    }
}

PIO_STATIC void pPrintf_handle_string(pPrintfInfo *info, pFormattingSpecification *spec, pBool cstring) {
    String str;
    if (cstring){
        char *c_str = va_arg(info->list, char *);
        str = pString((u8 *)c_str, strlen(c_str) );
    } else { 
        str = va_arg(info->list, String); 
    }
    info->count += pPrintf_print_justified(info->stream, spec, str);
}

PIO_STATIC void pPrintf_handle_color_clear(pPrintfInfo *info) {
    String reset = pCreateString("\x1b[0m");
    if (PSTD_EXPECT(color_output, 1)) {
        pStreamWriteString(info->stream, reset);
        info->count += reset.length; 
    }
    info->fmt++;
}

PIO_STATIC void pPrintf_handle_hash(pPrintfInfo *info, pFormattingSpecification *spec) {
    spec->alternative_form = true;
    info->fmt++;

    switch(*info->fmt) {
    case '-': return pPrintf_handle_minus(info, spec);
    case '0': return pPrintf_handle_zero(info, spec);
    case '.': return pPrintf_handle_dot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pPrintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);
    case 'o':
    case 'x': case 'X': pPrintf_handle_int(info, spec, *info->fmt);
    default: return;
    }
}

PIO_STATIC void pPrintf_handle_number(pPrintfInfo *info, pFormattingSpecification *spec) { 
    const char *restrict begin = info->fmt;
    char *end;
    if (*info->fmt != '*')
        spec->justification_count = strtoul(begin, &end, BASE_10);
    else {
        spec->justification_count = va_arg(info->list, int);
        end = (char*)info->fmt+1;
    }
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 

    switch (*info->fmt) {
    case '.': return pPrintf_handle_dot(info, spec);
    case 's': case 'S': return pPrintf_handle_string(info, spec, *info->fmt == 's' ? true : false);
    
    case 'h': case 'l':
    case 'j': case 'z':
    case 't': case 'L': return pPrintf_handle_length(info, spec); break;

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
    case 'p': return pPrintf_handle_pointer(info, spec);
    case 'b': return pPrintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pPrintf_handle_dot(pPrintfInfo *info, pFormattingSpecification *spec) { 
    const char *restrict begin = info->fmt + 1;
    char *end;
    if (*begin >= '1' && *begin <= '9')
        spec->zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec->zero_justification_count = va_arg(info->list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info->fmt; 
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 
    spec->prefix_zero = true; 

    switch (*info->fmt) {
    case '.': return pPrintf_handle_dot(info, spec);
    case 's': case 'S': return pPrintf_handle_string(info, spec, *info->fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pPrintf_handle_length(info, spec);

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
    case 'p': return pPrintf_handle_pointer(info, spec);
    case 'b': return pPrintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pPrintf_handle_zero(pPrintfInfo *info, pFormattingSpecification *spec) { 
    const char *restrict begin = info->fmt + 1;
    char *end;
    if (*begin >= '1' && *begin <= '9')
        spec->zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec->zero_justification_count = va_arg(info->list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info->fmt + 1; 
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 
    spec->prefix_zero = true; 

    switch (*info->fmt) {
    case '.': return pPrintf_handle_dot(info, spec);
    case 's': case 'S': return pPrintf_handle_string(info, spec, *info->fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pPrintf_handle_length(info, spec);
    
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
    case 'p': return pPrintf_handle_pointer(info, spec);
    case 'b': return pPrintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pPrintf_handle_space(pPrintfInfo *info, pFormattingSpecification *spec) { 
    while (*info->fmt == ' ') info->fmt++;

    switch( *info->fmt ) {
    case '-': return pPrintf_handle_minus(info, spec);
    case '0': return pPrintf_handle_zero(info, spec);
    case '.': return pPrintf_handle_dot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pPrintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);
    case 'o':
    case 'x': case 'X': pPrintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pPrintf_handle_length(pPrintfInfo *info, pFormattingSpecification *spec) { 
    const char *fmt = info->fmt;

    switch(*info->fmt) {
    case 'h': spec->length = (*(fmt + 1) == 'h') ? (info->fmt++, PFL_HH) : PFL_H; break;
    case 'l': spec->length = (*(fmt + 1) == 'l') ? (info->fmt++, PFL_LL) : PFL_L; break;
    case 'j': spec->length = PFL_J;   break;
    case 'z': spec->length = PFL_Z;   break;
    case 't': spec->length = PFL_T;   break;
    case 'L': spec->length = PFL_128; break;
    }
     
    info->fmt++;
    if (PSTD_EXPECT(spec->length == PFL_L && *info->fmt == 'c', 0) == false) {
        switch(*info->fmt) {
        case 'f': case 'F':
        case 'e': case 'E':
        case 'a': case 'A':
        case 'g': case 'G': return pPrintf_handle_float(info, spec);

        case 'u': case 'o': 
        case 'x': case 'X':
        case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
        case 'b': return pPrintf_handle_binary(info, spec);
        case 'n': return pPrintf_handle_characters_written(info, spec);
        default: *info->failflag = true; return;
        }
    } else {
        return pPrintf_handle_char(info, true);
    }
}

PIO_STATIC void pPrintf_handle_plus(pPrintfInfo *info, pFormattingSpecification *spec) { 
    spec->force_sign = true;
    
    info->fmt++;
    switch( *info->fmt ) {
    case '-': return pPrintf_handle_minus(info, spec);
    case '0': return pPrintf_handle_zero(info, spec);
    case '.': return pPrintf_handle_dot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pPrintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pPrintf_handle_minus(pPrintfInfo *info, pFormattingSpecification *spec) { 
    spec->right_justified = true; 
    
    info->fmt++;
    switch( *info->fmt ) {
    case '+': return pPrintf_handle_plus(info, spec);
    case '0': return pPrintf_handle_zero(info, spec);
    case '.': return pPrintf_handle_dot(info, spec);
    case '#': return pPrintf_handle_hash(info, spec);
    case 's': case 'S': return pPrintf_handle_string(info, spec, *info->fmt == 's' ? true : false);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pPrintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pPrintf_handle_float(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pPrintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}


PIO_STATIC void pPrintf_handle_signedint(
        pPrintfInfo *info, pFormattingSpecification *spec, s64 num, pBool always_print_sign);
PIO_STATIC void pPrintf_handle_octalint(
        pPrintfInfo *info, pFormattingSpecification *spec, s64 num, pBool always_print_sign);
PIO_STATIC void pPrintf_handle_hexadecimalint(
        pPrintfInfo *info, pFormattingSpecification *spec, u64 num, 
        pBool always_print_sign, pBool uppercase);
PIO_STATIC void pPrintf_handle_unsignedint(
        pPrintfInfo *info, pFormattingSpecification *spec, u64 num, pBool always_print_sign);

PIO_STATIC void pPrintf_handle_int(pPrintfInfo *info, pFormattingSpecification *spec, char printtype) {
    u64 num = 0;

    if ( PSTD_EXPECT( spec->length == PFL_DEFAULT, 1) ) {
        num = va_arg(info->list, s32);
    } else {
        switch(spec->length) {
            case PFL_HH: 
            case PFL_H: 
            case PFL_L:  num = va_arg(info->list, u32);       break;
            case PFL_LL: num = va_arg(info->list, u64);       break;
            case PFL_J:  num = va_arg(info->list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(info->list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return;
        }
    }

    switch (printtype) {
    case 'd': case 'i': return pPrintf_handle_signedint(info, spec, (s64)num, spec->force_sign);
    case 'o':           return pPrintf_handle_octalint(info, spec, (s64)num, spec->force_sign); 
    case 'x': case 'X': 
        return pPrintf_handle_hexadecimalint(info, spec, (s64)num, spec->force_sign, printtype == 'X');
    case 'u': return pPrintf_handle_unsignedint(info, spec, (s64)num, spec->force_sign);
    default: break;
    }

}

#define STRING_BUFFER_SIZE 25

PIO_STATIC 
void pPrintf_handle_signedint(
        pPrintfInfo *info, pFormattingSpecification *spec, s64 num, pBool always_print_sign) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];

    count = pSignedDecimalToString(buf, num);
    char *printbuf = buf;
    if (num && num > 0 && !always_print_sign) { printbuf++; count--; }
    info->count += pPrintf_print_justified(info->stream, spec, pString( (u8 *)printbuf, count ));
}

PIO_STATIC 
void pPrintf_handle_unsignedint(
        pPrintfInfo *info, pFormattingSpecification *spec, u64 num, pBool always_print_sign) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pUnsignedDecimalToString(buf, num);
    
    if (always_print_sign)
        pStreamWriteChar(info->stream, '+');

    info->count += pPrintf_print_justified(info->stream, spec, pString( (u8 *)buf, count )) + 1;
}

PIO_STATIC 
void pPrintf_handle_octalint(
        pPrintfInfo *info, pFormattingSpecification *spec, s64 num, pBool always_print_sign)
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pSignedOctalToString(buf, num);
    
    char *printbuf = buf;
    if (num > 0 && !always_print_sign) { printbuf++; count--; }
    
    if (spec->alternative_form && num != 0) {
        String str = pCreateString("0o");
        if (spec->prefix_zero) {
            u32 zeros = spec->zero_justification_count;
            spec->zero_justification_count = 0;
            info->count += pPrintf_print_justified(info->stream, spec, str) + 1;
            spec->justification_count = 0; 
            spec->zero_justification_count = zeros;
            pPrintf_print_justified( info->stream, spec, pString( (u8 *)printbuf, count ) );
        } else {
            info->count += pPrintf_print_justified(info->stream, spec, str) + 1;
            info->count += count;
            pStreamWriteString( info->stream, pString( (u8 *)printbuf, count ) );
        }
    }
    else {
        info->count += pPrintf_print_justified(info->stream, spec, pString( (u8 *)printbuf, count )) + 1;
    }
}

PIO_STATIC 
void pPrintf_handle_hexadecimalint(
        pPrintfInfo *info, pFormattingSpecification *spec, 
        u64 num, pBool always_print_sign, pBool uppercase) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = pUnsignedHexToString(buf, num);
   
    if (uppercase) {
        for (u32 i = 0; i < count + 1; i++) {
            buf[i] = (u8)toupper(buf[i]);
        }
    }

    if (always_print_sign)
        pStreamWriteChar(info->stream, '+');

    if (spec->alternative_form && num != 0) {
        String str[2] = { pCreateString("0x"), pCreateString("0X") };
        if (spec->prefix_zero) {
            u32 zeros = spec->zero_justification_count;
            spec->zero_justification_count = 0;
            info->count += pPrintf_print_justified(info->stream, spec, str[uppercase]) + 1;
            spec->justification_count = 0; 
            spec->zero_justification_count = zeros;
            pPrintf_print_justified( info->stream, spec, pString( (u8 *)buf, count ) );
        } else {
            info->count += pPrintf_print_justified(info->stream, spec, str[uppercase]) + 1;
            info->count += count;
            pStreamWriteString( info->stream, pString( (u8 *)buf, count ) );
        }
    }
    else {
        info->count += pPrintf_print_justified(info->stream, spec, pString( (u8 *)buf, count )) + 1;
    }
}

//TODO: this is stupid and needs replacing
PIO_STATIC void pPrintf_handle_float(pPrintfInfo *info, pFormattingSpecification *spec) {
    (void)spec;
#if PSTD_HAS_VLA
#else
    Allocator *cb = &info->stream.cb;
#endif
    const char *restrict begin = info->fmt; 
    while (*begin != '%') begin--;
    usize size = info->fmt - begin + 1;
#if PSTD_HAS_VLA
    char buf[size + 1];
#else
    char *buf = cb->allocator(cb, ALLOCATE, size + 1, NULL);
#endif
    memcpy(buf, begin, size);
    buf[size] = '\0';
#if PSTD_HAS_VLA
#else
    cb->allocator(cb, FREE, size + 1, buf);
#endif

    usize count = 0;
    count = vsnprintf(NULL, 0, buf, info->list);
#if PSTD_HAS_VLA
    char out[count + 1];
#else
    char *out = cb->allocator(cb, ALLOCATE, count + 1, NULL);
#endif
    
    info->count += vsnprintf(out, count, buf, info->list);
    pStreamWriteString(info->stream, pString((u8*)out, count - 1));
#if defined(PSTD_GNU_COMPATIBLE)
#else
    cb->allocator(cb, FREE, count + 1, out);
#endif
}

PIO_STATIC void pPrintf_handle_pointer(pPrintfInfo *info, pFormattingSpecification *spec) {
#if defined(PSTD_32)
    spec->length = PFL_DEFAULT;
#elif defined(PSTD_64)
    spec->length = PFL_LL;
#else
#error neither 32 or 64 bit!
#endif

    spec->alternative_form = true; 

    void *ptr = va_arg(info->list, void *);
    if (!ptr) {
        const String null = (String){.c_str = (u8*)"nullptr",.length = sizeof("nullptr") - 1};
        pStreamWriteString(info->stream, null);
        info->count += null.length;
        return;
    }
    return pPrintf_handle_hexadecimalint(info, spec, (usize)ptr, false, false);
}

PIO_STATIC void pPrintf_handle_characters_written(pPrintfInfo *info, pFormattingSpecification *spec) {
    if ( PSTD_EXPECT( spec->length == PFL_DEFAULT, 1) ) {
        s32 *count = va_arg(info->list, s32*);
        *count = info->count;
    } else {
        switch(spec->length) {
            case PFL_HH: {  u8  *count = va_arg(info->list, u8 *); *count = info->count; } break;  
            case PFL_H:  {  u16 *count = va_arg(info->list, u16*); *count = info->count; } break; 
            case PFL_L:  {  u32 *count = va_arg(info->list, u32*); *count = info->count; } break; 
            case PFL_LL: {  u64 *count = va_arg(info->list, u64*); *count = info->count; } break; 
            case PFL_J:  {  u64 *count = va_arg(info->list, u64*); *count = info->count; } break; 
            case PFL_Z:  {  u64 *count = va_arg(info->list, u64*); *count = info->count; } break; 
            case PFL_T:  {  u64 *count = va_arg(info->list, u64*); *count = info->count; } break; 
            case PFL_DEFAULT: case PFL_128:
            default: break;
        }
    }
}


// CUSTOM FORMATTING

void pFormatPushImpl(String fmt, FormatCallback *callback) {
    if (pSize(callbacks)) {
        for (usize i = 0; i < pSize(callbacks); i++)
            if (pStringCmp(fmt, callbacks[i].format))
                return;
    }
    struct UserCallback usercb = { fmt, callback };
    pPushBack(callbacks, usercb);
}

void pFormatPushAdvImpl(String fmt, FormatCallbackAdv *callback) {
    if (pSize(advcallbacks)) {
        for (usize i = 0; i < pSize(advcallbacks); i++)
            if (pStringCmp(fmt, advcallbacks[i].format))
                return;
    }
    struct AdvUserCallback usercb = { fmt, callback };
    pPushBack(advcallbacks, usercb);
}

void pFormatPopImpl(String fmt) {
    struct UserCallback *remove = NULL;
    for (usize i = 0; i < pSize(callbacks); i++)
        if (pStringCmp(fmt, callbacks[i].format))
            remove = callbacks + i;
    
    if (remove) pRemove(callbacks, remove);
}

void pFormatPopAdvImpl(String fmt) {
    struct AdvUserCallback *remove = NULL;
    for (usize i = 0; i < pSize(advcallbacks); i++)
        if (pStringCmp(fmt, advcallbacks[i].format))
            remove = advcallbacks + i;
    
    if (remove) pRemove(advcallbacks, remove);
}
