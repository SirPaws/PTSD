
#include "vector.h"
#include "pio.h"

typedef pCreateVectorStruct(iVector, int) iVector;


pPrintfInfo VectorFormat(pPrintfInfo info) {
    iVector *vector = va_arg(info.list, iVector *);
    StreamWrite(info.stream, '['); info.count++;
    for (int *it = pVectorBegin((void *)vector); it != pVectorEnd((void *)vector); it++){
        char buf[20];
        u32 count = pSignedDecimalToString(buf, *it);
        if (*it >= 0)
             StreamWrite(info.stream, (String){ (u8 *)buf + 1, count - 1}), info.count += count-1;
        else StreamWrite(info.stream, (String){ (u8 *)buf, count}), info.count += count;

        if (it == (int *)pVectorEnd((void *)vector) - 1)
             StreamWrite(info.stream, ']'), info.count++;
        else StreamWriteString(info.stream, pCreateString(", ")), info.count += 2;
    }
    return info; 
}


int main(void) {
    iVector *vector = (void *)pInitVector((VectorInfo){ .datasize = sizeof(int), .initialsize = 3}); 
    int *pos = pVectorPushBack((void *)&vector, &(int){32});
    int *second = pVectorInsert((void *)&vector, pos, &(int){55});

    usize count = pVectorSize((void *)vector);
    printf("vector holds %llu elements\n", count);
    
    pFormatPush("V", VectorFormat);

    int *third = pVectorInsert((void *)&vector, second, &(int){-63});
    pPrintf("the vector holds %V\n", vector);
    pVectorErase((void *)vector, third);
    pFormatPop("V");

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



