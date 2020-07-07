#pragma once

#include "general.h"

#if PLANG_C

#define pCreateString(str) (String){ .c_str = (u8 *)(str), .length = sizeof((str)) - 1 }

typedef struct String String;
struct String {
	u8 *c_str;
    usize length;
};

typedef struct StringSpan StringSpan;
struct StringSpan {
    u8 *begin, *end;
};

bool pStringCmp(const String rhs, const String lhs);
String pStringCopy(const String str);
// void pStringStreamPush(struct StringStream *ss, const char *str);
// void pStringStreamPushf(struct StringStream *ss, const char *fmt, ...);
#endif

#if PLANG_CPP
#include <string.h>

struct String {
	const char* c_str = nullptr;
	u64 length = 0;
	String(const char* string, u64 length) :c_str(string), length(length) {}
	template<u64 count>
	constexpr String(const char(&string)[count]) : c_str(string), length(count - 1) {}

	String() {}
	bool operator== (const String& other) {
		if (length == other.length) return strncmp(c_str, other.c_str, length) == 0;
		return false;
	}
};
#endif

