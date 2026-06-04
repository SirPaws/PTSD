#include "pstr.h"

void *freaky_alloc(void *, usize);

int main(void) {
    const pstr_t *text = pstr_const_create("hello world!", freaky_alloc);
}


