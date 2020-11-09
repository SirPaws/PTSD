#include "pstring.h"
#include "allocator.h"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

bool pStringCmp(const struct String rhs, const struct String lhs) {
	if (rhs.length != lhs.length) return false;
	return (strncmp((const char *)rhs.c_str, (const char *)lhs.c_str, rhs.length) == 0);
}

struct String pStringCopy(const struct String str) {
    char *dst = pAllocateBuffer(sizeof(char) * str.length);
    struct String r = pString((u8 *)dst, str.length);
    memcpy(dst, str.c_str, sizeof(char) * str.length);
	return r;
}

String pStringAppendCharacter(String *str, u8 character) {
    void *tmp = pReallocateBuffer(str->c_str, str->length + 1);
    str->c_str = tmp;
    str->c_str[str->length++] = character;
    return *str;
}

String pStringAppendString(String *str, String string) {
    void* tmp = pReallocateBuffer(str->c_str, str->length + string.length);
    str->c_str = tmp;

    memcpy(str->c_str + str->length, string.c_str, string.length);
    str->length += string.length;
    return *str;
}
