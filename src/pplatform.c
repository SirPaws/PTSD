#if !defined(PPLATFORM_HEADER_ONLY)
#include "pplatform.h"
#endif


#if defined(PSTD_WASM)
#include <emscripten.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(PSTD_WINDOWS)
#include <Windows.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <winnt.h>
#elif defined(PSTD_LINUX)
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(__APPLE__)
#error support for mac has to been implemented
#endif 


phandle_t *pnull_handle(void) {
#if defined(PSTD_WINDOWS)
    return NULL; 
#else
    return (void*)((u32) -1);
#endif
}

pbool_t penable_console_color_output(void) {
#if defined(PSTD_WINDOWS)
    pbool_t result = SetConsoleMode(GetModuleHandle(NULL), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return result != 0;
#else
    return true; // don't know if it's enabled but we assume it is
#endif
}

phandle_t *pget_stdout_handle(void) {
#if defined(PSTD_WINDOWS)
    return GetStdHandle(STD_OUTPUT_HANDLE); 
#else
    return (void*)((u32) 1);
#endif
}

phandle_t *pget_stdin_handle(void) {
#if defined(PSTD_WINDOWS)
    return GetStdHandle(STD_INPUT_HANDLE); 
#else
    return (void*)((u32) 0);
#endif
}

pfilestat_t pstat_file(phandle_t *handle) {
#if defined(PSTD_WINDOWS)
    if (!handle || handle == INVALID_HANDLE_VALUE) return (pfilestat_t){0};
#else
#endif
    return (pfilestat_t){0};
}

pfilestat_t pget_filestat(const char *file) {
    pfilestat_t result;
    if (!file) return (pfilestat_t){0};

#if defined(PSTD_WINDOWS)
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
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
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

phandle_t *pfile_open(const char *filename, pfile_access_t access) {
#if defined(PSTD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#elif  defined(PSTD_LINUX)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    return (void*)(u64)open(filename, file_access);
#elif  defined(PSTD_WASM)
    int file_access = -1;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : -1;
    file_access = access & P_WRITE_ACCESS ? ((!access) ? O_RDWR : O_WRONLY ) : -1;
    return (void*)(u64)open(filename, file_access);
#endif
}
phandle_t *pfile_create(const char *filename, pfile_access_t access) {
#if defined(PSTD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#elif  defined(PSTD_LINUX)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    file_access|= O_CREAT;
    return (void*)(u64)open(filename, file_access);
#elif  defined(PSTD_WASM)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : -1;
    file_access = access & P_WRITE_ACCESS ? ((!access) ? O_RDWR : O_WRONLY ) : -1;
    file_access|= O_CREAT;
    return (void*)(u64)open(filename, file_access);
#endif
}

void pfile_close(phandle_t *handle) {
#if defined(PSTD_WINDOWS)
    CloseHandle(handle);
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
    u32 fh = (u32)((void *)handle);
    close(fh);
#endif
}

pbool_t pfile_write(phandle_t *handle, pstring_t buf) {
#if defined(PSTD_WINDOWS)
    return WriteFile(handle, buf.c_str, (u32)buf.length, NULL, NULL);
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
    s32 result = write((u64)(void*)handle, buf.c_str, (u32)buf.length);
    return result == -1 ? false : true;
#endif
}

pbool_t pfile_read(phandle_t *handle, pbuffer_t buf) {
#if defined(PSTD_WINDOWS)
    DWORD bytes_read = 0;
    ReadFile(handle, buf.c_str, buf.length, &bytes_read, NULL);
    return bytes_read != 0;
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
    s32 result = read((u64)(void*)handle, buf.c_str, buf.length);
    return result == -1 ? false : true;
#endif
}

pbool_t pseek(phandle_t *handle, isize size, enum pseek_mode_t mode) {
#if defined(PSTD_WINDOWS)
    
    DWORD wmode;
    switch (mode) {
    case P_SEEK_SET: wmode = FILE_BEGIN; break;
    case P_SEEK_END: wmode = FILE_END;   break;
    case P_SEEK_CURRENT:
    default: wmode = FILE_CURRENT;
    }

    DWORD result = SetFilePointer(handle, size, 0, wmode);
    return result != INVALID_SET_FILE_POINTER;
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
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

void *pmemory_map_file(phandle_t *handle, pfile_access_t access, u64 size, u64 offset) {
#if defined(PSTD_WINDOWS)

    DWORD protection = 0;
    switch (access & (P_WRITE_ACCESS|P_READ_ACCESS)) {
    case P_WRITE_ACCESS|P_READ_ACCESS: 
        protection = access & P_EXECUTABLE ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE; break;
    case P_READ_ACCESS: protection = access & P_EXECUTABLE ? PAGE_EXECUTE_READ : PAGE_READONLY; break;
    case P_WRITE_ACCESS: return NULL;
    }

    phandle_t *mapped_file = CreateFileMapping(handle, NULL, protection, 0, 0, NULL);
    if (mapped_file == NULL) return NULL;

    DWORD file_access = 0;
    switch (access & (P_WRITE_ACCESS|P_READ_ACCESS)) {
    case P_WRITE_ACCESS|P_READ_ACCESS: file_access = FILE_MAP_ALL_ACCESS; break;
    case P_READ_ACCESS:                file_access = FILE_MAP_READ; break;
    case P_WRITE_ACCESS:               file_access = FILE_MAP_WRITE; break;
    }
    LARGE_INTEGER i = {.QuadPart = offset};
    if (access & P_EXECUTABLE) file_access |= FILE_MAP_EXECUTE;
    void *mapping = MapViewOfFile(mapped_file, file_access, i.HighPart, i.LowPart, size);
    CloseHandle(mapped_file);
    return mapping;
#elif  defined(PSTD_LINUX) || defined(PSTD_WASM)
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
pbool_t punmap_file(void *handle) {
    if (UnmapViewOfFile(handle)) 
         return true;
    else return false;
}
