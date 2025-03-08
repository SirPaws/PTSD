#define PTSD_UTF8_STANDALONE
#include "putf8.h"
#include <threads.h>

int main(void) {
    putf8_t *string0 = putf8("ålborg");
    putf8_t *string1 = putf8(u8"ålborg");
    putf8_t *string2 = putf8(string0);
    putf8_t *string3 = putf8(u8"🏳️‍⚧️");

    putf8_t *string4 = putf8(u8"\U0001F469\U0001F3FD\U0000200D\U00002764\U0000FE0F\U0000200D\U0001F48B\U0000200D\U0001F468\U0001F3FB");
    putf8_meta_t *meta = ((putf8_meta_t *)string0) - 1;
    printf("cstr length: %zu\n", putf8_len("hi"));
    printf("cstr length: %zu\n", putf8_len(u8"hi"));
    printf(" str length: %zu\n", putf8_len(string0));
    printf(" str length: %zu\n", putf8_len(string0));
    printf("%s: (size: %zu, length: %zu)\n", (char*)string0, meta->size, meta->length);

    printf("%6s: ", (char*)string0);
    putf8_char_iter_t iter = putf8_chars(string0);
    for (pcodepoint_t chr = putf8_char_next(&iter); chr; chr = putf8_char_next(&iter)) {
        printf("U+%05x ", chr);
    }
    printf("\n");
    printf("%6s : ", (char*)string3);
    iter = putf8_chars(string3);
    for (pcodepoint_t chr = putf8_char_next(&iter); chr; chr = putf8_char_next(&iter)) {
        printf("U+%05x ", chr);
    }
    
    #define abc(a, ...) a

    printf("\n");
    printf("%6s : ", (char*)string4);
    iter = putf8_chars(string4);
    putf8_foreach(&iter, chr) {
        printf("`u{%05x}", chr);
    }

    putf8_free(string0);
    putf8_free(string1);
    putf8_free(string2);
    putf8_free(string3);
    putf8_free(string4);

    // usize len  = putf8_len(string); // length in characters
    // usize size = putf8_size(string);
}





