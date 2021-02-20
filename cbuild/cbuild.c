#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cbuild.h"

// DYNARRAY HEADER
#ifndef P_STRECHY_BUFFER_GROWTH_COUNT
#define P_STRECHY_BUFFER_GROWTH_COUNT 2
#endif

#if defined(CBUILD_GNU_COMPATIBLE) // not an msvc compiler

#define pSizeof(value) (sizeof(__typeof(value)))

#define pGetMeta(array) ({                                              \
            StrechyBufferMeta *pGetMeta_meta =                          \
                pGetArrayMetaData((array), pSizeof((array)[0]), false); \
            (array) = (void*)(pGetMeta_meta + 1);                       \
            pGetMeta_meta;                                              \
        })

#define pGetMetaOrCreate(array) ({                                  \
            StrechyBufferMeta *pGetMeta_meta =                      \
            pGetArrayMetaData((array), pSizeof((array)[0]), true);  \
            (array) = (void*)(pGetMeta_meta + 1);                   \
            pGetMeta_meta;                                          \
        })

#define pSize(array)   (pGetMeta(array)->size) 
#define pLength(array) (pGetMeta(array)->size)
#define pLen(array)    (pGetMeta(array)->size)

#define pFreeStrechyBuffer(array) pFreeBuffer(pGetMeta(array))
#define pSetCapacity(array, count) ({                                               \
    if (!(array)) {                                                                 \
        StrechyBufferMeta *pSetCapacity_meta = pZeroAllocateBuffer(                 \
                    (pSizeof((array)[0]) * (count)) + sizeof(StrechyBufferMeta));   \
        pSetCapacity_meta->endofstorage = (pSizeof((array)[0]) * (count));          \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    } else {                                                                        \
        __auto_type pSetCapacity_meta = pGetMeta(array);                            \
        void *pSetCapacity_tmp = pReallocateBuffer(pSetCapacity_meta,               \
                (pSizeof((array)[0]) * (count)) + sizeof(StrechyBufferMeta));       \
        pSetCapacity_meta = pSetCapacity_tmp;                                       \
        (array) = (void*)(pSetCapacity_meta + 1);                                   \
    }                                                                               \
    (array);                                                                        \
})


#define pSetCap     pSetCapacity
#define pSetCount   pSetCapacity
#define pSetSize    pSetCapacity
#define pSetLength  pSetCapacity
#define pReserve    pSetCapacity

#define pPushBack(array, value) ({                                 \
    pGetMetaOrCreate(array);                                       \
    pMaybeGrowStrechyBuffer(&(array), pSizeof((array)[0]));        \
    __auto_type pPushBack_ret = (array) + pSize(array)++;          \
    *pPushBack_ret = (value);                                      \
    pPushBack_ret;                                                 \
})

#define pPushBytes(array, value, bytes) ({              \
    pGetMetaOrCreate(array);                            \
    pMaybeByteGrowStrechyBuffer(&(array), (bytes));     \
    memcpy((array) + pSize(array), (value), (bytes));   \
    __auto_type pPushBack_ret = (array) + pSize(array); \
    pSize(array) += (bytes);                            \
    pPushBack_ret;                                      \
})

#define pBegin(array) ({ (array); })
#define pEnd(array) ({ (array) + pSize(array); })

#define pInsert(array, position, value) ({                                                      \
    __auto_type pInsert_array = pGetMetaOrCreate(array);                                        \
    usize pInsert_size = pSizeof( value );                                                      \
    usize pInsert_offset = (position) - pBegin(array);                                          \
    __typeof(value) *pInsert_result = NULL;                                                     \
    if (pInsert_array->size && pInsert_offset >= pInsert_array->size) {}                        \
    else {                                                                                      \
        pMaybeGrowStrechyBuffer(&(array), pInsert_size);                                        \
        /* first we extract all elements after the place where we want                        */\
        /* to insert and then we shift them one element forward                               */\
        /* here is an example we wan't to insert 6 at the place pointed to below              */\
        /* [1, 2, 3, 4]                                                                       */\
        /*     ^                                                                              */\
        /* we make a new array that holds [2, 3, 4]                                           */\
        /* we insert that into the array                                                      */\
        /* [1, 2, 2, 3, 4]                                                                    */\
        /* then we insert the value                                                           */\
        /* [1, 6, 2, 3, 4]                                                                    */\
        usize pInsert_elems = pInsert_array->size - pInsert_offset;                             \
        if (pInsert_elems) {                                                                    \
            memmove((array) + pInsert_offset + 1,                                               \
                    (array) + pInsert_offset, pInsert_elems * pInsert_size);                    \
        }                                                                                       \
                                                                                                \
        pInsert_array->size++;                                                                  \
        (array)[pInsert_offset] = value;                                                        \
        pInsert_result = (array) + pInsert_offset;                                              \
    }                                                                                           \
    pInsert_result;                                                                             \
})


#define pPopBack(array) ({                                      \
            __typeof((array)[0]) pPopBack_result = {0};         \
            if (pSize(array) == 0) {}                           \
            else {                                              \
                pPopBack_result = (array)[(pSize(array)--) - 1];\
            }                                                   \
            pPopBack_result;                                    \
        })

#define pRemove(array, position) ({                                                 \
    __typeof((array)[0]) pRemove_result = (__typeof((array)[0])){0};                \
    usize pRemove_offset = (position) - pBegin(array);                              \
    if (pRemove_offset >= pSize(array)) {}                                          \
    else if (pRemove_offset == pSize(array) - 1) {                                  \
        pSize(array)--;                                                             \
        pRemove_result = (array)[pRemove_offset];                                   \
    } else {                                                                        \
        pRemove_result = (array)[pRemove_offset];                                   \
        usize pRemove_elems = pSize(array) - pRemove_offset;                        \
        memmove((array) + pRemove_offset, (array) + pRemove_offset + 1,             \
                pRemove_elems * pSizeof((array)[0]));                               \
        pSize(array)--;                                                             \
    }                                                                               \
    pRemove_result;                                                                 \
})

// arr:  another dynamic array
#define pCopyStrechyBuffer(arr) ({\
    __auto_type pCopyStrechyBuffer_array  = pGetMeta(array);                            \
    usize pCopyStrechyBuffer_size = pSizeof((arr)[0]) * pCopyStrechyBuffer_array->size; \
    StrechyBufferMeta *pCopyStrechyBuffer_copy =                                        \
        pAllocateBuffer(sizeof(StrechyBufferMeta) * pCopyStrechyBuffer_size)            \
    pCopyStrechyBuffer_copy->endofstorage = pCopyStrechyBuffer_size;                    \
    pCopyStrechyBuffer_copy->size = pCopyStrechyBuffer_array->size;                     \
    memcpy((arr), pCopyStrechyBuffer_array + 1, pCopyStrechyBuffer_size;                \
    (__typeof((arr)[0])*)( pCopyStrechyBuffer_copy + 1);                                \
})

// type: the type of array we want
// arr:  a static array
#define pCopyArray(type, arr) ({                    \
    type pCopyArray_tmp = NULL;                     \
    pReserve(pCopyArray_tmp, countof(arr));         \
    memcpy(pCopyArray_tmp, (arr), sizeof(arr));     \
    pGetMeta(pCopyArray_tmp)->size = countof(arr);  \
    pCopyArray_tmp;                                 \
})



typedef struct StrechyBufferMeta StrechyBufferMeta;
struct StrechyBufferMeta {
    usize size;
    usize endofstorage;
};

// how many elements we should add
static void pStrechyBufferByteGrow(void* array, usize bytes);
static void pStrechyBufferGrow(void* array, usize datasize, usize count);

CBUILD_UNUSED
static void pMaybeByteGrowStrechyBuffer(void* array, usize bytes) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if (meta->size + bytes > meta->endofstorage) {
        pStrechyBufferByteGrow(array, bytes);
    }
}

CBUILD_UNUSED
static void pMaybeGrowStrechyBuffer(void* array, usize datasize) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStrechyBufferGrow(array, datasize, P_STRECHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStrechyBufferByteGrow(void* array_ptr, usize bytes) {
    if (!bytes || !array_ptr) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;


    void* tmp = pReallocateBuffer(meta, sizeof(StrechyBufferMeta) + meta->endofstorage + bytes);
    assert(tmp); meta = tmp;
    meta->endofstorage += bytes;
    *(u8**)array_ptr = (void*)(meta + 1);
}

static void pStrechyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StrechyBufferMeta) + array_size;
    void* tmp = pReallocateBuffer(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1);
}
StrechyBufferMeta* pGetArrayMetaData(void* array, usize data_size, pBool create) {
    if (!array) {
        if (!create) { // should probably just return NULL
            static StrechyBufferMeta nil = { 0 };
            nil = (StrechyBufferMeta){ 0 };
            return &nil;
        }
        StrechyBufferMeta* meta = pZeroAllocateBuffer(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

#if !defined(NDEBUG)
StrechyBufferMeta* _debug_GetMeta(void* array) { //NOLINT
    StrechyBufferMeta* address = array;
    return address - 1;
}
#endif
#else // using mvsc compiler
#define pSizeof(value) (sizeof(value))

#define pGetMetaOrCreate(array)                                                         \
    ((array) = (void*) pGetArrayMetaData((array), pSizeof((array)[0]),true),            \
            (array) = ((StrechyBufferMeta*)array) + 1, ((StrechyBufferMeta*)array) - 1)

#define pGetMeta(array) (pGetArrayMetaData((array), pSizeof((array)[0]), false))


#define pSize(array)   (pGetMeta(array)->size) 
#define pLength(array) (pGetMeta(array)->size)
#define pLen(array)    (pGetMeta(array)->size)

#define pFreeStrechyBuffer(array) pFreeBuffer(pGetMeta(array))

#define pSetCapacity(array, count)   \
    (pSetArraySize(pGetMetaOrCreate(array), &(array), (pSizeof((array)[0]) * (count))), array) 


#define pSetCap     pSetCapacity
#define pSetCount   pSetCapacity
#define pSetSize    pSetCapacity
#define pSetLength  pSetCapacity
#define pReserve    pSetCapacity

#define pPushBack(array, value) \
    (pGetMetaOrCreate(array), pMaybeGrowStrechyBuffer(&(array), pSizeof((array)[0])),\
        *((array)+pSize(array)) = (value), pGetMeta(array)->size++, ((array)+(pSize(array)-1)))

#define pBegin(array) (array)
#define pEnd(array) ((array) + pSize(array))

#define pInsert(array, position, value)\
    (pGetMetaOrCreate(array), (((position) - pBegin(array)) >= pSize(array) ? NULL :              \
        ((position) = pInsertAtLocation(pGetMeta(array), &(array), &(position), pSizeof(value)))),\
        (*(position) = (value)), position)



#define pPopBack(array) /*due to not having any typeof we just return msvc (trash) on size 0*/\
    (pSize(array) == 0 ? (array)[-9034] : (pGetMeta(array)->size--, (array)[pSize(array)]))   \

#define pRemove(array, position)                                            \
    (((position) - pBegin(array)) >= pSize(array) ? *(position) :             \
    ((((position) - pBegin(array)) == pSize(array) - 1) ? pPopBack(array) : \
                (array)[pSwapAndPop(pGetMeta(array), &(array), &(position), pSizeof((array)[0]))]))

#define pCopyStrechyBuffer(array)    \
    (pCopyBuffer(pGetMeta(array), pSizeof((array)[0]))) 

#define pCopyArray(array) \
    (pCopyStaticArray((array), pSizeof((array)[0]), sizeof((array)))


typedef struct StrechyBufferMeta StrechyBufferMeta;
struct StrechyBufferMeta {
    usize size;
    usize endofstorage;
};

// how many elements we should add
static void pStrechyBufferGrow(void* array, usize datasize, usize count);

static void pMaybeGrowStrechyBuffer(void* array, usize datasize) {
    StrechyBufferMeta** meta_ptr = array;
    StrechyBufferMeta* meta = (*meta_ptr) - 1;

    if ((meta->size + 1) * datasize > meta->endofstorage) {
        pStrechyBufferGrow(array, datasize, P_STRECHY_BUFFER_GROWTH_COUNT);
    }
}

static void pStrechyBufferGrow(void* array_ptr, usize datasize, usize count) {
    if (!count || !array_ptr || !datasize) return;
    u8* array = *(u8**)array_ptr;
    StrechyBufferMeta* meta = ((StrechyBufferMeta*)array) - 1;

    usize array_size = meta->endofstorage + (datasize * count);
    usize size = sizeof(StrechyBufferMeta) + array_size;
    void* tmp = pReallocateBuffer(meta, size);
    assert(tmp); meta = tmp;
    meta->endofstorage += datasize * count;
    *(u8**)array_ptr = (void*)(meta + 1);
}

static StrechyBufferMeta p_strechy_buffer_nil = { 0 };
static StrechyBufferMeta* pGetArrayMetaData(void* array, usize data_size, pBool should_create) {
    if (!array) {
        if (!should_create) {
            p_strechy_buffer_nil = (StrechyBufferMeta){ 0 };
            return &p_strechy_buffer_nil;
        }
        StrechyBufferMeta* meta = pZeroAllocateBuffer(sizeof(StrechyBufferMeta) + data_size);
        meta->endofstorage = data_size;
        meta->size = 0;
        array = (meta + 1);
    }

    return ((StrechyBufferMeta*)array) - 1;
}

static StrechyBufferMeta* pSetArraySize(StrechyBufferMeta* meta, void* array_ptr, usize new_size) {
    void* tmp = pReallocateBuffer(meta, (new_size)+sizeof(StrechyBufferMeta));
    assert(tmp); meta = tmp;
    meta->endofstorage = new_size;
    *((void**)array_ptr) = meta + 1;
    return meta;
}

static void* pCopyBuffer(StrechyBufferMeta* src, usize data_size) {
    if (src->size != 0) {
        StrechyBufferMeta* meta = 
            pZeroAllocateBuffer((data_size * src->size)+ sizeof(StrechyBufferMeta));
        meta->endofstorage = src->size * data_size;
        meta->size = src->size;
        memcpy(meta + 1, src + 1, src->size * data_size);
        return meta + 1;
    }
    else {
        void* arr;
        StrechyBufferMeta* meta = pNewArray(&arr, data_size);
        return arr;
    }
}

static void* pCopyStaticArray(void* array, usize data_size, usize count) {
    if (count != 0) {
        StrechyBufferMeta* meta = 
            pZeroAllocateBuffer((data_size * count) + sizeof(StrechyBufferMeta));
        meta->endofstorage = count * data_size;
        meta->size = count;
        memcpy(meta + 1, array, count * data_size);
        return meta + 1;
    }
    else {
        void* arr;
        StrechyBufferMeta* meta = pNewArray(&arr, data_size);
        return arr;
    }
}

static void* pInsertAtLocation(StrechyBufferMeta* meta, void* array, void* location, usize data_size) {
    pMaybeGrowStrechyBuffer(array, data_size);

    /* first we extract all elements after the place where we want
     * to insert and then we shift them one element forward
     * here is an example we wan't to insert 6 at the place pointed to below
     * [1, 2, 3, 4]
     *     ^
     * we make a new array that holds [2, 3, 4]
     * we insert that into the array
     * [1, 2, 2, 3, 4]
     * then we insert the value
     * [1, 6, 2, 3, 4]
     */

    u8* array_ = *(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(array_ + ((offset + 1) * data_size), (array_)+(offset * data_size), elems * data_size);
    meta->size++;
    return array_ + (offset * data_size);
}

static usize pSwapAndPop(StrechyBufferMeta* meta, void* array, void* location, usize data_size) {
    u8* buffer = malloc(data_size);
    u8* data_start = *(void**)location;
    memmove(buffer, data_start, data_size);

    u8* array_ = *(void**)array;
    usize offset = (((u8*)(*(void**)location)) - array_) / data_size;
    usize elems = meta->size - offset;
    memmove(data_start, data_start + data_size, elems * data_size);
    memmove(array_ + (--meta->size) * data_size, buffer, data_size);
    free(buffer);
    return meta->size;
}
#endif

// CBUILD_MACRO_UTIL_HEADER
#define pHas2Args_(_0, a, b, _3, answer, _5, ...) answer
#define pHas2Args(a, ...) pHas2Args_(0, a, ## __VA_ARGS__, 1, 1, 0, 1) 

#define CBUILD_CONCAT_( a, b ) a##b
#define CBUILD_CONCAT( a, b ) CBUILD_CONCAT_( a, b )

#define CBUILD_STRINGIFY_(x) #x
#define CBUILD_STRINGIFY(x) CBUILD_STRINGIFY_(x)

// DYNALG HEADER
#define pForEach(array, ...)     pForEach_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)

#define pForEach1(array, name)  for( __auto_type name = pBegin(array); name != pEnd(array); name++) // NOLINT
#define pForEach0(array)        pForEach1(array, it)
#define pForEach__(array, args) CBUILD_CONCAT(pForEach, args)
#define pForEach_(array, ...)    pForEach__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEachR1(array, name)  for( __auto_type name = pEnd(array) - 1; name != pBegin(array) - 1; name++) // NOLINT
#define pForEachR0(array)        pForEachR1(array, it)
#define pForEachR__(array, args) CBUILD_CONCAT(pForEachR, args)
#define pForEachR_(array, ...)   pForEachR__(array, pHas2Args( array, ## __VA_ARGS__ ))

#define pForEach(array, ...)     pForEach_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)
#define pForEachI(array, ...)    pForEachR_(array, ## __VA_ARGS__)(array, ## __VA_ARGS__)

// PPLATFORM HEADER
typedef struct pHandle pHandle;

pHandle *pNullHandle(void);

pHandle *pGetSTDOutHandle(void);
pHandle *pGetSTDInHandle(void);

typedef struct pFileStat pFileStat;
struct pFileStat {
    pBool exists;
    u64 filesize;
    u64 creationtime;
    u64 accesstime;
    u64 writetime;
};

pFileStat pGetFileStat(const char *file);

typedef u8 pFileAccess;
enum pFileAccess {
    P_WRITE_ACCESS = 0b01,
    P_READ_ACCESS  = 0b10,
};

pHandle *pFileOpen(const char *file,   pFileAccess access);
pHandle *pFileCreate(const char *file, pFileAccess access);

void pFileClose(pHandle *handle);

pBool pFileWrite(pHandle *handle, String buf);
pBool pFileRead(pHandle *handle, String buf);

enum pSeekMode {
    P_SEEK_SET,
    P_SEEK_CURRENT,
    P_SEEK_END,
};

pBool pSeek(pHandle *handle, isize size, enum pSeekMode mode);

// if the system supports colored output we enable it
// if it doesn't this function will do nothing
pBool pEnableConsoleColorOutput(void);
// PPLATFORM SOURCE
#if defined(CBUILD_WINDOWS)
#include <Windows.h>
#elif defined(CBUILD_LINUX)
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(__APPLE__)
#error support for mac has to been implemented
#endif

pHandle *pNullHandle(void) {
#if defined(CBUILD_WINDOWS)
    return NULL; 
#else
    return (void*)((u32) -1);
#endif
}

pBool pEnableConsoleColorOutput(void) {
#if defined(CBUILD_WINDOWS)
    BOOL result = SetConsoleMode(GetModuleHandle(NULL), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return result != 0;
#else
#endif
}

pHandle *pGetSTDOutHandle(void) {
#if defined(CBUILD_WINDOWS)
    return GetStdHandle(STD_OUTPUT_HANDLE); 
#else
    return (void*)((u32) 1);
#endif
}

pHandle *pGetSTDInHandle(void) {
#if defined(CBUILD_WINDOWS)
    return GetStdHandle(STD_INPUT_HANDLE); 
#else
    return (void*)((u32) 0);
#endif
}

pFileStat pGetFileStat(const char *file) {
    pFileStat result;
#if defined(CBUILD_WINDOWS)
#define WIN32_COMBINE_HIGH_LOW( high, low ) ((u64)(high) << 31 | (low)) 
    WIN32_FILE_ATTRIBUTE_DATA data;
    result.exists   = GetFileAttributesEx(file, GetFileExInfoStandard, &data);

    result.filesize = WIN32_COMBINE_HIGH_LOW(data.nFileSizeHigh, data.nFileSizeLow);

    result.creationtime = WIN32_COMBINE_HIGH_LOW(data.ftCreationTime.dwHighDateTime, 
            data.ftCreationTime.dwLowDateTime);
    result.accesstime   = WIN32_COMBINE_HIGH_LOW(data.ftLastAccessTime.dwHighDateTime,
            data.ftLastAccessTime.dwLowDateTime);
    result.writetime    = WIN32_COMBINE_HIGH_LOW(data.ftLastWriteTime.dwHighDateTime, 
            data.ftLastWriteTime.dwLowDateTime);
#undef WIN32_COMBINE_HIGH_LOW
#else
    struct stat data;
    s32 statret = stat(file, &data);
    result.exists = statret == -1 ? false : true; 
    result.filesize = data.st_size;
    result.creationtime = data.st_ctim.tv_nsec; 
    result.accesstime   = data.st_atim.tv_nsec;
    result.writetime    = data.st_mtim.tv_nsec;
#endif
    return result;
}

pHandle *pFileOpen(const char *filename, pFileAccess access) {
#if defined(CBUILD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    return (void*)(u64)open(filename, file_access);
#endif
}
pHandle *pFileCreate(const char *filename, pFileAccess access) {
#if defined(CBUILD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    file_access|= O_CREAT;
    return (void*)(u64)open(filename, file_access);
#endif
}

void pFileClose(pHandle *handle) {
#if defined(CBUILD_WINDOWS)
    CloseHandle(handle);
#else
    u32 fh = (u32)((void *)handle);
    close(fh);
#endif
}

pBool pFileWrite(pHandle *handle, String buf) {
#if defined(CBUILD_WINDOWS)
    return WriteFile(handle, buf.c_str, (u32)buf.length, NULL, NULL);
#else
    s32 result = write((u64)(void*)handle, buf.c_str, (u32)buf.length);
    return result == -1 ? false : true;
#endif
}

pBool pFileRead(pHandle *handle, String buf) {
#if defined(CBUILD_WINDOWS)
    DWORD bytes_read = 0;
    ReadFile(handle, buf.c_str, buf.length, &bytes_read, NULL);
    return bytes_read != 0;
#else
    s32 result = read((u64)(void*)handle, buf.c_str, buf.length);
    return result == -1 ? false : true;
#endif
}

pBool pSeek(pHandle *handle, isize size, enum pSeekMode mode) {
#if defined(CBUILD_WINDOWS)
    
    DWORD wmode;
    switch (mode) {
    case P_SEEK_SET: wmode = FILE_BEGIN; break;
    case P_SEEK_END: wmode = FILE_END;   break;
    case P_SEEK_CURRENT:
    default: wmode = FILE_CURRENT;
    }

    DWORD result = SetFilePointer(handle, size, 0, wmode);
    return result != INVALID_SET_FILE_POINTER;
#else
    u32 wmode;
    switch (mode) {
    case P_SEEK_SET: wmode = SEEK_SET; break;
    case P_SEEK_END: wmode = SEEK_END; break;
    case P_SEEK_CURRENT:
    default: wmode = SEEK_CUR;
    }

    off_t offset = lseek((u64)(void*)handle, size, mode);
    return offset != -1;
#endif
}


#if defined(CBUILD_WINDOWS)
#include <direct.h>
#endif

void maybeConvertBackslash(String filepath) {
    for (usize i = 0; i < filepath.length; i++) {
        if (filepath.c_str[i] == '\\')
            filepath.c_str[i] = '/';
    }
}
usize findCommonPath(usize len, char str[len], String fullpath) {
    
    u8 *last  = fullpath.c_str + (fullpath.length - 1); 
    u8 *first = last; 

    u8 *file = (u8*)str + (len - 1);
    while (*first == *file) {
        first--; file--;
    };

    return last - first;
}

void buildReset(BuildContext *ctx) {
    buildFree(ctx);
    buildSetDefaults(ctx);
}
void buildFree(BuildContext *ctx) {
#define maybefree(array) if ((array)) pFreeStrechyBuffer((array)) 
    maybefree(ctx->flags);
    maybefree(ctx->includes);
    maybefree(ctx->lib_dirs);
    maybefree(ctx->libs);
    maybefree(ctx->files);
#undef maybefree
    memset(ctx, 0, sizeof *ctx);
}

void buildSetDefaults(BuildContext *ctx) {
#define initArray(array, count) \
    (array) = NULL; pReserve(array, count)\

    initArray(ctx->flags, 10);
    initArray(ctx->includes, 10);
    initArray(ctx->lib_dirs, 10);
    initArray(ctx->libs, 10);
    initArray(ctx->files, 50);
#undef initArray

    ctx->int_dir = ".\\bin\\int";
    ctx->out_dir = ".\\bin";
    ctx->working_dir = ".";
    ctx->out_name = "out";
}

void setBuildType(BuildContext *ctx, enum BuildType type) {
    ctx->type = type;
} 

void setBuildMode(BuildContext *ctx, enum BuildMode mode) {
    ctx->mode = mode;
}
void setWorkingDir(BuildContext *ctx, char *path) {
    ctx->working_dir = path;  
}
void setIntemidiaryDir(BuildContext *ctx, char *path) {
    ctx->int_dir = path; 
}
void setOutputDir(BuildContext *ctx, char *path) {
    ctx->out_dir = path; 
}
void setOutputName(BuildContext *ctx, char *name) {
    ctx->out_name = name; 
}

void constructCompileCommands(BuildContext *ctx) {
#define pPushStr(array, str) pPushBytes((array), (str), sizeof(str) - 1)
    u8 *command = NULL;
    pPushStr(command, "clang ");

    pForEach(ctx->flags, flag) {
        usize len = strlen(*flag);
        pPushBytes(command, *flag, len); 
        pPushStr(command, " ");
    }

    pForEach(ctx->includes, include) {
        usize len = strlen(*include);
        pPushStr(command, "-I"); 
        pPushBytes(command, *include, len); 
        pPushStr(command, " ");
    }
    pPushStr(command, "-I. ");



    switch(ctx->mode) {
    case MODE_DEBUG:   pPushStr(command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: pPushStr(command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
                  pPushStr(command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    pHandle *compile_commands = pFileCreate((char*)"compile_commands.json", 
            P_READ_ACCESS|P_WRITE_ACCESS);
    char buf[4096]; // NOLINT
    char *filepart = NULL; // NOLINT

#define createString(str) ((String){ .length = sizeof(str) - 1, (u8*)(str) }) 

    pFileWrite(compile_commands, createString("[\n"));

    char **last_file = ctx->files + pSize(ctx->files) - 1;
    pForEach(ctx->files) {
        GetFullPathName(*it, 4096, buf, NULL); // NOLINT
        String fullpath = {
            .length = strlen(buf),
        };
        fullpath.c_str = malloc(fullpath.length + 1);
        memcpy(fullpath.c_str, buf, fullpath.length);




        maybeConvertBackslash(fullpath);
        usize len = strlen(*it);
        len = findCommonPath(len, *it, fullpath);


        fullpath.length -= len + 1;

        pFileWrite(compile_commands, createString("\t{\n"));

        pFileWrite(compile_commands, createString("\t\t\"directory\": \""));
        pFileWrite(compile_commands, fullpath);
        pFileWrite(compile_commands, createString("\",\n"));

        pFileWrite(compile_commands, createString("\t\t\"command\": \""));
        pFileWrite(compile_commands, (String){ pSize(command), command});
        pFileWrite(compile_commands, (String){ strlen(*it), (u8*)*it});

        pFileWrite(compile_commands, createString("\",\n"));

        fullpath.length += len + 1;
        pFileWrite(compile_commands, createString("\t\t\"file\": \""));
        pFileWrite(compile_commands, fullpath);
        pFileWrite(compile_commands, createString("\"\n"));

        pFileWrite(compile_commands, createString("\t}"));
        if (it != last_file)
            pFileWrite(compile_commands, createString(",\n"));
        else pFileWrite(compile_commands, createString("\n"));

        free(fullpath.c_str);
    }
    pFileWrite(compile_commands, createString("]"));
    pFileClose(compile_commands);
}


void addElementsToStringArray(StrechyCharBuffer *buf, usize n, char *str[n]) { 
    if (__builtin_expect(pSize(*buf) == 0, 1)) {
        if (n > pGetMeta(*buf)->endofstorage/sizeof(char*)) {
            void *tmp = realloc(pGetMeta(*buf), n * sizeof(char*));
            assert(tmp); *buf = (void*)((u8*)tmp + sizeof(StrechyBufferMeta));
            pGetMeta(*buf)->endofstorage = n * sizeof(char*);
        }
        
        memcpy(*buf, str, n * sizeof(char*));
        pSize(*buf) = n;
        return;
    } else {
        if (n > pGetMeta(*buf)->endofstorage/sizeof(char*)) {
            if (n > pGetMeta(*buf)->endofstorage/sizeof(char*)) {
                void *tmp = realloc(pGetMeta(*buf), n * sizeof(char*));
                assert(tmp); *buf = (void*)((u8*)tmp + sizeof(StrechyBufferMeta));
                pGetMeta(*buf)->endofstorage = n * sizeof(char*);
            }
        }
        for (usize i = 0; i < n; i++) {
            pPushBack(*buf, str[i]);
        }
    }
}



void setBuildFlag(BuildContext *ctx, char *flag) {
    pPushBack(ctx->flags, flag);
}
void setBuildFlags(BuildContext *ctx, usize count, char *flags[]) {
    addElementsToStringArray(&ctx->flags, count, flags);
}

void addBuildFile(BuildContext *ctx, char *filepath) {
    pPushBack(ctx->files, filepath);
}
void addBuildFiles(BuildContext *ctx, usize count, char *filepaths[count]) {
    addElementsToStringArray(&ctx->files, count, filepaths);
}

void addIncludeDir(BuildContext *ctx, char *filepath){
    pPushBack(ctx->includes, filepath);
}
void addIncludeDirs(BuildContext *ctx, usize count, char *filepaths[count]) {
    addElementsToStringArray(&ctx->includes, count, filepaths);
}

void addLibraryDir(BuildContext *ctx, char *filepath) {
    pPushBack(ctx->lib_dirs, filepath);
}
void addLibraryDirs(BuildContext *ctx, usize count, char *filepaths[count]) {
    addElementsToStringArray(&ctx->lib_dirs, count, filepaths);
}

void addLibrary(BuildContext *ctx, char *name) {
    pPushBack(ctx->libs, name);
}
void addLibraries(BuildContext *ctx,usize count, char *filepaths[count]) {
    addElementsToStringArray(&ctx->libs, count, filepaths);
}

pBool anyOf(char check, usize count, const u8 character[static count]) {
    
    for (usize i = 0; i < count; i++) {
        if ((u8)check == character[i]) return true;
    }
    return false;
}

void handleCurrentDir(u8 **pos) {
    u8 *begin = *pos;
    if (*begin == '.' && anyOf(*(begin + 1), 2, (u8[]){ '/', '\\' }))
        begin += 2;
    *pos = begin;
}

void handlePreviousDir(u8 **pos) {
    u8 *begin = *pos;

    if (*begin == '.' && *(begin+1) == '.')
        begin += 2;
    if (anyOf(*begin, 2, (u8[]){ '/', '\\' }))
        begin++;
 
    *pos = begin;
}



pBool makeDirectoryRecursive(String path) {
    u8 *buf = malloc(path.length + 1);
    memcpy(buf, path.c_str, path.length);
    buf[path.length] = '\0';

    u8 *path_begin = path.c_str;
    u8 *begin = (u8*)path.c_str;
    while (*begin != '\0') {
        if (*begin == '.') {
            handleCurrentDir(&begin);
            handlePreviousDir(&begin);
        }
        u8 *end = begin;
        while (!(*end == '\\' || *end == '/')) {
            if (*end == '\0') break;
            end++;
        }

        char tmp = buf[end - path_begin];
        buf[end - path_begin] = '\0';
        _mkdir((char*)buf);
        if (errno == ENOENT) {
            return false;
        }
        buf[end - path_begin] = tmp;
        begin = end + 1;
    }
    return true;
}

u8 *getFileExtensionAndName(u8 *path, usize length, u8 **extension) {
    u8 *extension_marker = path + (length-1);
    while (*extension_marker != '.') {
        extension_marker--;
        if (extension_marker == path) break;
    }
    u8 *file = (u8*)path;

    while (!anyOf(*file, 2, (u8[]){'/', '\\'})) {
        if (file == path) break;
        file--;
    }

    *extension = extension_marker;
    if (file == path)
         return path;
    else return file + 1;
}

void execute(BuildContext *ctx) { // NOLINT

    String intermediate = {
        .length = strlen(ctx->int_dir),
        .c_str  = (u8*)ctx->int_dir
    };

    String out = {
        .length = strlen(ctx->out_dir),
        .c_str  = (u8*)ctx->out_dir
    };

    SetCurrentDirectory(ctx->working_dir);
    if (!makeDirectoryRecursive(intermediate)) {
        puts("could not create intermediate directory");
        exit(-1);
    }
    if (!makeDirectoryRecursive(out)) {
        puts("could not create intermediate directory");
        exit(-1);
    }


#define pPushStr(array, str) pPushBytes((array), (str), sizeof(str) - 1)
    u8 *command = NULL;
    pPushStr(command, "clang ");

    pForEach(ctx->flags, flag) {
        usize len = strlen(*flag);
        pPushBytes(command, *flag, len); 
        pPushStr(command, " ");
    }

    pForEach(ctx->includes, include) {
        usize len = strlen(*include);
        pPushStr(command, "-I"); 
        pPushBytes(command, *include, len); 
        pPushStr(command, " ");
    }
    pPushStr(command, "-I. ");

#if defined(CBUILD_WINDOWS)
        // these shouldn't be constants
        char archiver[] = "llvm-lib /OUT:";
        char extension[] = ".lib";
        
        char dll_archiver[] = "llvm-lib /DLL /OUT:";
        char dll_extension[] = ".dll";
#else
        char archiver[] = "ar rcs lib";
        char extension[] = ".a";

        char dll_archiver[] = "clang -shared -o";
        char dll_extension[] = ".so";
#endif


    switch(ctx->mode) {
    case MODE_DEBUG:   pPushStr(command, "-g -O0 -Wall -Wextra -DDEBUG ");  break;
    case MODE_RELEASE: pPushStr(command, "-O2 -Ofast -DRELEASE -DNDEBUG "); break;
    case MODE_RELEASE_WITH_DEBUG: 
        pPushStr(command, "-g -O2 -Ofast -DRELEASE -DDEBUG -DREL_WITH_DEBUG "); break;
    default: break;
    }

    if (ctx->type & UNITY_BUILD) {
        usize dirlen = strlen(ctx->int_dir);
        if (anyOf(ctx->int_dir[dirlen - 1], 2, (u8[]){ '/', '\\' }))
            dirlen--;

        usize filepath_len = dirlen + sizeof("/unity_build.c") - 1;
        char *filepath = malloc(filepath_len + 1);


        memcpy(filepath, ctx->int_dir, dirlen); 
        memcpy(filepath + dirlen, "/unity_build.c", sizeof("/unity_build.c") - 1);
        filepath[filepath_len] = '\0';

        // remove the UNITY_BUILD flag
        pHandle *build_file = pFileCreate((char*)filepath, P_READ_ACCESS|P_WRITE_ACCESS);
        pForEach(ctx->files) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension_marker = file + (length-1);
            while (*extension_marker != '.') {
                extension_marker--;
                if (extension_marker == file) break;
            }
            if (memcmp(extension_marker, ".c", 2) != 0) continue;

            String prepend = {
                .length  = sizeof("#include \"") - 1,
                .c_str = (u8*)"#include \""
            };
            String append = {
                .length  = sizeof("\" // NOLINT\n") - 1,
                .c_str = (u8*)"\" // NOLINT\n"
            };

            pFileWrite(build_file, prepend);
            pFileWrite(build_file, (String){ length, file });
            pFileWrite(build_file, append);
        }
        pFileClose(build_file);

        pForEach(ctx->lib_dirs, dirs) {
            pPushStr(command, "-L");
            pPushBytes(command, *dirs, strlen(*dirs));
            pPushStr(command, " ");
        }

        u8 *iterator = pPushStr(command, "-c ");
        usize idx = iterator - command;

        pPushBytes(command, filepath, filepath_len);
        pPushStr(command, " -o ");
        filepath[filepath_len - 1] = 'o';
        pPushBytes(command, filepath, filepath_len);
        pPushStr(command, " ");
        pPushBytes(command, &(char){'\0'}, 1);
       
        printf("running command:\n%s\n", command);
        // compile to object file
        system((char*)command); // NOLINT

        u8 *next_command = NULL;
        pReserve(next_command, pGetMeta(command)->endofstorage);
        memcpy(next_command, command, idx);
        pSize(next_command) += idx;

        ctx->type &= 0b0111; // NOLINT
        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                pPushBytes(next_command, filepath, filepath_len);
                pPushStr(next_command, " -o ");
                
                pPushBytes(next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");
                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));

#if defined(CBUILD_WINDOWS)
                pPushStr(next_command, ".exe");
#endif
                pPushStr(next_command, " ");
                pForEach(ctx->libs, lib) {
                    pPushStr(next_command, "-l");
                    pPushBytes(next_command, *lib, strlen(*lib));
                    pPushStr(next_command, " ");
                }
                pPushBytes(next_command, &(char){'\0'}, 1);
                
                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        case STATIC_LIB: {
                pSize(next_command) = 0;
                pPushBytes(next_command, archiver, sizeof(archiver) - 1);

                pPushBytes(next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");

                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));
                pPushBytes(next_command, extension, sizeof(extension) - 1); 
            
                pPushStr(next_command, " ");

                pPushBytes(next_command, filepath, filepath_len);
                pPushBytes(next_command, &(char){'\0'}, 1);

                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        case DYNAMIC_LIB: {
                pSize(next_command) = 0;
                pPushBytes(next_command, dll_archiver,  sizeof(dll_archiver) - 1);
                
                pPushBytes(next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");

                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));

                pPushBytes(next_command, dll_extension, sizeof(dll_extension) - 1); 

                pPushStr(next_command, " ");

                pPushBytes(next_command, filepath, filepath_len);
                pPushBytes(next_command, &(char){'\0'}, 1);

                printf("running command:\n%s\n", next_command);
                system((char*)next_command); // NOLINT
            } break;
        default: break;
        }

        
        pFreeStrechyBuffer(next_command);
        pFreeStrechyBuffer(command);
    } else {
        u8 *libs = NULL;
        pForEach(ctx->libs, lib) {
            pPushStr(libs, " -l");
            pPushBytes(libs, *lib, strlen(*lib));
        }

        usize command_len = pSize(command);
        pForEach(ctx->files) {
            usize length = strlen(*it);
            u8 *file = (u8*)*it;
            u8 *extension = file + (length-1);
            
            u8 *filename = getFileExtensionAndName(file, length, &extension);
            if (memcmp(extension, ".c", 2) != 0) continue;
            usize filename_len = length - (filename - file) - 2;

            pPushStr(command, "-c ");
            pPushBytes(command, file, length);
            pPushStr(command, " -o ");

            pPushBytes(command, intermediate.c_str, intermediate.length);
            if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                pPushStr(command, "/");

            pPushBytes(command, filename, filename_len);
            pPushStr(command, ".o");
            pPushBytes(command, &(u8){'\0'}, 1);
            
            system((char*)command); // NOLINT
            pSize(command) = command_len;
        }
        
        pForEach(ctx->lib_dirs, dirs) {
            pPushStr(command, "-L");
            pPushBytes(command, *dirs, strlen(*dirs));
            pPushStr(command, " ");
        }

        u8 *next_command = NULL;
        pReserve(next_command, pGetMeta(command)->endofstorage);
        memcpy(next_command, command, command_len);
        pSize(next_command) += command_len;

        switch(ctx->type) { // NOLINT
        case EXECUTABLE: {
                pPushBytes(next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");
                pPushStr(next_command, "*.o ");

                pPushStr(next_command, "-o ");
                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));
#if defined(CBUILD_WINDOWS)
                pPushStr(next_command, ".exe");
#endif
                pPushStr(next_command, " ");
                pForEach(ctx->libs, lib) {
                    pPushStr(next_command, "-l");
                    pPushBytes(next_command, *lib, strlen(*lib));
                    pPushStr(next_command, " ");
                }
                pPushBytes(next_command, &(char){'\0'}, 1);
                system((char*)next_command); // NOLINT
            } break;
        case STATIC_LIB: {
                pSize(next_command) = 0;
                pPushBytes(next_command, archiver, sizeof(archiver) - 1);

                pPushBytes(next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");

                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));
                pPushBytes(next_command, extension, sizeof(extension) - 1); 
            
                pPushStr(next_command, " ");

                pPushBytes(next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");
                pPushStr(next_command, "*.o ");
                pPushBytes(next_command, &(char){'\0'}, 1);

                system((char*)next_command); // NOLINT
            } break;
        case DYNAMIC_LIB: {
                pSize(next_command) = 0;
                pPushBytes(next_command, dll_archiver,  sizeof(dll_archiver) - 1);
                
                pPushBytes(next_command, out.c_str, out.length);
                if (!anyOf(out.c_str[out.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");

                pPushBytes(next_command, ctx->out_name, strlen(ctx->out_name));

                pPushBytes(next_command, dll_extension, sizeof(dll_extension) - 1); 

                pPushStr(next_command, " ");

                pPushBytes(next_command, intermediate.c_str, intermediate.length);
                if (!anyOf(intermediate.c_str[intermediate.length - 1], 2, (u8[]) { '/', '\\' }))
                    pPushStr(next_command, "/");
                pPushStr(next_command, "*.o ");
                pPushBytes(next_command, &(char){'\0'}, 1);

                system((char*)next_command); // NOLINT
            } break;
        default: break;
        }
    }
}
