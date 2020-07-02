#include "pstring.h"
#include "allocator.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

extern Allocator *pCurrentAllocatorFunc;
extern void *pCurrentAllocatorUserData;

bool pStringCmp(const struct String *rhs, const struct String *lhs) {
	if (rhs->length != lhs->length) return false;
	return (strncmp((const char *)rhs->c_str, (const char *)lhs->c_str, rhs->length) == 0);
}

struct String pStringCopy(const struct String str) {
    char *dst = pCurrentAllocatorFunc(NULL, sizeof(char) * str.length, 0, MALLOC, pCurrentAllocatorUserData);
    struct String r = { (u8 *)dst, str.length };
    memcpy(dst, str.c_str, sizeof(char) * str.length);
	return r;
}
/*
void pStringStreamPush(struct StringStream *ss, const char *str) {
    u64 size = ss->buffer_size + strlen(str);
    ss->c_str = realloc((char *)ss->c_str, size + 1);
    ss->c_str[size] = '\0';  
    ss->c_str = (u8 *)strcat((char *)ss->c_str, str);
    ss->buffer_size = size + 1;
}

void pStringStreamPushf(struct StringStream *ss, const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
        u64 count = (unsigned)vsnprintf(NULL, 0, fmt, list);    
        char buffer[count + 1];
        vsprintf(buffer, fmt, list);
        //vsnprintf(buffer, count, fmt, list);
        buffer[count] = '\0';
    va_end(list);

    if (ss->c_str) {
        u64 outsize = count + ss->buffer_size; 
        char copybuffer[outsize + 1];
        sprintf(copybuffer, "%s%s", ss->c_str, buffer);
        ss->c_str = realloc(ss->c_str, outsize + 1);
        memcpy_s(ss->c_str, outsize + 1, copybuffer, outsize + 1);
        ss->c_str[outsize] = '\0';
        ss->buffer_size = outsize + 1;
    }
    else{
        ss->c_str = malloc(count + 1);
        memcpy_s(ss->c_str, count +1, buffer, count + 1);
        ss->c_str[count] = '\0';
        ss->buffer_size = count + 1;
    } 
}
*/

#pragma clang diagnostic pop

