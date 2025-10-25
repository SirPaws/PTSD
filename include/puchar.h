#pragma once
#ifndef PTSD_UCHAR_HEADER
#define PTSD_UCHAR_HEADER
#include "general.h"
#include <limits.h>
#include <uchar.h>
#if !(defined(__STDC_VERSION_UCHAR_H__) && __STDC_VERSION_UCHAR_H__ >= 202311L)
typedef unsigned char char8_t;
#endif

typedef s32 pcodepoint_t;
typedef unsigned _BitInt(8) pchar8_t;
typedef unsigned _BitInt(16) pchar16_t;
typedef unsigned _BitInt(32) pchar32_t;

#define pu_internal_with_value_(name, value, ...) ({ pcodepoint_t name = value; __VA_ARGS__ })
#define pu_internal_is_ascii(c, true_branch, false_branch) \
    pu_internal_with_value_(pu_is_ascii_c, (c), \
        pu_is_ascii_c > 127 ? (false_branch) : (true_branch); \
    )

#define pu_is_ascii_whitespace(c)\
    (pu_is_ascii_whitespace[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_control(c)\
    (pu_is_ascii_control[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x20))])
#define pu_is_ascii_print(c)\
    (pu_is_ascii_print[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_blank(c)\
    (pu_is_ascii_blank[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_graphic(c)\
    (pu_is_ascii_graphic[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_punctuation(c)\
    (pu_is_ascii_punctuation[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_alphanumeric(c)\
    (pu_is_ascii_alphanumeric[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_alphabetic(c)\
    (pu_is_ascii_alphabetic[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_upper(c)\
    (pu_is_ascii_upper[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_lower(c)\
    (pu_is_ascii_lower[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_digit(c)\
    (pu_is_ascii_digit[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])
#define pu_is_ascii_digit(c) pu_is_digit(c)
#define pu_is_ascii_hexadecimal_digit(c)\
    (pu_is_ascii_hexadecimal_digit[(usize)pu_internal_is_ascii((c), (pu_is_ascii_c), (0x00))])


bool pu_is_whitespace(pcodepoint_t);

static constexpr bool pu_is_ascii_control[UCHAR_MAX] = {
    [ 0] = true, [ 1] = true, [ 2] = true, [ 3] = true, [ 4] = true,
    [ 5] = true, [ 6] = true, [ 7] = true, [ 8] = true, [ 9] = true, 
    [10] = true, [11] = true, [12] = true, [13] = true, [14] = true, 
    [15] = true, [16] = true, [17] = true, [18] = true, [19] = true, 
    [20] = true, [21] = true, [22] = true, [23] = true, [24] = true, 
    [25] = true, [26] = true, [27] = true, [28] = true, [29] = true, 
    [30] = true, [31] = true, [127] = true
};

static constexpr bool pu_is_ascii_print[UCHAR_MAX] = {
    [ 33] = true, [ 34] = true, [ 35] = true, [ 36] = true, [ 37] = true,
    [ 38] = true, [ 39] = true, [ 40] = true, [ 41] = true, [ 42] = true, 
    [ 43] = true, [ 44] = true, [ 45] = true, [ 46] = true, [ 47] = true, 
    [ 48] = true, [ 49] = true, [ 50] = true, [ 51] = true, [ 52] = true,
    [ 53] = true, [ 54] = true, [ 55] = true, [ 56] = true, [ 57] = true, 
    [ 58] = true, [ 59] = true, [ 60] = true, [ 61] = true, [ 62] = true, 
    [ 63] = true, [ 64] = true, [ 65] = true, [ 66] = true, [ 67] = true, 
    [ 68] = true, [ 69] = true, [ 70] = true, [ 71] = true, [ 72] = true, 
    [ 73] = true, [ 74] = true, [ 75] = true, [ 76] = true, [ 77] = true, 
    [ 78] = true, [ 79] = true, [ 80] = true, [ 81] = true, [ 82] = true, 
    [ 83] = true, [ 84] = true, [ 85] = true, [ 86] = true, [ 87] = true, 
    [ 88] = true, [ 89] = true, [ 90] = true, [ 91] = true, [ 92] = true, 
    [ 93] = true, [ 94] = true, [ 95] = true, [ 96] = true, [ 97] = true, 
    [ 98] = true, [ 99] = true, [100] = true, [101] = true, [102] = true, 
    [103] = true, [104] = true, [105] = true, [106] = true, [107] = true, 
    [108] = true, [109] = true, [110] = true, [111] = true, [112] = true, 
    [113] = true, [114] = true, [115] = true, [116] = true, [117] = true, 
    [118] = true, [119] = true, [120] = true, [121] = true, [122] = true, 
    [123] = true, [124] = true, [125] = true, [126] = true, 
};

static const bool pu_is_ascii_whitespace[UCHAR_MAX] = {
    ['\t'] = true, ['\n'] = true, ['\v'] = true,
    ['\f'] = true, ['\r'] = true, [ ' '] = true
};

static constexpr bool pu_is_ascii_blank[UCHAR_MAX] = {
    ['\t'] = true, [' '] = true
};

static constexpr bool pu_is_ascii_graphic[UCHAR_MAX] = {
    [ 33] = true, [ 34] = true, [ 35] = true, [ 36] = true,
    [ 37] = true, [ 38] = true, [ 39] = true, [ 40] = true, 
    [ 41] = true, [ 42] = true, [ 43] = true, [ 44] = true, 
    [ 45] = true, [ 46] = true, [ 47] = true, [ 48] = true, 
    [ 49] = true, [ 50] = true, [ 51] = true, [ 52] = true, 
    [ 53] = true, [ 54] = true, [ 55] = true, [ 56] = true, 
    [ 57] = true, [ 58] = true, [ 59] = true, [ 60] = true, 
    [ 61] = true, [ 62] = true, [ 63] = true, [ 64] = true, 
    [ 65] = true, [ 66] = true, [ 67] = true, [ 68] = true, 
    [ 69] = true, [ 70] = true, [ 71] = true, [ 72] = true, 
    [ 73] = true, [ 74] = true, [ 75] = true, [ 76] = true, 
    [ 77] = true, [ 78] = true, [ 79] = true, [ 80] = true, 
    [ 81] = true, [ 82] = true, [ 83] = true, [ 84] = true, 
    [ 85] = true, [ 86] = true, [ 87] = true, [ 88] = true, 
    [ 89] = true, [ 90] = true, [ 91] = true, [ 92] = true, 
    [ 93] = true, [ 94] = true, [ 95] = true, [ 96] = true, 
    [ 97] = true, [ 98] = true, [ 99] = true, [100] = true, 
    [101] = true, [102] = true, [103] = true, [104] = true, 
    [105] = true, [106] = true, [107] = true, [108] = true, 
    [109] = true, [110] = true, [111] = true, [112] = true, 
    [113] = true, [114] = true, [115] = true, [116] = true, 
    [117] = true, [118] = true, [119] = true, [120] = true, 
    [121] = true, [122] = true, [123] = true, [124] = true, 
    [125] = true, [126] = true, 
};

static constexpr bool pu_is_ascii_punctuation[UCHAR_MAX] = {
    [ 33] = true, [ 34] = true, [ 35] = true, [ 36] = true,
    [ 37] = true, [ 38] = true, [ 39] = true, [ 40] = true,
    [ 41] = true, [ 42] = true, [ 43] = true, [ 44] = true, 
    [ 45] = true, [ 46] = true, [ 47] = true, [ 58] = true, 
    [ 59] = true, [ 60] = true, [ 61] = true, [ 62] = true, 
    [ 63] = true, [ 64] = true, [ 91] = true, [ 92] = true, 
    [ 93] = true, [ 94] = true, [ 95] = true, [ 96] = true, 
    [123] = true, [124] = true, [125] = true, [126] = true, 
};

static constexpr bool pu_is_ascii_alphanumeric[UCHAR_MAX] = {
    ['0'] = true, ['1'] = true, ['2'] = true, ['3'] = true,
    ['4'] = true, ['5'] = true, ['6'] = true, ['7'] = true, 
    ['8'] = true, ['9'] = true,

    ['A'] = true, ['B'] = true, ['C'] = true, ['D'] = true,
    ['E'] = true, ['F'] = true, ['G'] = true, ['H'] = true, 
    ['I'] = true, ['J'] = true, ['K'] = true, ['L'] = true, 
    ['M'] = true, ['N'] = true, ['O'] = true, ['P'] = true, 
    ['Q'] = true, ['R'] = true, ['S'] = true, ['T'] = true, 
    ['U'] = true, ['V'] = true, ['W'] = true, ['X'] = true, 
    ['Y'] = true, ['Z'] = true,  	
    
    ['a'] = true, ['b'] = true, ['c'] = true, ['d'] = true, 
    ['e'] = true, ['f'] = true, ['g'] = true, ['h'] = true, 
    ['i'] = true, ['j'] = true, ['k'] = true, ['l'] = true, 
    ['m'] = true, ['n'] = true, ['o'] = true, ['p'] = true, 
    ['q'] = true, ['r'] = true, ['s'] = true, ['t'] = true, 
    ['u'] = true, ['v'] = true, ['w'] = true, ['x'] = true, 
    ['y'] = true, ['z'] = true
};

static constexpr bool pu_is_ascii_alphabetic[UCHAR_MAX] = {
    ['A'] = true, ['B'] = true, ['C'] = true, ['D'] = true,
    ['E'] = true, ['F'] = true, ['G'] = true, ['H'] = true, 
    ['I'] = true, ['J'] = true, ['K'] = true, ['L'] = true, 
    ['M'] = true, ['N'] = true, ['O'] = true, ['P'] = true, 
    ['Q'] = true, ['R'] = true, ['S'] = true, ['T'] = true, 
    ['U'] = true, ['V'] = true, ['W'] = true, ['X'] = true, 
    ['Y'] = true, ['Z'] = true,  	
    
    ['a'] = true, ['b'] = true, ['c'] = true, ['d'] = true, 
    ['e'] = true, ['f'] = true, ['g'] = true, ['h'] = true, 
    ['i'] = true, ['j'] = true, ['k'] = true, ['l'] = true, 
    ['m'] = true, ['n'] = true, ['o'] = true, ['p'] = true, 
    ['q'] = true, ['r'] = true, ['s'] = true, ['t'] = true, 
    ['u'] = true, ['v'] = true, ['w'] = true, ['x'] = true, 
    ['y'] = true, ['z'] = true
};

static constexpr bool pu_is_ascii_upper[UCHAR_MAX] = {
    ['A'] = true, ['B'] = true, ['C'] = true, ['D'] = true,
    ['E'] = true, ['F'] = true, ['G'] = true, ['H'] = true, 
    ['I'] = true, ['J'] = true, ['K'] = true, ['L'] = true, 
    ['M'] = true, ['N'] = true, ['O'] = true, ['P'] = true, 
    ['Q'] = true, ['R'] = true, ['S'] = true, ['T'] = true, 
    ['U'] = true, ['V'] = true, ['W'] = true, ['X'] = true, 
    ['Y'] = true, ['Z'] = true,  	
};

static constexpr bool pu_is_ascii_lower[UCHAR_MAX] = {
    ['a'] = true, ['b'] = true, ['c'] = true, ['d'] = true, 
    ['e'] = true, ['f'] = true, ['g'] = true, ['h'] = true, 
    ['i'] = true, ['j'] = true, ['k'] = true, ['l'] = true, 
    ['m'] = true, ['n'] = true, ['o'] = true, ['p'] = true, 
    ['q'] = true, ['r'] = true, ['s'] = true, ['t'] = true, 
    ['u'] = true, ['v'] = true, ['w'] = true, ['x'] = true, 
    ['y'] = true, ['z'] = true
};

static constexpr bool pu_is_ascii_digit[UCHAR_MAX] = {
    ['0'] = true, ['1'] = true, ['2'] = true, ['3'] = true,
    ['4'] = true, ['5'] = true, ['6'] = true, ['7'] = true, 
    ['8'] = true, ['9'] = true,
};

static constexpr bool pu_is_ascii_hexadecimal_digit[UCHAR_MAX] = {
    ['0'] = true, ['1'] = true, ['2'] = true, ['3'] = true,
    ['4'] = true, ['5'] = true, ['6'] = true, ['7'] = true, 
    ['8'] = true, ['9'] = true, ['A'] = true, ['B'] = true, 
    ['C'] = true, ['D'] = true, ['E'] = true, ['F'] = true,
    ['a'] = true, ['b'] = true, ['c'] = true, ['d'] = true, 
    ['e'] = true, ['f'] = true
};

#endif // PTSD_UCHAR_HEADER
