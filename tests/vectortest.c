
#include "vector.h"
#include "pio.h"

typedef pCreateVectorStruct(iVector, int) iVector;


static FormatCallbackTuple VectorFormat(GenericStream *stream, const char *restrict extra_args, va_list list) {
    iVector *vector = va_arg(list, iVector *);
    StreamWrite(stream, '[');
    for (int *it = pVectorBegin((void *)vector); it != pVectorEnd((void *)vector); it++){
        char buf[20];
        u32 count = pItoa(buf, *it);
        StreamWrite(stream, (String){ (u8 *)buf + 1, count });
        if (it == (int *)pVectorEnd((void *)vector) - 1)
            StreamWrite(stream, ']');
        else StreamWrite(stream, pCreateString(", "));
    }
    return (FormatCallbackTuple){ list, extra_args }; 
}


int main(void) {
    iVector *vector = (void *)pInitVector((VectorInfo){ .datasize = sizeof(int), .initialsize = 2}); 
    int *pos = pVectorPushBack((void *)&vector, &(int){32});
    int *second = pVectorInsert((void *)&vector, pos, &(int){55});

    usize count = pVectorSize((void *)vector);
    printf("vector holds %llu elements\n", count);

    for (int *it = pVectorBegin((void *)vector); it != pVectorEnd((void *)vector); it++){
        printf("%p holds %i\n", (void *)it, *it);
    }

    pVectorErase((void *)vector, second);
    count = pVectorSize((void *)vector);
    printf("vector holds %llu element\n", count);

    pFreeVector((void *)vector);

    int *test = malloc(sizeof(int));
    *test = 0xbad;
    printf("%p holds 0x%x", (void *)test, *test);
    free(test);

    return 0;
}



