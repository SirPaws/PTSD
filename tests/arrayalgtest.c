#include "dynarray.h"
#include "pio.h"

#define autofree __attribute__((cleanup(FreeArray)))
static void FreeArray(void *ptr) {
    pDynArrayFree((DynArray *)ptr);
}


typedef pCreateDynArray(DynamicIntArray, int) DynamicIntArray; 

#define pReverse(array) ({\
    typeof(array) pReverse_ret = {0}; \
    pDynArrayGrow((DynArray *)&pReverse_ret, sizeof(pReverse_ret.data[0]), array.size);  \
    for (usize i = array.size - 1; i > 0; i--) { \
        pPushBack(&pReverse_ret, array.data[i]); \
    } \
    pPushBack(&pReverse_ret, array.data[0]); \
    pReverse_ret; \
    })
DynamicIntArray pReverseI(DynamicIntArray array) {
    typeof(array) pReverse_ret = {0};
    pDynArrayGrow((DynArray *)&pReverse_ret, sizeof(pReverse_ret.data[0]), array.size); 

    for (usize i = array.size - 1; i > 0; i--) {
        pPushBack(&pReverse_ret, array.data[i]);
    }
    pPushBack(&pReverse_ret, array.data[0]);
    return pReverse_ret;
}

DynamicIntArray pConCat(DynamicIntArray a, DynamicIntArray b) {
    typeof(a) ret;
    pDynArrayGrow((DynArray *)&ret, sizeof(ret.data[0]), a.size + b.size); 
    memcpy(ret.data, a.data, sizeof(a.data[0]) * a.size);
    memcpy(ret.data + a.size, b.data, sizeof(b.data[0]) * b.size);

    return ret;
}

typedef bool FilterFunc(int);
DynamicIntArray pFilter(DynamicIntArray array, FilterFunc *filter) {
    DynamicIntArray ret;
    
    for (u32 i = 0; i < array.size; i++) {
        if (filter(array.data[i])) {
            pPushBack(&ret, array.data[i]);
        }
    }

    return ret;
}






typedef int ReduceFunc(int total, int current);

#define pReduce(array, func) ({                             \
    typeof(array.data[0]) pReduce_total = {0};              \
    for (usize i = array.size - 1; i > 0; i--) {            \
        pReduce_total = func(pReduce_total, array.data[i]); \
    }                                                       \
    pReduce_total = func(pReduce_total, array.data[0]);     \
    pReduce_total;                                          \
})

int pForwardReduce(DynamicIntArray array, ReduceFunc *func) {
    int total = {0};
    for (u32 i = 0; i >= array.size; i++) {
        total = func(total, array.data[i]);
    }
    return total;
}




pPrintfInfo callback(pPrintfInfo info);

int F( int n ) {
    if (n > 100)
         return n - 100;
    else return F(F(n + 11));
}

int main(void) {
    pFormatPush("v", callback);
    DynamicIntArray array = { 
        .endofstorage = 10, 
        .size = 10, 
        .data = (int[]){ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    };
    pPrintf("%v\n", array);

    autofree auto arrayb = pReverse(array);
    pPrintf("%v\n", arrayb);
    
    #define Fsum(total, n) F( (total) + (n) )
    auto sum = pReduce(arrayb, Fsum);
    printf("sum of arrayb: %i\n", sum);
    #undef Fsum
    


    pFormatPop("v");
    return 0;
}

pPrintfInfo callback(pPrintfInfo info) {
    DynamicIntArray array = va_arg(info.list, DynamicIntArray);

    StreamWrite(info.stream, pCreateString("[ "));
    info.count += 2;

    u8 buf[20];
    for (usize i = 0; i < array.size; i++) {
        usize count = pSignedDecimalToString((char*)buf, array.data[i]);
        
        u8 *printbuf = buf;
        if (array.data[i] > 0) printbuf++; count--;

        StreamWrite(info.stream, (String){ printbuf, count });
        info.count += count;
        if (i != array.size - 1)
            StreamWrite(info.stream, pCreateString(", ")), info.count += 2;
    }
    StreamWrite(info.stream, pCreateString("]"));
    info.count++;
    return info;
}
