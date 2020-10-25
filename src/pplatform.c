#include "pplatform.h"

#if defined(PSTD_WINDOWS)
#include <Windows.h>
#elif defined(PSTD_LINUX)
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(__APPLE__)
#error support for mac has to been implemented
#endif


pHandle *pNullHandle(void) {
#if defined(PSTD_WINDOWS)
    return NULL; 
#else
    return (void*)((u32) -1);
#endif
}


pHandle *pGetSTDOutHandle(void) {
#if defined(PSTD_WINDOWS)
    return GetStdHandle(STD_OUTPUT_HANDLE); 
#else
    return (void*)((u32) 1);
#endif
}

pHandle *pGetSTDInHandle(void) {
#if defined(PSTD_WINDOWS)
    return GetStdHandle(STD_INPUT_HANDLE); 
#else
    return (void*)((u32) 0);
#endif
}


pFileStat pGetFileStat(const char *file) {
    pFileStat result;
#if defined(PSTD_WINDOWS)
    WIN32_FILE_ATTRIBUTE_DATA data;
    result.exists   = GetFileAttributesEx(file, GetFileExInfoStandard, &data);
    result.filesize = ((u64)data.nFileSizeHigh << 31) | data.nFileSizeLow;
    result.creationtime = ((u64)data.ftCreationTime.dwHighDateTime << 31) 
                        | data.ftCreationTime.dwLowDateTime;
    result.accesstime   = ((u64)data.ftLastAccessTime.dwHighDateTime << 31) 
                        | data.ftLastAccessTime.dwLowDateTime;
    result.writetime    = ((u64)data.ftLastWriteTime.dwHighDateTime << 31) 
                        | data.ftLastWriteTime.dwLowDateTime;
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
#if defined(PSTD_WINDOWS)
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
#if defined(PSTD_WINDOWS)
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
#if defined(PSTD_WINDOWS)
    CloseHandle(handle);
#else
    u32 fh = (u32)((void *)handle);
    close(fh);
#endif
}

bool pFileWrite(pHandle *handle, String buf) {
#if defined(PSTD_WINDOWS)
    return WriteFile(handle, buf.c_str, (u32)buf.length, NULL, NULL);
#else
    s32 result = write((u64)(void*)handle, buf.c_str, (u32)buf.length);
    return result == -1 ? false : true;
#endif
}

bool pFileRead(pHandle *handle, String buf) {
#if defined(PSTD_WINDOWS)
    return ReadFile(handle, buf.c_str, buf.length, NULL, NULL);
#else
    s32 result = read((u64)(void*)handle, buf.c_str, buf.length);
    return result == -1 ? false : true;
#endif
}
