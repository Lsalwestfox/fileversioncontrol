/*****************************************************************************.
 *
 * FILE        : FVCNT.C
 * PROJECT     : File Version Control (FVC)
 * COMPANY     : 
 * AUTHOR      : Lsalwestfox
 * CREATED     : D09/26/2026T15:09:09-UTC
 * EDITED      : D09/26/2026T15:09:09-UTC
 * DESCRIPTION : Windows NT layer of the tool
 * NOTICE      : Copyright (C) 2026 Lsalwestfox
 *
 *****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "FVC.C"

static int _errno7;

int get_errno()
{
    return(_errno7);
}

void *talloc(ullong amount)
{
    static char    start_page[16l * 1024l * 1024l];
    static char   *memory = start_page;
    static ullong  capacity = sizeof(start_page);
    static ullong  head;

    head   = (head   + 15) & -16;
    amount = (amount + 15) & -16;
    if (head + amount > capacity)
    {
        capacity *= 2;
        memory = (char*)VirtualAlloc(0,
                                     capacity,
                                     MEM_RESERVE | MEM_COMMIT,
                                     PAGE_READWRITE);
        head = amount;
        return(memory);
    }
    else
    {
        ullong i = 0;
        while (i < amount)
            memory[head + i++] = 0;

        head += amount;
        return(memory + head - amount);
    }
}

string8_t read_entire_file(const char *path)
{
    HANDLE        handle;
    string8_t     out;
    DWORD         n;
    
    _errno7 = 0;
    handle = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (handle == INVALID_HANDLE_VALUE)
    {
        _errno7 = 1;
        return(NULLSTR);
    }

    if ((n = GetFileSize(handle, 0)) == INVALID_FILE_SIZE)
    {
        _errno7 = 2;
        CloseHandle(handle);
        return(NULLSTR);
    }

    out = STR((char*)talloc(n + 1), n);
    ReadFile(handle, out.str, out.size, &n, 0);
    CloseHandle(handle);
    return(out);
}

writer open_writer(const char *path)
{
    HANDLE handle = CreateFileA(path, GENERIC_WRITE, 7, 0,
                                TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return(0);
    }
    else
    {
        return(handle);
    }
}

void cwrite_to(writer write, const char *str)
{
    int l = 0;
    for (; str[l]; l++);
    write_to(write, STR(str, l));
}

void write_to(writer write, string8_t content)
{
    if (write)
    {
        DWORD _;
        WriteFile(write, content.str, content.size, &_, 0);
    }
}

void cprint(const char *str)
{
    int l = 0;
    for (; str[l]; l++);
    print(STR(str, l));
}

void print(string8_t string)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(handle, string.str, string.size, 0, 0);
}

string8_t timestamp()
{
    string8_t  out;
    SYSTEMTIME time;
    GetSystemTime(&time);
    out.str = (char*)talloc(out.size = 24);
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
    return(out);
}

void _start()
{
    LPSTR   os_command_line = GetCommandLineA();
    char   *buffer          = 0;
    char  **argv            = 0;
    int     argc            = 0;
    int     i               = -1;
    int     j               = 0;
    bool    string          = 0;
    bool    flag            = 1;
    bool    skip            = 0;
    int     ch              = 0;
    int     k               = -1;

    while ((ch = os_command_line[++k]))
    {
        bool white = ch == ' ' || ch == '\r';

        if (skip)
        {
            skip = 0;
            continue;
        }

        if (flag && !white)
        {
            argc++;
            flag = 0;
        }

        if (!string && white)
            flag = 1;

        if (ch == '"')
            string = !string; 

        if (ch == '\\')
            skip = 1;
    }

    i      = 0;
    j      = 0;
    skip   = 0;
    string = 0;
    flag   = 1;
    argv   = (char**)talloc(k + argc * sizeof(*argv) + 32);
    buffer = (char*)(argv + argc) + 16;

    for (k = 0; (ch = os_command_line[k]); k++)
    {
        bool white = ch == ' ' || ch == '\r';

        if (skip)
        {
            skip = 0;
            buffer[i++] = ch;
            continue;
        }

        if (flag && !white)
        {
            buffer[i++] = 0;
            argv[j++] = buffer + i;
            flag = 0;
        }

        if (!string && white)
        {
            flag = 1;
            continue;
        }

        if (ch == '"')
        {
            string = !string;
            continue;
        }

        if (ch == '\\')
        {
            skip = 1;
            continue;
        }

        buffer[i++] = ch;
    }

    ExitProcess(main(j, argv));
}

/* CRT requirement */
void *memset(void *dest, int c, SIZE_T size)
{
    char *d = (char*)dest;
    while(size--) *d++ = c;
    return(dest);
}

/*****************************************************************************,
 *
 * HISTORY:
 *
 * [D09/26/2026T15:09:09-UTC] Initial commit
 *    - Lsalwestfox
 *
 *****************************************************************************/

