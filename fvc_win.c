/******************************************************************************.
**
** FILE        : fvc_win.c
** PROJECT     : File Version Control (FVC)
** COMPANY     : 
** AUTHOR      : Lsalwestfox
** CREATED     : D10/15/2026T10:37:38-UTC
** EDITED      : D10/15/2026T10:37:38-UTC
** DESCRIPTION : Windows platform layer
** NOTICE      : Copyright (C) 2026 Lsalwestfox
**
*******************************************************************************/

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define NOMINMAX            1
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_MEAN_AND_LEAN 1
#define VC_EXTRALEAN        1
#include <windows.h>
#include <shellapi.h>

#include <stdint.h>
#include "fvc.c"


// This is a permanent arena and is never cleared
// There's even not scratch buffer, we have infinite memory
void *talloc(imm size) {
    local_persist byte  static_memory[MB(16)];
    local_persist imm   memsize = size_of(static_memory);
    local_persist byte *memory  = static_memory;
    local_persist imm   used;
    
    umm m = (xxu memory + used + MEMORY_ALIGNMENT - 1) & -MEMORY_ALIGNMENT;
    if (m + size <= xxu memory + memsize) {
        used = m - xxu memory + size;
        return xx m;
    } else {
        memsize <<= 1;
        memory = VirtualAlloc(0, memsize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        used = size;
        return memory;
    }
}

internal WCHAR *win32_to_utf8(int *size_out, s8 string) {
    int    size = MultiByteToWideChar(CP_UTF8, 0, string.str, string.len, 0, 0);
    WCHAR *wstr = talloc((size + 1) * sizeof *wstr);
    MultiByteToWideChar(CP_UTF8, 0, string.str, string.len, wstr, size);
    if (size_out) {
        *size_out = size;
    }
    return wstr;
}

void print(s8 message) {
    int    size = 0;
    WCHAR *wstr = win32_to_utf8(&size, message);
    wstr[size] = '\n';
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), wstr, size + 1, 0, 0);
}

bool mkdir(s8 path) {
    WCHAR *wpath = win32_to_utf8(0, path);
    DWORD  attr  = GetFileAttributesW(wpath);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return 0;
    else
        return !CreateDirectoryW(wpath, 0);
}

u64 get_file_hash(s8 path) {
    WCHAR                     *wpath = win32_to_utf8(0, path);
    WIN32_FILE_ATTRIBUTE_DATA  data  = {0};
    GetFileAttributesExW(wpath, GetFileExInfoStandard, &data);
    return ((u64)data.ftLastWriteTime.dwHighDateTime << 32l) | data.ftLastWriteTime.dwLowDateTime;
}

file_t open_file(imm *size, s8 path, int mode) {
    WCHAR  *wpath  = win32_to_utf8(0, path);
    HANDLE  handle = INVALID_HANDLE_VALUE;
    switch (mode) {
        case FILE_READ: {
            handle = CreateFileW(wpath,
                                 GENERIC_READ,
                                 FILE_SHARE_READ,
                                 0,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 0);
        } break;
        
        case FILE_WRITE: {
            handle = CreateFileW(wpath,
                                 GENERIC_WRITE,
                                 0,
                                 0,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 0);
        } break;
        
        case FILE_TRUNCATE: {
            handle = CreateFileW(wpath,
                                 GENERIC_WRITE,
                                 0,
                                 0,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 0);
        } break;
    }
    
    if (handle == INVALID_HANDLE_VALUE) {
        handle = 0;
    } else if (size) {
        LARGE_INTEGER li = {0};
        GetFileSizeEx(handle, &li);
        *size = li.QuadPart;
    }
    
    return handle;
}

void read_file(void *out, imm offset, imm size, file_t file) {
    if (file) {
        DWORD _;
        LARGE_INTEGER li = {.QuadPart = offset};
        SetFilePointerEx(file, li, &li, FILE_BEGIN);
        ReadFile(file, out, size, &_, 0);
    }
}

void write_file(file_t file, imm offset, imm size, void *in) {
    if (file) {
        DWORD _;
        LARGE_INTEGER li = {.QuadPart = offset};
        SetFilePointerEx(file, li, &li, FILE_BEGIN);
        WriteFile(file, in, size, &_, 0);
    }
}

void close_file(file_t file) {
    if (file) {
        CloseHandle(file);
    }
}

s8 get_timestamp() {
    s8         out;
    SYSTEMTIME time;
    GetSystemTime(&time);
    out.str = talloc(out.len = 24);
    out.str[0]  = 'D';
    out.str[1]  = '0' + time.wMonth / 10;
    out.str[2]  = '0' + time.wMonth % 10;
    out.str[3]  = '/';
    out.str[4]  = '0' + time.wDay / 10;
    out.str[5]  = '0' + time.wDay % 10;
    out.str[6]  = '/';
    out.str[7]  = '0' + time.wYear / 1000 % 10;
    out.str[8]  = '0' + time.wYear / 100  % 10;
    out.str[9]  = '0' + time.wYear / 10   % 10;
    out.str[10] = '0' + time.wYear        % 10;
    out.str[11] = 'T';
    out.str[12] = '0' + time.wHour / 10;
    out.str[13] = '0' + time.wHour % 10;
    out.str[14] = ':';
    out.str[15] = '0' + time.wMinute / 10;
    out.str[16] = '0' + time.wMinute % 10;
    out.str[17] = ':';
    out.str[18] = '0' + time.wSecond / 10;
    out.str[19] = '0' + time.wSecond % 10;
    out.str[20] = '-';
    out.str[21] = 'U';
    out.str[22] = 'T';
    out.str[23] = 'C';
    return out;
}

void _start() {
    // Get UTF-16 argv and also argc
    int     argc  = 0;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!wargv) ExitProcess(1);
    
    // Get how much bytes need for UTF-8 argv
    imm argv_size = 0;
    for (int i = 0; i < argc; ++i) {
        argv_size += WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    }
    
    // Get UTF-8 argv
    char **argv = talloc(argv_size + sizeof(*argv) * argc);
    char  *text = xx (argv + argc);
    for (int i = 0; i < argc; ++i) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, text, 9999999l, 0, 0);
        argv[i] = text;
        text += len;
    }
    
    // Call main
    LocalFree(wargv);
    ExitProcess(main(argc, argv));
}

// CRT stuff
// int _fltused = 0x9875;

void *memset(void *d, int s, size_t sz) {
    byte *bd = d;
    while(sz--) *bd++ = s;
    return d;
}

void *memcpy(void *d, void const *s, size_t sz) {
    const byte *bs = s;
    byte       *bd = d;
    while(sz--) *bd++ = *bs++;
    return d;
}

/******************************************************************************,
**
** HISTORY:
**
** At D10/15/2026T10:37:38-UTC by Lsalwestfox
**    - Initial commit
**
*******************************************************************************/
