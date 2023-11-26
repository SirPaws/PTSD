#if !defined(PPLATFORM_HEADER_ONLY)
#include "pplatform.h"
#include "pstacktrace.h"
#endif


#if defined(PTSD_WASM)
#include <emscripten.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(PTSD_WINDOWS)
#define PATH_MAX (32767)
#include <Windows.h>
#include <handleapi.h>
#include <memoryapi.h>
#include <winnt.h>
#elif defined(PTSD_LINUX)
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#elif defined(__APPLE__)
#error support for mac has to been implemented
#endif 


phandle_t *pnull_handle(void) {
#if defined(PTSD_WINDOWS)
    return NULL; 
#else
    return (void*)((u32) -1);
#endif
}

bool penable_console_color_output(void) {
#if defined(PTSD_WINDOWS)
    bool result = SetConsoleMode(GetModuleHandle(NULL), ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return result != 0;
#else
    return true; // don't know if it's enabled but we assume it is
#endif
}

phandle_t *pget_stdout_handle(void) {
#if defined(PTSD_WINDOWS)
    return GetStdHandle(STD_OUTPUT_HANDLE); 
#else
    return (void*)((u32) 1);
#endif
}

phandle_t *pget_stdin_handle(void) {
#if defined(PTSD_WINDOWS)
    return GetStdHandle(STD_INPUT_HANDLE); 
#else
    return (void*)((u32) 0);
#endif
}


pstring_t pfullpath(pstring_t path) {
    char input[PATH_MAX + 1];
    if (path.length > PATH_MAX) {
        panic("path '%S' is too long!", path);
    }
    memset(input, 0, path.length + 1);
    memcpy(input, path.c_str, path.length);

#if defined(PTSD_WINDOWS)

    phandle_t *handle = pfile_open(input, P_READ_ACCESS);
    if (handle == NULL) {
        panic("could not get full path for '%S'", path);
    }

    BY_HANDLE_FILE_INFORMATION info = {0};
    if (!GetFileInformationByHandle(handle, &info)) {
        pfile_close(handle);
        panic("could not get full path for '%S'", path);
    }
    if (info.nNumberOfLinks > 1) {
        
    }

    u32 size = GetFinalPathNameByHandle(handle, NULL, 0, FILE_NAME_NORMALIZED);
    if (size == 0) {
        pfile_close(handle);
        panic("could not get full path for '%S'", path);
    }

    char *buffer = pzero_allocate(size + 1);
    if (!buffer) {
        pfile_close(handle);
        panic("could not get full path for '%S'", path);
    }

    u32 actual_size = GetFinalPathNameByHandle(handle, buffer, size + 1, FILE_NAME_NORMALIZED);
    if (actual_size != size) {
        pfile_close(handle);
        panic("could not get full path for '%S'", path);
    }

    pfile_close(handle);
    return pstring(buffer, size);
#else
    char *result = realpath(input, buffer);
    if (result == NULL) {
        panic("could not get full path for '%S'", path);
    }
#endif
}

pfilestat_t pstat_file(phandle_t *handle) {
#if defined(PTSD_WINDOWS)
    if (!handle || handle == INVALID_HANDLE_VALUE) return (pfilestat_t){0};//NOLINT
#else
    (void)handle;
#endif
    return (pfilestat_t){0};
}

pfilestat_t pget_filestat(const char *file) {
    pfilestat_t result;
    if (!file) return (pfilestat_t){0};

#if defined(PTSD_WINDOWS)
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
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
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

pfilestat_ex_t pfilestat_ex(const char *file, bool include_link_path) {
#define WIN32_COMBINE_HIGH_LOW( high, low ) ((u64)(high) << 31 | (low)) 
    if (!file) return (pfilestat_ex_t){0};

    pfilestat_ex_t result = {0};

    WIN32_FILE_ATTRIBUTE_DATA data;
    bool exists = GetFileAttributesEx(file, GetFileExInfoStandard, &data);
    if (!exists) return (pfilestat_ex_t){.type = PFT_NOT_FOUND};

    result.type = PFT_REGULAR;
    if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        result.type = PFT_SYMLINK;
    else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        result.type = PFT_DIRECTORY;

    result.exists = exists;
    result.filesize = WIN32_COMBINE_HIGH_LOW(data.nFileSizeHigh, data.nFileSizeLow);

    result.creationtime = WIN32_COMBINE_HIGH_LOW(data.ftCreationTime.dwHighDateTime, 
            data.ftCreationTime.dwLowDateTime);
    result.accesstime   = WIN32_COMBINE_HIGH_LOW(data.ftLastAccessTime.dwHighDateTime,
            data.ftLastAccessTime.dwLowDateTime);
    result.writetime    = WIN32_COMBINE_HIGH_LOW(data.ftLastWriteTime.dwHighDateTime, 
            data.ftLastWriteTime.dwLowDateTime);
    result.num_links = 1;
    
    result.is_hidden       = data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
    result.is_readonly     = data.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    result.is_system_owned = data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
    result.is_temporary    = data.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;
    
    phandle_t *handle = pfile_open(file, P_READ_ACCESS);
    BY_HANDLE_FILE_INFORMATION info = {0};
    if (!GetFileInformationByHandle(handle, &info)) {
        pfile_close(handle);
        return result;
    }

    result.num_links = info.nNumberOfLinks;
    result.link_exists   = true;
    result.link_filesize = WIN32_COMBINE_HIGH_LOW(info.nFileSizeHigh, info.nFileSizeLow);

    result.link_creationtime = WIN32_COMBINE_HIGH_LOW(info.ftCreationTime.dwHighDateTime, 
            info.ftCreationTime.dwLowDateTime);
    result.link_accesstime   = WIN32_COMBINE_HIGH_LOW(info.ftLastAccessTime.dwHighDateTime,
            info.ftLastAccessTime.dwLowDateTime);
    result.link_writetime    = WIN32_COMBINE_HIGH_LOW(info.ftLastWriteTime.dwHighDateTime, 
            info.ftLastWriteTime.dwLowDateTime);
    
    result.is_link_hidden       = info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
    result.is_link_readonly     = info.dwFileAttributes & FILE_ATTRIBUTE_READONLY;
    result.is_link_system_owned = info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM;
    result.is_link_temporary    = info.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY;

    result.id = WIN32_COMBINE_HIGH_LOW(info.nFileIndexHigh, info.nFileIndexLow);

    if (include_link_path) {
        u32 size = GetFinalPathNameByHandle(handle, NULL, 0, FILE_NAME_NORMALIZED);
        if (size == 0) {
            pfile_close(handle);
            panic("could not get full path for '%s'", file);
        }

        char *buffer = pzero_allocate(size + 1);
        if (!buffer) {
            pfile_close(handle);
            panic("could not get full path for '%s'", file);
        }

        u32 actual_size = GetFinalPathNameByHandle(handle, buffer, size + 1, FILE_NAME_NORMALIZED);
        if (actual_size != size) {
            pfile_close(handle);
            panic("could not get link path for '%s'", file);
        }

        result.link_target = pstring(buffer, size);
    }

    pfile_close(handle);
    return result;
#undef WIN32_COMBINE_HIGH_LOW
}

pfilestat_ex_t pfilestat_exs(pstring_t str, bool include_link_path) {
    pstring_t copy = pcopy_string(str);
    pfilestat_ex_t result = pfilestat_ex(copy.c_str, include_link_path);
    pfree_string(&copy);
    return result;
}

u64 pfile_id(const char *file) {
#define WIN32_COMBINE_HIGH_LOW( high, low ) ((u64)(high) << 31 | (low)) 
    if (!file) return 0;
    phandle_t *handle = pfile_open(file, P_READ_ACCESS);
    BY_HANDLE_FILE_INFORMATION info = {0};
    if (!GetFileInformationByHandle(handle, &info)) {
        pfile_close(handle);
        return 0;
    }

    u64 id = WIN32_COMBINE_HIGH_LOW(info.nFileIndexHigh, info.nFileIndexLow);
    pfile_close(handle);
    return id;
#undef WIN32_COMBINE_HIGH_LOW
}

u64 pfile_ids(pstring_t file) {
    pstring_t copy = pcopy_string(file);
    u64 result = pfile_id(copy.c_str);
    pfree_string(&copy);
    return result;
}

phandle_t *pfile_open(const char *filename, pfile_access_t access) {
#if defined(PTSD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#elif  defined(PTSD_LINUX)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    return (void*)(u64)open(filename, file_access);
#elif  defined(PTSD_WASM)
    int file_access = -1;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : -1;
    file_access = access & P_WRITE_ACCESS ? ((!access) ? O_RDWR : O_WRONLY ) : -1;
    return (void*)(u64)open(filename, file_access);
#endif
}
phandle_t *pfile_create(const char *filename, pfile_access_t access) {
#if defined(PTSD_WINDOWS)
    DWORD file_access;
    file_access  = access & P_READ_ACCESS  ? GENERIC_READ  : 0;
    file_access |= access & P_WRITE_ACCESS ? GENERIC_WRITE : 0;
    return CreateFile(filename, file_access, 0, 
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#elif  defined(PTSD_LINUX)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : 0;
    file_access = access & P_WRITE_ACCESS ? (access ? O_RDWR : O_WRONLY ) : 0;
    file_access|= O_CREAT;
    return (void*)(u64)open(filename, file_access);
#elif  defined(PTSD_WASM)
    int file_access;
    file_access = access & P_READ_ACCESS  ? O_RDONLY : -1;
    file_access = access & P_WRITE_ACCESS ? ((!access) ? O_RDWR : O_WRONLY ) : -1;
    file_access|= O_CREAT;
    return (void*)(u64)open(filename, file_access);
#endif
}

void pfile_close(phandle_t *handle) {
#if defined(PTSD_WINDOWS)
    CloseHandle(handle);
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    u64 fh = (u64)((void *)handle);
    close(fh);
#endif
}

bool pfile_write(phandle_t *handle, pstring_t buf) {
#if defined(PTSD_WINDOWS)
    return WriteFile(handle, buf.c_str, (u32)buf.length, NULL, NULL);
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    s32 result = write((u64)(void*)handle, buf.c_str, (u32)buf.length);
    return result == -1 ? false : true;
#endif
}

bool pfile_read(phandle_t *handle, pbuffer_t buf) {
#if defined(PTSD_WINDOWS)
    DWORD bytes_read = 0;
    ReadFile(handle, buf.c_str, buf.length, &bytes_read, NULL);
    return bytes_read != 0;
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    s32 result = read((u64)(void*)handle, buf.c_str, buf.length);
    return result == -1 ? false : true;
#endif
}

bool pseek(phandle_t *handle, isize size, enum pseek_mode_t mode) {//NOLINT
#if defined(PTSD_WINDOWS)
    
    DWORD wmode;
    switch (mode) {
    case P_SEEK_SET: wmode = FILE_BEGIN; break;
    case P_SEEK_END: wmode = FILE_END;   break;
    case P_SEEK_CURRENT:
    default: wmode = FILE_CURRENT;
    }

    if (size > LONG_MAX) panic("seek size too long");

    DWORD result = SetFilePointer(handle, (long)size, 0, wmode);
    return result != INVALID_SET_FILE_POINTER;
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
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

void *pmemory_map_file(phandle_t *handle, pfile_access_t access, u64 size, u64 offset) {//NOLINT
#if defined(PTSD_WINDOWS)

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
    LARGE_INTEGER i = {.QuadPart = (s64)offset};
    if (access & P_EXECUTABLE) file_access |= FILE_MAP_EXECUTE;
    void *mapping = MapViewOfFile(mapped_file, file_access, i.HighPart, i.LowPart, size);
    CloseHandle(mapped_file);
    return mapping;
#elif defined(PTSD_LINUX) || defined(PTSD_WASM)
    assert(false);
    return NULL;
#endif
}
bool punmap_file(void *handle) {
#if defined(PTSD_WINDOWS)
    if (UnmapViewOfFile(handle)) 
         return true;
    else return false;
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    return true;
#endif
}



char pnext_in_environment_path(const char **buffer, const char *file, char *out, usize *length);
bool pfind_in_environment_path(const char *file, pstring_t *out) {
#if defined(PTSD_WINDOWS)

    char buffer[PATH_MAX];
    usize length = GetEnvironmentVariable("Path", buffer, PATH_MAX);
    if (!length) return false;

    const char *buffer_ptr = buffer;

    char path[32767];
    while(pnext_in_environment_path(&buffer_ptr, file, path, &length)) {
        pfilestat_t stat = pget_filestat(path);
        if(stat.exists) {
            pstring_t str = {
                .c_str  = pallocate(length + 1),
                .length = length 
            };
            memcpy(str.c_str, path, length);
            str.c_str[length] = '\0';

            *out = str;
            return true;
        }
    }
    return false;
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    return false;
#endif
}

char pnext_in_environment_path(const char **buffer_ptr, const char *file, char *out, usize *length) {
#if defined(PTSD_WINDOWS)
    const char *begin = *buffer_ptr;
    const char *end   = *buffer_ptr;
    while (!(*end == '\0' || *end == ';')) end++;

    *length = end - begin;
    memcpy(out, begin, *length + 1);
    out[*length] = '\\';
    
    usize len = strlen(file);
    memcpy(out + *length + 1, file, len + 1);
    *length += len + 1;
    out[1 + *length] = '\0';
    if (*end) *buffer_ptr = end + 1;
    return *end;
#elif  defined(PTSD_LINUX) || defined(PTSD_WASM)
    return '\0';
#endif
}

