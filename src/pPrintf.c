#include "pio.h"
#include "stretchy_buffer.h"
#include <ctype.h>

#if !defined(PSTD_C11)
#error what the fuck is going on?
#endif 

#define PIO_STATIC PSTD_UNUSED static inline 

#define BASE_10 10

typedef struct padv_user_callback_t padv_user_callback_t; 
struct padv_user_callback_t {
    pstring_t format;
    pformat_callback_adv_t *callback;
};

typedef struct puser_callback_t puser_callback_t; 
struct puser_callback_t {
    pstring_t format;
    pformat_callback_t *callback;
};

typedef struct pbinary_string_return_t pbinary_string_return_t;
struct pbinary_string_return_t {
    char *buffer;
    pstring_t str;
    pbool_t iszero;
};

static padv_user_callback_t *stretchy advcallbacks = NULL;
static puser_callback_t     *stretchy callbacks    = NULL;

// forward declaration
#ifndef PSTD_IO_GLOBAL_T_DEFINED
struct pio_global_t {
    pstd_stream_t           std_stream;
    u32                     default_code_page;
    pgeneric_stream_t       current_stream;
    pbool_t                 colored_output;
#if defined(PSTD_WINDOWS)
    u32                     console_mode;
#endif
};
#define PSTD_IO_GLOBAL_T_DEFINED
#endif

static struct pio_global_t PIO_GLOBALS;

PIO_STATIC void pprintf_handle_length(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_plus(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_minus(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_space(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_zero(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_hash(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_dot(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_number(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_bool(pprintf_info_t *info, pformatting_specification_t *spec);

PIO_STATIC void pprintf_handle_char(pprintf_info_t *info, pbool_t wide);
PIO_STATIC void pprintf_handle_string(pprintf_info_t *info, pformatting_specification_t *spec, pbool_t cstr);
PIO_STATIC void pprintf_handle_int(pprintf_info_t *info, pformatting_specification_t *spec, char printtype);
PIO_STATIC void pprintf_handle_float(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_pointer(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_characters_written(pprintf_info_t *info, pformatting_specification_t *spec);

PIO_STATIC void pprintf_handle_binary(pprintf_info_t *info, pformatting_specification_t *spec);
PIO_STATIC void pprintf_handle_background_color(pprintf_info_t *info);
PIO_STATIC void pprintf_handle_foreground_color(pprintf_info_t *info);
PIO_STATIC void pprintf_handle_color_clear(pprintf_info_t *info);

PIO_STATIC u64 pprintf_print_justified(pgeneric_stream_t *stream, pformatting_specification_t *spec, pstring_t str);
PIO_STATIC void pprintf_get_rgb(const char *restrict* fmt, pstring_t rgb[3]);

#if defined(PSTD_USE_ALLOCATOR)
PIO_STATIC struct pbinary_string_return_t pmake_binary_string(u64 bitcount, u64 num, pallocator_t *cb);
#else
PIO_STATIC struct pbinary_string_return_t pmake_binary_string(u64 bitcount, u64 num);
#endif

u32 pvbprintf(pgeneric_stream_t *stream, const char *restrict fmt, va_list list) {
    u32 printcount = 0;

    while(*fmt) {
        if (PSTD_EXPECT(*fmt != '%', 1)) {
            const char *restrict fmt_next = fmt;
            while (pchar_anyof(*fmt_next, 2, (char[2]){ '%', '\0'}) == false) fmt_next++;
            pstream_write_string(stream, pstring((char*)fmt, (usize)(fmt_next - fmt)));
            printcount += fmt_next - fmt;
            fmt = fmt_next;
        }
        else {
            pbool_t failed = false;
            struct pformatting_specification_t jinfo = {
                .right_justified = false,
                .justification_count = 0,
                .prefix_zero = false,
                .length = PFL_DEFAULT,
            };
            const char* restrict fmt_next = fmt + 1;
            pprintf_info_t pinfo = {
                .stream = stream,
                .fmt = fmt_next,
                .count = printcount,
                .failflag = &failed,
                .list     = &list
            };

#define SetBitCount(n, increment) bitcount = n; bitcountset = true; fmt_next += increment
            pbool_t found_format = false;
            for (usize i = 0; i < psb_size(callbacks); i++) {
                if (*fmt_next == callbacks[i].format.c_str[0]) {
                    found_format = true;
                    pstring_t format = callbacks[i].format;
                    for (usize j = 1; j < format.length; j++)
                        if (format.c_str[j] != fmt_next[j]) { found_format = false; break; }

                    if (found_format) {
                        callbacks[i].callback(&pinfo); break;
                    }
                }
            }
            if (!found_format) {
                switch (*fmt_next) {
                case 'n': pprintf_handle_characters_written(&pinfo, &jinfo); break;
                case '-': pprintf_handle_minus(&pinfo, &jinfo); break;
                case '+': pprintf_handle_plus( &pinfo, &jinfo); break;
                case '0': pprintf_handle_zero( &pinfo, &jinfo); break;
                case '#': pprintf_handle_hash( &pinfo, &jinfo); break;
                case '.': pprintf_handle_dot(  &pinfo, &jinfo); break;
                case ' ': pprintf_handle_space(&pinfo, &jinfo); break;

                case '1': case '2':
                case '3': case '4':
                case '5': case '6':
                case '7': case '8':
                case '9': case '*': pprintf_handle_number(&pinfo, &jinfo); break;

                case 'h': case 'l':
                case 'j': case 'z':
                case 't': case 'L': pprintf_handle_length(&pinfo, &jinfo); break;

                case 's': case 'S': 
                    pprintf_handle_string(&pinfo, &jinfo, *fmt_next == 's' ? true : false); break;

                case 'f': case 'F':
                case 'e': case 'E':
                case 'a': case 'A':
                case 'g': case 'G': pprintf_handle_float(&pinfo, &jinfo); break;

                case 'u':
                case 'x': case 'X':
                case 'i': case 'd':  pprintf_handle_int(&pinfo, &jinfo, *fmt_next); break;
                case 'p': pprintf_handle_pointer(&pinfo, &jinfo); break;
                case 'b': pprintf_handle_binary(&pinfo, &jinfo);  break;
                case 'B': pprintf_handle_bool(&pinfo, &jinfo);    break;
                case 'c': pprintf_handle_char(&pinfo, false);     break;
                case 'C': {
                        if (memcmp(fmt_next, "Cc", 2) == 0) { 
                            pprintf_handle_color_clear(&pinfo); 
                            break; 
                        }
                        else if (memcmp(fmt_next, "Cbg", 3) == 0) { 
                            pprintf_handle_background_color(&pinfo);
                            break;
                        }
                        else if (memcmp(fmt_next, "Cfg", 3) == 0) { 
                            pprintf_handle_foreground_color(&pinfo); 
                            break; 
                        }
                    }
                default: {
                        failed = true;
                    }
                }
            }
            if (failed) {
                pstream_write_string(stream, pstring((char*)fmt, pinfo.fmt - fmt));
                fmt = pinfo.fmt + 1;
            }
            fmt = pinfo.fmt + 1;
            printcount = pinfo.count;
        }
    }
    return printcount;
}

#if defined(PSTD_USE_ALLOCATOR)
PIO_STATIC pbinary_string_return_t pmake_binary_string(u64 bitcount, u64 num, pallocator_t *cb) {
#else
PIO_STATIC pbinary_string_return_t pmake_binary_string(u64 bitcount, u64 num) { //NOLINT
#endif
    struct pbinary_string_return_t ret = { 0 };

    u64 bit = 1ULL << (bitcount - 1);
#if defined(PSTD_USE_ALLOCATOR)
    ret.buffer = cb->allocator(cb, ALLOCATE, bitcount + 1, NULL);
#else
    ret.buffer = pallocate(bitcount + 1);
#endif
    for (u64 i = 0; i < bitcount; i++) {
       ret.buffer[i] = '0' + ((num & bit) ? 1 : 0); //NOLINT
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



PIO_STATIC u64 pprintf_print_justified(pgeneric_stream_t *stream, pformatting_specification_t *spec, pstring_t string) {
    s64 space_count = spec->justification_count; 
    s64 zero_count  = spec->zero_justification_count; 
    s64 count = space_count + zero_count;
#if defined(PSTD_USE_ALLOCATOR)
    pallocator_t cb = stream->cb;
#endif

    s64 test = count - (s64)string.length;
    if (test > 0) {
        space_count -= (s64)string.length;
        zero_count  -= (s64)string.length;
        usize space_size = space_count > 0 ? space_count : 1;
        usize zero_size  = zero_count  > 0 ? zero_count  : 1;
#if defined(PSTD_USE_ALLOCATOR)
        char *spaces = cb.allocator(&cb, ALLOCATE, space_size, NULL);
        char *zeros  = cb.allocator(&cb, ALLOCATE, zero_size, NULL);
#else
        char *spaces = pallocate(space_size);
        char *zeros  = pallocate(zero_size);
#endif
        if (space_count > 0)  memset(spaces, ' ', space_count);
        if (zero_count  > 0)  memset(zeros,  '0', zero_count);

        if (PSTD_EXPECT(!spec->right_justified, 1)) {
            if (space_count > 0) pstream_write_string(stream, pstring( spaces, space_count ));
            if (zero_count > 0)  pstream_write_string(stream, pstring( zeros, zero_count ));
            pstream_write_string(stream, string);
        } else {
            if (zero_count > 0)  pstream_write_string(stream, pstring( zeros, zero_count ));
            pstream_write_string(stream, string);
            if (space_count > 0) pstream_write_string(stream, pstring( spaces, space_count ));
        }
#if defined(PSTD_USE_ALLOCATOR)
        cb.allocator(&cb, SIZED_FREE, space_size, spaces);
        cb.allocator(&cb, SIZED_FREE, zero_size, zeros);
#else
        psized_free(space_size, spaces);
        psized_free(zero_size, zeros);
#endif
        return string.length + (u64)test;
    } else {
        pstream_write_string(stream, string);
        return string.length;
    }
}

PIO_STATIC pbool_t pis_rgb_whitespace(char chr) {
     return chr == ' ' 
         || chr == '\t' 
         || chr == '\r' 
         || chr == '\n';
}

PIO_STATIC void pprintf_get_rgb(const char *restrict* fmtptr, pstring_t RGB[3]) {
    const char *restrict fmt = *fmtptr;
    int n = 0;
    while(*fmt != ')') {
        if (n >= 3) break;
        const char *begin = fmt + 1;
        const char *end   = fmt + 1;
        // we check if there is any whitespace that needs to be skipped
        // this allows %Cfg( 255 , 255 , 255 )
        while( pis_rgb_whitespace(*end) ) begin++, end++;
        while( *end >= '0' && *end <= '9') end++;
        RGB[n++] = pstring((char*)begin, (usize)(end - begin) ); 
        while( pis_rgb_whitespace(*end) ) end++;

        fmt = (char *)end;
    }
    *fmtptr = fmt;
}

PIO_STATIC void pprintf_handle_binary(pprintf_info_t *info, pformatting_specification_t *spec) {
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
    pallocator_t *cb = &info->stream->cb;
#endif

    if (spec->length == PFL_128) {
        union {
            struct { u64 high, low; };
            long double ld;
        } conv;
        long double ld = va_arg(*info->list, long double);
        conv.high = 0;
        conv.low  = 0;
        conv.ld = ld;

#if defined(PSTD_USE_ALLOCATOR)
        pbinary_string_return_t high = pmake_binary_string(numbits[PFL_LL], conv.high, cb);
        pbinary_string_return_t low  = pmake_binary_string(numbits[PFL_LL], conv.low,  cb);
#else                                       
        pbinary_string_return_t high = pmake_binary_string(numbits[PFL_LL], conv.high);
        pbinary_string_return_t low  = pmake_binary_string(numbits[PFL_LL], conv.low);
#endif
        if (spec->zero_justification_count == 0 && spec->prefix_zero) {
            spec->zero_justification_count = numbits[PFL_LL];
            info->count += pprintf_print_justified(info->stream, spec, high.str);
            pstream_write_string(info->stream, low.str);
        } else {
            pprintf_print_justified(info->stream, spec, high.str);
            pstream_write_string(info->stream, low.str);
        }

        info->count += low.str.length; 
#if defined(PSTD_USE_ALLOCATOR)
        cb->allocator(cb, SIZED_FREE, numbits[PFL_LL], high.buffer);
        cb->allocator(cb, SIZED_FREE, numbits[PFL_LL], low.buffer);
#else
        psized_free(numbits[PFL_LL], high.buffer);
        psized_free(numbits[PFL_LL], low.buffer);
#endif
    } else {
        u64 num;
        if (PSTD_EXPECT(spec->length == PFL_DEFAULT, 1)) {
            num = va_arg(*info->list, s32);
        }
        else {
            switch (spec->length) {
            case PFL_HH:
            case PFL_H:
            case PFL_L:  num = va_arg(*info->list, u32);       break;
            case PFL_LL: num = va_arg(*info->list, u64);       break;
            case PFL_J:  num = va_arg(*info->list, intmax_t);  break;
            case PFL_Z:              
            case PFL_T:  num = va_arg(*info->list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return;
            }
        }
        
#if defined(PSTD_USE_ALLOCATOR)
        pbinary_string_return_t binary = pmake_binary_string(numbits[ spec->length ], num, cb);
#else
        pbinary_string_return_t binary = pmake_binary_string(numbits[ spec->length ], num);
#endif
        if (spec->zero_justification_count == 0 && spec->prefix_zero) {
            spec->zero_justification_count = binary.iszero ? 0 : numbits[ spec->length ]; 
            info->count += pprintf_print_justified(info->stream, spec, binary.str);
        } else {
            info->count += pprintf_print_justified(info->stream, spec, binary.str);
        }
#if defined(PSTD_USE_ALLOCATOR)
        cb->allocator(cb, SIZED_FREE, numbits[ spec->length ], binary.buffer);
#else
        psized_free(numbits[ spec->length ], binary.buffer);
#endif
    }
}

PIO_STATIC void pprintf_handle_background_color(pprintf_info_t *info) {
        info->fmt += 3;
        if (*info->fmt != '(') return;
        
        pstring_t RGB[3];
        pprintf_get_rgb(&info->fmt, RGB);
    if (PSTD_EXPECT(PIO_GLOBALS.colored_output, 1)) {
        static const pstring_t header = pcreate_const_string("\x1b[48;2;");
        pstream_write_string(info->stream, header);
        pstream_write_string(info->stream, RGB[0]);
        pstream_write_char(info->stream, ';');
        pstream_write_string(info->stream, RGB[1]);
        pstream_write_char(info->stream, ';');
        pstream_write_string(info->stream, RGB[2]);
        pstream_write_char(info->stream, 'm');
        info->count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }
}

PIO_STATIC void pprintf_handle_foreground_color(pprintf_info_t *info) {
    info->fmt += 3;
    if (*info->fmt != '(') return;
    pstring_t RGB[3];
    pprintf_get_rgb(&info->fmt, RGB);

    if (PSTD_EXPECT(PIO_GLOBALS.colored_output, 1)) {
        static const pstring_t header = pcreate_const_string("\x1b[38;2;");
        pstream_write_string(info->stream, header);
        pstream_write_string(info->stream, RGB[0]);
        pstream_write_char(info->stream, ';');
        pstream_write_string(info->stream, RGB[1]);
        pstream_write_char(info->stream, ';');
        pstream_write_string(info->stream, RGB[2]);
        pstream_write_char(info->stream, 'm');
        info->count += RGB[0].length 
            +  RGB[1].length 
            +  RGB[2].length
            +  header.length 
            +  3;
    }
}

PIO_STATIC void pprintf_handle_char(pprintf_info_t *info, pbool_t wide) {
    if (PSTD_EXPECT(wide == false, 1)){
        int character = va_arg(*info->list, int);
        pstream_write_char(info->stream, (char)character);
        info->count++;
    } else {
        char *character = va_arg(*info->list, char *);
        u32 len = pget_utf8_length(character);
        pstream_write_string(info->stream, pstring( character, len));
        info->count += len;
    }
}

PIO_STATIC void pprintf_handle_string(pprintf_info_t *info, pformatting_specification_t *spec, pbool_t cstring) {
    pstring_t str;
    if (cstring){
        char *c_str = va_arg(*info->list, char *);
        str = pstring(c_str, strlen(c_str) );
    } else { 
        str = va_arg(*info->list, pstring_t); 
    }

    if (spec->zero_justification_count) {
        str.length = spec->zero_justification_count;
        spec->zero_justification_count = 0;
    }
    info->count += pprintf_print_justified(info->stream, spec, str);
}

PIO_STATIC void pprintf_handle_color_clear(pprintf_info_t *info) {
    static const pstring_t reset = pcreate_const_string("\x1b[0m");
    if (PSTD_EXPECT(PIO_GLOBALS.colored_output, 1)) {
        pstream_write_string(info->stream, reset);
        info->count += reset.length; 
    }
    info->fmt++;
}

PIO_STATIC void pprintf_handle_hash(pprintf_info_t *info, pformatting_specification_t *spec) {
    spec->alternative_form = true;
    info->fmt++;

    switch(*info->fmt) {
    case 'h': case 'l':
    case 'j': case 'z':
    case 't': case 'L': return pprintf_handle_length(info, spec); break;
    case '-': return pprintf_handle_minus(info, spec);
    case '0': return pprintf_handle_zero(info, spec);
    case '.': return pprintf_handle_dot(info, spec);
    case 'p': return pprintf_handle_pointer(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pprintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);
    case 'o':
    case 'x': case 'X': pprintf_handle_int(info, spec, *info->fmt);
    case 'B': pprintf_handle_bool(info, spec);
    default: return;
    }
}

PIO_STATIC void pprintf_handle_number(pprintf_info_t *info, pformatting_specification_t *spec) { 
    const char *restrict begin = info->fmt;
    char *end;
    if (*info->fmt != '*')
        spec->justification_count = strtoul(begin, &end, BASE_10);
    else {
        spec->justification_count = va_arg(*info->list, int);
        end = (char*)info->fmt+1;
    }
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 

    switch (*info->fmt) {
    case '.': return pprintf_handle_dot(info, spec);
    case 's': case 'S': return pprintf_handle_string(info, spec, *info->fmt == 's' ? true : false);
    
    case 'h': case 'l':
    case 'j': case 'z':
    case 't': case 'L': return pprintf_handle_length(info, spec); break;

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
    case 'p': return pprintf_handle_pointer(info, spec);
    case 'b': return pprintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pprintf_handle_bool(pprintf_info_t *info, pformatting_specification_t *spec) {
    static const pstring_t strings[] = {
        pcreate_const_string("false"),
        pcreate_const_string("true"),
        pcreate_const_string("FALSE"),
        pcreate_const_string("TRUE"),
    };
    u64 num = 0;
    if ( PSTD_EXPECT( spec->length == PFL_DEFAULT, 1) ) {
        num = va_arg(*info->list, s32);
    } else {
        switch(spec->length) {
            case PFL_HH: 
            case PFL_H: 
            case PFL_L:  num = va_arg(*info->list, u32);       break;
            case PFL_LL: num = va_arg(*info->list, u64);       break;
            case PFL_J:  num = va_arg(*info->list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(*info->list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return;
        }
    }

    pstring_t output = strings[(num != 0) + (2*spec->alternative_form)];
    pstream_write_string(info->stream, output);
    info->count += output.length;
}


PIO_STATIC void pprintf_handle_dot(pprintf_info_t *info, pformatting_specification_t *spec) { 
    const char *restrict begin = info->fmt + 1;
    char *end;
    if (*begin >= '0' && *begin <= '9')
        spec->zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec->zero_justification_count = va_arg(*info->list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info->fmt; 
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 
    spec->prefix_zero = true; 

    switch (*info->fmt) {
    case '.': return pprintf_handle_dot(info, spec);
    case 's': case 'S': return pprintf_handle_string(info, spec, *info->fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pprintf_handle_length(info, spec);

    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
    case 'p': return pprintf_handle_pointer(info, spec);
    case 'b': return pprintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pprintf_handle_zero(pprintf_info_t *info, pformatting_specification_t *spec) { 
    const char *restrict begin = info->fmt + 1;
    char *end;
    if (*begin >= '1' && *begin <= '9')
        spec->zero_justification_count = strtoul(begin, &end, BASE_10);
    else if (*begin == '*'){
        spec->zero_justification_count = va_arg(*info->list, int);
        end = (char*)begin+1;
    } 
    else end = (char*)info->fmt + 1; 
    // maybe fmt_next = ++end; not sure tbh
    info->fmt = end; 
    spec->prefix_zero = true; 

    switch (*info->fmt) {
    case '.': return pprintf_handle_dot(info, spec);
    case 's': case 'S': return pprintf_handle_string(info, spec, *info->fmt == 's' ? true : false);

    case 'h': case 'l':
    case 'j': case 'z': 
    case 't': case 'L': return pprintf_handle_length(info, spec);
    
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);

    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
    case 'p': return pprintf_handle_pointer(info, spec);
    case 'b': return pprintf_handle_binary(info, spec);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pprintf_handle_space(pprintf_info_t *info, pformatting_specification_t *spec) { 
    while (*info->fmt == ' ') info->fmt++;

    switch( *info->fmt ) {
    case '-': return pprintf_handle_minus(info, spec);
    case '0': return pprintf_handle_zero(info, spec);
    case '.': return pprintf_handle_dot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pprintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);
    case 'o':
    case 'x': case 'X': pprintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pprintf_handle_length(pprintf_info_t *info, pformatting_specification_t *spec) { 
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
        case 'g': case 'G': return pprintf_handle_float(info, spec);

        case 'u': case 'o': 
        case 'x': case 'X':
        case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
        case 'b': return pprintf_handle_binary(info, spec);
        case 'B': return pprintf_handle_bool(info, spec);
        case 'n': return pprintf_handle_characters_written(info, spec);
        case '#': return pprintf_handle_hash(info, spec);
        default: *info->failflag = true; return;
        }
    } else {
        return pprintf_handle_char(info, true);
    }
}

PIO_STATIC void pprintf_handle_plus(pprintf_info_t *info, pformatting_specification_t *spec) { 
    spec->force_sign = true;
    
    info->fmt++;
    switch( *info->fmt ) {
    case '-': return pprintf_handle_minus(info, spec);
    case '0': return pprintf_handle_zero(info, spec);
    case '.': return pprintf_handle_dot(info, spec);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pprintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}

PIO_STATIC void pprintf_handle_minus(pprintf_info_t *info, pformatting_specification_t *spec) { 
    spec->right_justified = true; 
    
    info->fmt++;
    switch( *info->fmt ) {
    case '+': return pprintf_handle_plus(info, spec);
    case '0': return pprintf_handle_zero(info, spec);
    case '.': return pprintf_handle_dot(info, spec);
    case '#': return pprintf_handle_hash(info, spec);
    case 's': case 'S': return pprintf_handle_string(info, spec, *info->fmt == 's' ? true : false);
    case '1': case '2':
    case '3': case '4':
    case '5': case '6':
    case '7': case '8':
    case '9': case '*': return pprintf_handle_number(info, spec);
    case 'f': case 'F':
    case 'e': case 'E':
    case 'a': case 'A':
    case 'g': case 'G': return pprintf_handle_float(info, spec);
    case 'u': case 'o': 
    case 'x': case 'X':
    case 'i': case 'd': return pprintf_handle_int(info, spec, *info->fmt);
    default: *info->failflag = true; 
    }
}


PIO_STATIC void pprintf_handle_signedint(
        pprintf_info_t *info, pformatting_specification_t *spec, s64 num, pbool_t always_print_sign);
PIO_STATIC void pprintf_handle_octalint(
        pprintf_info_t *info, pformatting_specification_t *spec, s64 num, pbool_t always_print_sign);
PIO_STATIC void pprintf_handle_hexadecimalint(
        pprintf_info_t *info, pformatting_specification_t *spec, u64 num, 
        pbool_t always_print_sign, pbool_t uppercase);
PIO_STATIC void pprintf_handle_unsignedint(
        pprintf_info_t *info, pformatting_specification_t *spec, u64 num, pbool_t always_print_sign);

PIO_STATIC void pprintf_handle_int(pprintf_info_t *info, pformatting_specification_t *spec, char printtype) {
    u64 num = 0;

    if ( PSTD_EXPECT( spec->length == PFL_DEFAULT, 1) ) {
        num = va_arg(*info->list, s32);
    } else {
        switch(spec->length) {
            case PFL_HH: 
            case PFL_H: 
            case PFL_L:  num = va_arg(*info->list, u32);       break;
            case PFL_LL: num = va_arg(*info->list, u64);       break;
            case PFL_J:  num = va_arg(*info->list, intmax_t);  break;
            case PFL_Z:
            case PFL_T:  num = va_arg(*info->list, usize); break;
            case PFL_DEFAULT: case PFL_128:
            default: return;
        }
    }

    switch (printtype) {
    case 'd': case 'i': return pprintf_handle_signedint(info, spec, (s64)num, spec->force_sign);
    case 'o':           return pprintf_handle_octalint( info, spec, (s64)num, spec->force_sign); 
    case 'x': case 'X': 
        return pprintf_handle_hexadecimalint(info, spec, (s64)num, spec->force_sign, printtype == 'X');
    case 'u': return pprintf_handle_unsignedint(info, spec, (s64)num, spec->force_sign);
    default: break;
    }

}

#define STRING_BUFFER_SIZE 25

PIO_STATIC 
void pprintf_handle_signedint(
        pprintf_info_t *info, pformatting_specification_t *spec, s64 num, pbool_t always_print_sign) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];

    count = psigned_decimal_to_string(buf, num);
    char *printbuf = buf;
    if (num && num > 0 && !always_print_sign) { printbuf++; count--; }
    info->count += pprintf_print_justified(info->stream, spec, pstring( printbuf, count ));
}

PIO_STATIC 
void pprintf_handle_unsignedint(
        pprintf_info_t *info, pformatting_specification_t *spec, u64 num, pbool_t always_print_sign) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = punsigned_decimal_to_string(buf, num);
    
    if (always_print_sign)
        pstream_write_char(info->stream, '+');

    info->count += pprintf_print_justified(info->stream, spec, pstring( buf, count )) + 1;
}

PIO_STATIC 
void pprintf_handle_octalint(
        pprintf_info_t *info, pformatting_specification_t *spec, s64 num, pbool_t always_print_sign)
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = psigned_octal_to_string(buf, num);
    
    char *printbuf = buf;
    if (num > 0 && !always_print_sign) { printbuf++; count--; }
    
    if (spec->alternative_form && num != 0) {
        static const pstring_t str = pcreate_const_string("0o");
        if (spec->prefix_zero) {
            u32 zeros = spec->zero_justification_count;
            spec->zero_justification_count = 0;
            info->count += pprintf_print_justified(info->stream, spec, str) + 1;
            spec->justification_count = 0; 
            spec->zero_justification_count = zeros;
            pprintf_print_justified( info->stream, spec, pstring( printbuf, count ) );
        } else {
            info->count += pprintf_print_justified(info->stream, spec, str) + 1;
            info->count += count;
            pstream_write_string( info->stream, pstring( printbuf, count ) );
        }
    }
    else {
        info->count += pprintf_print_justified(info->stream, spec, pstring( printbuf, count )) + 1;
    }
}

PIO_STATIC 
void pprintf_handle_hexadecimalint(
        pprintf_info_t *info, pformatting_specification_t *spec, 
        u64 num, pbool_t always_print_sign, pbool_t uppercase) 
{
    u32 count;
    char buf[STRING_BUFFER_SIZE];
    count = punsigned_hex_to_string(buf, num);
   
    if (uppercase) {
        for (u32 i = 0; i < count + 1; i++) {
            buf[i] = toupper(buf[i]);
        }
    }

    if (always_print_sign)
        pstream_write_char(info->stream, '+');

    if (spec->alternative_form && num != 0) {
        static const pstring_t str[2] = { pcreate_const_string("0x"), pcreate_const_string("0X") };
        if (spec->prefix_zero) {
            u32 zeros = spec->zero_justification_count;
            spec->zero_justification_count = 0;
            info->count += pprintf_print_justified(info->stream, spec, str[uppercase]) + 1;
            spec->justification_count = 0; 
            spec->zero_justification_count = zeros;
            pprintf_print_justified( info->stream, spec, pstring( buf, count ) );
        } else {
            info->count += pprintf_print_justified(info->stream, spec, str[uppercase]) + 1;
            info->count += count;
            pstream_write_string( info->stream, pstring( buf, count ) );
        }
    }
    else {
        info->count += pprintf_print_justified(info->stream, spec, pstring( buf, count )) + 1;
    }
}

//TODO: this is stupid and needs replacing
PIO_STATIC void pprintf_handle_float(pprintf_info_t *info, pformatting_specification_t *spec) {
    (void)spec;
#if PSTD_HAS_VLA
#else
    pallocator_t *cb = &info->stream.cb;
#endif
    f64 value = va_arg(*info->list, f64);
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
    count = snprintf(NULL, 0, buf, value);
#if PSTD_HAS_VLA
    char out[count + 1];
#else
    char *out = cb->allocator(cb, ALLOCATE, count + 1, NULL);
#endif
    
    info->count += snprintf(out, count, buf, value);
    pstream_write_string(info->stream, pstring(out, count - 1));
#if defined(PSTD_GNU_COMPATIBLE)
#else
    cb->allocator(cb, FREE, count + 1, out);
#endif
}

PIO_STATIC void pprintf_handle_pointer(pprintf_info_t *info, pformatting_specification_t *spec) {
#if defined(PSTD_32)
    spec->length = PFL_DEFAULT;
#elif defined(PSTD_64)
    spec->length = PFL_LL;
#else
#error neither 32 or 64 bit!
#endif

    pbool_t uppercase = spec->alternative_form;
    spec->alternative_form = true; 

    void *ptr = va_arg(*info->list, void *);
    if (!ptr) {
        static const pstring_t null = pcreate_const_string("nullptr");
        pstream_write_string(info->stream, null);
        info->count += null.length;
        return;
    }
    return pprintf_handle_hexadecimalint(info, spec, (usize)ptr, false, uppercase);
}

PIO_STATIC void pprintf_handle_characters_written(pprintf_info_t *info, pformatting_specification_t *spec) {
    if ( PSTD_EXPECT( spec->length == PFL_DEFAULT, 1) ) {
        s32 *count = va_arg(*info->list, s32*);
        *count = (s32)info->count;
    } else {
        switch(spec->length) {
            case PFL_HH: {  u8  *count = va_arg(*info->list, u8 *); *count = info->count; } break;  
            case PFL_H:  {  u16 *count = va_arg(*info->list, u16*); *count = info->count; } break; 
            case PFL_L:  {  u32 *count = va_arg(*info->list, u32*); *count = info->count; } break; 
            case PFL_LL: {  u64 *count = va_arg(*info->list, u64*); *count = info->count; } break; 
            case PFL_J:  {  u64 *count = va_arg(*info->list, u64*); *count = info->count; } break; 
            case PFL_Z:  {  u64 *count = va_arg(*info->list, u64*); *count = info->count; } break; 
            case PFL_T:  {  u64 *count = va_arg(*info->list, u64*); *count = info->count; } break; 
            case PFL_DEFAULT: case PFL_128:
            default: break;
        }
    }
}


// CUSTOM FORMATTING

void pformat_push_impl(pstring_t fmt, pformat_callback_t *callback) {
    if (psb_size(callbacks)) {
        for (usize i = 0; i < psb_size(callbacks); i++)
            if (pcmp_string(fmt, callbacks[i].format))
                return;
    }
    struct puser_callback_t usercb = { fmt, callback };
    psb_pushback(callbacks, usercb);
}

void pformat_push_adv_impl(pstring_t fmt, pformat_callback_adv_t *callback) {
    if (psb_size(advcallbacks)) {
        for (usize i = 0; i < psb_size(advcallbacks); i++)
            if (pcmp_string(fmt, advcallbacks[i].format))
                return;
    }
    struct padv_user_callback_t usercb = { fmt, callback };
    psb_pushback(advcallbacks, usercb);
}

void pformat_pop_impl(pstring_t fmt) {
    struct puser_callback_t *remove = NULL;
    for (usize i = 0; i < psb_size(callbacks); i++)
        if (pcmp_string(fmt, callbacks[i].format))
            remove = callbacks + i;
    
    if (remove) psb_remove(callbacks, remove);
}

void pformat_pop_adv_impl(pstring_t fmt) {
    struct padv_user_callback_t *remove = NULL;
    for (usize i = 0; i < psb_size(advcallbacks); i++)
        if (pcmp_string(fmt, advcallbacks[i].format))
            remove = advcallbacks + i;
    
    if (remove) psb_remove(advcallbacks, remove);
}
