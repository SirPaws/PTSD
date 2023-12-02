#include "punique_list.h"

struct thing {
    int v;
};

static inline pul_ordering_t thing_compare(void *set_ptr,  size_t size, void         *a_, void         *_b) {
    (void)set_ptr;
    (void)size;
    struct thing *a = a_, *b = _b;

    printf("using a custom equality (?) function\n");
    return a->v - b->v;
}

int main(void) {
    {
        int *set = NULL;
        pul_insert(set, 1);
        pul_insert(set, 2);
        pul_insert(set, 3);
        pul_insert(set, 3);

        assert(pul_size(set) == 3);
        pul_sort(set);

        printf("int set: ");
        for (usize i = 0; i < pul_size(set); i++) {
            printf("%d ", set[i]);
        }
        putchar('\n');
        pul_free(set);
    }

    {
        struct thing *set = NULL;

        pul_insert(set, (struct thing){3});
        pul_insert(set, (struct thing){2});
        pul_insert(set, (struct thing){1});
        pul_insert(set, (struct thing){3});

        assert(pul_size(set) == 3);
        pul_sort(set);

        printf("default set: ");
        for (usize i = 0; i < pul_size(set); i++) {
            printf("%d ", set[i].v);
        }
        putchar('\n');
        pul_free(set);
    }


    {
        struct thing *set = NULL;
        pul_set_compare_func(set, thing_compare);

        pul_insert(set, (struct thing){3});
        pul_insert(set, (struct thing){2});
        pul_insert(set, (struct thing){1});
        pul_insert(set, (struct thing){3});

        assert(pul_size(set) == 3);
        printf("sorting...\n");
        pul_sort(set);

        printf("default set: ");
        for (usize i = 0; i < pul_size(set); i++) {
            printf("%d ", set[i].v);
        }
        putchar('\n');
        pul_free(set);
    }
}

