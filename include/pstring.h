#pragma once
#ifndef PSTD_PSTRING_HEADER
#define PSTD_PSTRING_HEADER
#include "general.h"

#define pCreateString(str) (String){ .c_str = (u8 *)(str), .length = sizeof((str)) - 1 }

typedef struct String String;
struct String {
    usize length;
	u8 *c_str;
};

typedef struct StringSpan StringSpan;
struct StringSpan {
    u8 *begin, *end;
};

bool pStringCmp(String rhs, String lhs);
String pStringCopy(String str);

PSTD_MAYBE_UNUSED
static String pString(u8 *c_str, usize length) {
    return (String){ length, c_str };
}

// reallocates str 
String pStringAppendCharacter(String *str, u8 character);
String pStringAppendString(String *str, String string);
#define pStringAppend(str, value) \
    _Generic((value), String: pStringAppendString, default: pStringAppendCharacter)(str, value) 

// void pStringStreamPush(struct StringStream *ss, const char *str);
// void pStringStreamPushf(struct StringStream *ss, const char *fmt, ...);
#endif // PSTD_PSTRING_HEADER
