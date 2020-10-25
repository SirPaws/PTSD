#include "allocator.h"
#include "dynarray.h"
#include "dynalg.h"
struct TempAllocation {
    usize size;
    void *data;
};

struct AllocatorInfo default_allocator;

pCreateDynArray(TempAllocator, struct TempAllocation *);
void* pTempAllocator(void* block, usize size, usize count, enum AllocatorType type, void* userdata) {
    struct TempAllocator *allocator = userdata;
    switch(type) {
    case MALLOC: MALLOC: {
            if (size == 0) return NULL;

            struct TempAllocation *allocation = malloc(sizeof *allocation + size);
            
            struct AllocatorInfo self = pGlobalAllocator();
            pSetGlobalAllocator(default_allocator);
            pPushBack(allocator, ((void*)allocation));
            pSetGlobalAllocator(self);

            allocation->size = size;
            allocation->data = allocation + 1;
            return allocation->data;
        }
    case ARRAYALLOC: size = size * count;
    case ZALLOC: {
            if (size == 0) return NULL;

            struct TempAllocation *allocation = malloc(sizeof *allocation + size);

            struct AllocatorInfo self = pGlobalAllocator();
            pSetGlobalAllocator(default_allocator);
            pPushBack(allocator, ((void*)allocation));
            pSetGlobalAllocator(self);

            allocation->size = size;
            allocation->data = allocation + 1;
            memset(allocation->data, 0, size);
            return allocation->data;
        }
	case REALLOC: {
            if (size == 0) goto FREE;
            else if (block == 0) goto MALLOC;
            struct TempAllocation *allocation = (void*)(((u8*)block) - sizeof *allocation);
            allocation->size = size;
            realloc(allocation, sizeof *allocation + size);
            return allocation;
        }
    case FREE: FREE: {
            struct TempAllocation *allocation = (void*)(((u8*)block) - sizeof *allocation);
            free(allocation);
            pRemove(allocator, &allocation);
            return 0;
        }
    }
}

void TempAllocatorCleanup(struct TempAllocator *allocator) {
    pForEach(allocator, it) {
        struct TempAllocation *allocation = *it;
        free(allocation);
    }
}

#define TempAllocator __attribute__(( cleanup(TempAllocatorCleanup) )) struct TempAllocator


int main(void) {
    int *i = pAllocateBuffer(sizeof *i);
    assert(i);

    int *z = pZeroAllocateBuffer(sizeof *z);
    assert(!*z);

    int *array = pAllocateArray(100, sizeof(int));
    assert(array);

    pFreeBuffer(array);
    pFreeBuffer(z);
    pFreeBuffer(i);
    
    default_allocator = pGlobalAllocator();
    {
        TempAllocator allocator = { 0 };
        struct AllocatorInfo tmp_alloc = {
            .allocator = pTempAllocator,
            .userdata = &allocator
        };
        pSetGlobalAllocator(tmp_alloc);

        u64 big_array[4096 * 2] = { 0 };

        struct BigStruct {
            u64 arr[4096];
            f64 farr[4096];
        } *big_alloc0 = pAllocateBuffer(sizeof *big_alloc0),
          *big_alloc1 = pZeroAllocateBuffer(sizeof *big_alloc1),
          *big_alloc2 = pAllocateBuffer(sizeof *big_alloc2),
          *big_alloc3 = pZeroAllocateBuffer(sizeof *big_alloc3),
          *big_alloc4 = pAllocateBuffer(sizeof *big_alloc4),
          *big_alloc5 = pZeroAllocateBuffer(sizeof *big_alloc5),
          *big_alloc6 = pAllocateBuffer(sizeof *big_alloc6);

        assert(big_alloc0);
        assert(big_alloc1);assert(memcmp(big_alloc1, big_array, sizeof *big_alloc1) == 0);
        assert(big_alloc2);
        assert(big_alloc3);assert(memcmp(big_alloc3, big_array, sizeof *big_alloc3) == 0);
        assert(big_alloc4);
        assert(big_alloc5);assert(memcmp(big_alloc5, big_array, sizeof *big_alloc5) == 0);
        assert(big_alloc6);
            
    }
    pSetGlobalAllocator(default_allocator);
    
    return 0;
}




