#define PTSD_UTF8_STANDALONE
#include "putf8.h"
#include "putf8_util.h"
#include <threads.h>

int main(void) {
    pstr_t *string0 = pstr("ålborg");
    pstr_t *string1 = pstr(u8"ålborg");
    pstr_t *string2 = pstr(string0);
    pstr_t *string3 = pstr(u8"🏳️‍⚧️");

    pstr_t *lit = pstr_literal(u8"ålborg");
    assert(lit[0] == u8"å"[0]);
    assert(pstr_len(lit) == 6);

    pstr_t *string4 = pstr(u8"\U0001F469\U0001F3FD\U0000200D\U00002764\U0000FE0F\U0000200D\U0001F48B\U0000200D\U0001F468\U0001F3FB");
    pstr_meta_t *meta = ((pstr_meta_t *)string0) - 1;
    printf("cstr length: %zu\n", pstr_len("hi"));
    printf("cstr length: %zu\n", pstr_len(u8"hi"));
    printf(" str length: %zu\n", pstr_len(string0));
    printf(" str length: %zu\n", pstr_len(string0));
    printf("%s: (size: %zu, length: %zu)\n", (char*)string0, meta->size, meta->length);

    printf("%6s: ", (char*)string0);
    auto iter = pstr_iter(string0);
    for (pcodepoint_t chr = pstr_char_next(&iter); chr; chr = pstr_char_next(&iter)) {
        printf("U+%05x ", chr);
    }
    printf("\n");
    printf("%6s : ", (char*)string3);
    iter = pstr_iter(string3);
    for (pcodepoint_t chr = pstr_char_next(&iter); chr; chr = pstr_char_next(&iter)) {
        printf("U+%05x ", chr);
    }

    printf("\n");
    printf("%6s : ", (char*)string4);
    for (pstr_t chr = pstr_char_next(&iter); chr;
         chr = pstr_char_next(&iter)) {
      printf("`u{%05x}", (int)chr);
    }

    pstr_free(string0);
    pstr_free(string1);
    pstr_free(string2);
    pstr_free(string3);
    pstr_free(string4);

    // usize len  = pstr_len(string); // length in characters
    // usize size = pstr_size(string);
}





