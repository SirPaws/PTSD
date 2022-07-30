#include "pstacktrace.h"

void bar(char *c) {
   passert(c != 0);
}

void foo() {
    bar(0);
}

int main(void)
{
    foo();
}

