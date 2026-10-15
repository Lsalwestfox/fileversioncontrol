/******************************************************************************.
**
** FILE        : fvc.c
** PROJECT     : File Version Control (FVC)
** COMPANY     : 
** AUTHOR      : Lsalwestfox
** CREATED     : D10/15/2026T10:37:38-UTC
** EDITED      : D10/15/2026T11:14:16-UTC
** DESCRIPTION : Platform-indepedent layer
** NOTICE      : Copyright (C) 2026 Lsalwestfox
**
*******************************************************************************/

//
// This file must be included in the platform file for unity build
// It requires some types defined before including it, this is made
// like stdint on purpose so if you use stdint (which you probably are)
// you can just include this file, it's not always present so it's not
// here and must be in platform layer
//

//
// Also, this file works only on little-endian machines
//

typedef unsigned char byte;
typedef void *file_t;
typedef int bool;

typedef uint16_t  u16;
typedef int16_t   i16, s16;
typedef uint32_t  u32;
typedef int32_t   i32, s32;
typedef uint64_t  u64;
typedef int64_t   i64, s64;
typedef uintptr_t umm;
typedef intptr_t  imm, smm;

#define size_of (imm)sizeof
#define cast(x) (x)
#define xx cast(void*)
#define xxi cast(imm)
#define xxu cast(umm)
#define unused (void)

#define KB(x) (  (x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

#define local_persist static
#define internal static
#define global static

#define MEMORY_ALIGNMENT 16

typedef struct {
    char *str;
    imm   len;
} s8;
#define ms8(str, len) ((s8){str, len})
#define cs8(x) ms8(xx (x), size_of(x) - 1)
#define zs8 ms8(0, 0)

typedef struct {
    byte *data;
    imm   size;
} u8;
#define md8(data, size) ((u8){data, size})
#define zd8 md8(0, 0)

#define REPO_RIFF ('F' | ('V' << 8l) | ('C' << 16l) | ('1' << 24l))
typedef struct {
    u32 riff;
    u32 num_files;
} repo_header;

typedef struct {
    u64  *hashes;
    s8   *names;
    u32   length;
} repo_files;

// -----
// Platform layer definitions
// -----

// Memory allocator, this can be page allocator or permanent arena (recommended)
// I will not free any memory whatsoever, deal with it
// Returned memory is zeroed and aligned to MEMORY_ALIGNMENT
void *talloc(imm size);

// Print message to the console, it's in UTF-8
void print(s8 message);

// Created directory, return 0 on success and 1 on failure,
// If this is stub where mkdir is not available, return 1
bool mkdir(s8 path);

// Even if it says hash it actually returns last edited time of a file
// If OS don't has last edited time this is when you use hash, hash must never be 0
// If path not found or something wend wrong return 0
u64 get_file_hash(s8 path);

// Get's current time in DMM/dd/YYYYThh:mm:ss-TIMEZONE format, UTC preffered
s8 get_timestamp();

// Low-level file I/O
enum { FILE_READ, FILE_WRITE, FILE_TRUNCATE };
file_t open_file(imm *size, s8 path, int mode);
void read_file(void *out, imm offset, imm size, file_t file);
void write_file(file_t file, imm offset, imm size, void *in);
void close_file(file_t file);

// -----
// Platform indepedent code
// -----

s8 from_cstr(char *str) {
    s8 out = ms8(str, 0);
    while (str[out.len]) {
        ++out.len;
    }
    return out;
}

s8 str_concat(s8 a, s8 b) {
    s8 out = ms8(talloc(a.len + b.len), a.len + b.len);
    memcpy(out.str,         a.str, a.len);
    memcpy(out.str + a.len, b.str, b.len);
    return out;
}

bool str_equal(s8 a, s8 b) {
    if (a.len != b.len)
        return 0;
    
    for (int i = 0; i < a.len; ++i) {
        if (a.str[i] != b.str[i])
            return 0;
    }
    
    return 1;
}

s8 str_trim(s8 str) {
    while (str.len && (str.str[0] == ' ' || str.str[0] == '\t')) {
        ++str.str;
        --str.len;
    }
    
    while (str.len && (str.str[str.len - 1] == ' ' || str.str[str.len - 1] == '\t')) {
        --str.len;
    }
    
    return str;
}

u8 read_entire_file(s8 path) {
    u8     out  = zd8;
    file_t file = open_file(&out.size, path, FILE_READ);
    if (file) {
        out.data = talloc(out.size);
        read_file(out.data, 0, out.size, file);
    }
    close_file(file);
    return out;
}

void write_entire_file(s8 path, u8 data) {
    file_t file = open_file(0, path, FILE_TRUNCATE);
    write_file(file, 0, data.size, data.data);
    close_file(file);
}

s8 *split_lines(imm *num_lines, s8 string) {
    imm result_num = 1;
    
    for (imm i = 0; i < string.len; ++i) {
        if (string.str[i] == '\n') {
            ++result_num;
        }
    }
    
    imm   j      = 0;
    imm   start  = 0;
    s8   *result = talloc(result_num * size_of *result);
    for (imm i = 0; i < string.len; ++i) {
        if (string.str[i] == '\n') {
            s8 *p = result + j++;
            p->str = string.str + start;
            p->len = i - start;
            
            if (i && string.str[i - 1] == '\r') {
                --p->len;
            }
            
            start = i + 1;
        }
    }
    
    s8 *p = result + j;
    p->str = string.str + start;
    p->len = string.len - start;
    
    if (num_lines) {
        *num_lines = result_num;
    }
    
    return result;
}

bool is_line_history_header(s8 line) {
    if (line.len != 80) {
        return 0;
    }
    
    if (line.str[0] != '/' || line.str[79] != ',') {
        return 0;
    }
    
    for (imm i = 1; i < 79; ++i) {
        if (line.str[i] != '*') {
            return 0;
        }
    }
    
    return 1;
}

bool is_line_info_header(s8 line) {
    if (line.len != 80) {
        return 0;
    }
    
    if (line.str[0] != '/' || line.str[79] != '.') {
        return 0;
    }
    
    for (imm i = 1; i < 79; ++i) {
        if (line.str[i] != '*') {
            return 0;
        }
    }
    
    return 1;
}

bool is_line_closing_header(s8 line) {
    if (line.len != 80) {
        return 0;
    }
    
    if (line.str[79] != '/') {
        return 0;
    }
    
    for (imm i = 0; i < 79; ++i) {
        if (line.str[i] != '*') {
            return 0;
        }
    }
    
    return 1;
}

bool is_line_empty(s8 line) {
    for (imm i = 0; i < line.len; ++i) {
        if (line.str[i] != ' ' && line.str[i] != '\t') {
            return 0;
        }
    }
    
    return 1;
}

char str_consume(s8 *str) {
    if (str->len) {
        --str->len;
        ++str->str;
        return str->str[-1];
    }
    
    return 0;
}

imm is_line_last_edited(s8 line) {
    if (str_consume(&line) != '*' || str_consume(&line) != '*') {
        return -1;
    }
    
    imm index = 1;
    char last = 0;
    do {
        ++index;
        last = str_consume(&line);
    } while (last == ' ' || last == '\t');
    
    if (last != 'E' ||
        str_consume(&line) != 'D' ||
        str_consume(&line) != 'I' ||
        str_consume(&line) != 'T' ||
        str_consume(&line) != 'E' ||
        str_consume(&line) != 'D') {
        return -1;
    }
    
    index += 6;
    do {
        ++index;
        last = str_consume(&line);
    } while (last == ' ' || last == '\t');
    
    if (last != ':') {
        return -1;
    }
    
    return index;
}

repo_files get_repo_files() {
    repo_files   files  = {0};
    repo_header *header = 0;
    u8           data   = read_entire_file(cs8(".fvc/repo.fvc"));
    
    header = xx data.data;
    if (data.size < 8 || header->riff != REPO_RIFF) {
        return cast(repo_files) {0};
    }
    
    files.length = header->num_files;
    files.hashes = xx (header + 1);
    files.names  = talloc(files.length * size_of *files.names);
    
    imm j = xxu (files.hashes + files.length) - xxu data.data;
    if (j > data.size) {
        return cast(repo_files) {0};
    }
    
    for (u32 i = 0; i < files.length; ++i) {
        int start = j;
        while (j < data.size) {
            if (!data.data[j])
                break;
            else
                ++j;
        }
        
        files.names[i] = ms8(xx (data.data + start), j - start);
        if (j < data.size) ++j;
    }
    
    return files;
}

imm write_str(file_t file, imm p, s8 str) {
    write_file(file, p, str.len, str.str);
    return p + str.len;
}

// Argv is in UTF-8, always, if you don't have UTF-8 use ASCII, they're compatible
int main(int argc, char **argv) {
    if (argc <= 1) {
        print(cs8("File Version Control v0.1 by Lsalwestfox"));
        print(cs8("Usage: fvc [command] [flags]"));
        print(cs8("   init              - initializes the repo"));
        print(cs8("   add [files...]    - adds files to the repo to check"));
        print(cs8("   remove [files...] - remove files from the repo to check"));
        print(cs8("   commit [message]  - commit message to the repo"));
        print(cs8("   config            - view repo's config"));
        print(cs8("   diff              - show files that commit will change"));
        print(cs8("   fix               - registers file changes without commiting"));
        return 0;
    }
    
    imm  user_sys_lines;
    u8   user_sys_data = read_entire_file(cs8(".fvc/user.sys"));
    s8  *user_sys_lines_data = split_lines(&user_sys_lines, *cast(s8*) &user_sys_data);
    
    s8 user_sys_user      = cs8("Anon");
    s8 user_sys_project   = zs8;
    s8 user_sys_copyright = zs8;
    s8 user_sys_company   = zs8;
    
    for (imm i = 0; i < user_sys_lines; ++i) {
        s8 line = user_sys_lines_data[i];
        if (line.str[0] == ';' || is_line_empty(line)) {
            continue;
        }
        line = str_trim(line);
        
        s8 directive = line;
        s8 value     = zs8;
        
        // If no separator found, we do nothing which means directive is entire line without value
        for (imm j = 0; j < line.len; ++j) {
            if (line.str[j] == ' ' || line.str[j] == '\t' || line.str[j] == '=') {
                directive = str_trim(ms8(line.str, j++));
                value     = str_trim(ms8(line.str + j, line.len - j));
                break;
            }
        }
        
        // Normalizing directive because they are case-insensitive
        for (imm j = 0; j < directive.len; ++j) {
            if (directive.str[j] >= 'a' && directive.str[j] <= 'z') {
                directive.str[j] -= 0x20;
            }
        }
        
        if (str_equal(directive, cs8("REM"))) {
            // Do nothing
        } else if (str_equal(directive, cs8("USER"))) {
            user_sys_user = value;
        } else if (str_equal(directive, cs8("PROJECT"))) {
            user_sys_project = value;
        } else if (str_equal(directive, cs8("COPYRIGHT"))) {
            user_sys_copyright = value;
        } else if (str_equal(directive, cs8("COMPANY"))) {
            user_sys_company = value;
        } else {
            print(str_concat(cs8("Unknown directive: "), directive));
        }
    }
    
    s8 command = from_cstr(argv[1]);
    if (str_equal(command, cs8("init"))) {
        if (mkdir(cs8(".fvc"))) {
            print(cs8("WARNING: Failed to create .fvc folder"));
        }
        
        repo_header header = {.riff = REPO_RIFF};
        write_entire_file(cs8(".fvc/repo.fvc"), md8(xx &header, size_of header));
    } else if (str_equal(command, cs8("add"))) {
        repo_files files           = get_repo_files();
        u32        estimate        = argc - 2;
        u32        actually_added  = 0;
        u64       *hashes          = talloc((files.length + estimate) * sizeof *hashes);
        s8         strings         = zs8;
        
        // NOTE(Zorin): We can reduce number of concats by assuming names here are
        // alrealy null terminated, because they are knowing code of get_repo_files()
        for (u32 i = 0; i < files.length; ++i) {
            s8 str = files.names[i];
            ++str.len;
            strings = str_concat(strings, str);
            hashes[i] = files.hashes[i];
        }
        
        for (int i = 2; i < argc; ++i) {
            s8 path = from_cstr(argv[i]);
            print(str_concat(cs8("... adding "), path));
            
            for (u32 j = 0; j < files.length; ++j) {
                if (str_equal(files.names[j], path)) {
                    print(cs8("WARNING: File alrealy in the repo!"));
                    goto end1;
                }
            }
            
            u64 hash = get_file_hash(path);
            if (!hash) {
                print(cs8("WARNING: File does not exists!"));
                goto end1;
            }
            
            // Insert 0 at purpose so you could commit to it which means generating header
            hashes[actually_added + files.length] = 0;
            ++actually_added;
            
            ++path.len;
            strings = str_concat(strings, path);
            
            end1:
            unused 0;
        }
        
        u32         num_files = files.length + actually_added;
        repo_header header    = {.riff = REPO_RIFF, .num_files = num_files};
        file_t      file      = open_file(0, cs8(".fvc/repo.fvc"), FILE_TRUNCATE);
        
        imm p = 0, l = 0;
        write_file(file, p, (l = size_of header), &header);     p += l;
        write_file(file, p, (l = num_files << 3), hashes);      p += l;
        write_file(file, p, (l = strings.len),    strings.str); p += l;
        close_file(file);
    } else if (str_equal(command, cs8("remove"))) {
        repo_files files           = get_repo_files();
        u64       *hashes          = talloc(files.length * size_of *hashes);
        s8         strings         = zs8;
        u32        num_files       = 0;
        u32        tm              = argc - 2;
        bool      *founds          = talloc(tm * size_of *founds);
        
        for (u32 i = 0; i < files.length; ++i) {
            s8 str = files.names[i];
            
            for (int j = 2; j < argc; ++j) {
                if (str_equal(from_cstr(argv[j]), str)) {
                    print(str_concat(cs8("... removing "), str));
                    founds[j - 2] = 1;
                    goto end;
                }
            }
            
            ++str.len;
            strings = str_concat(strings, str);
            hashes[num_files++] = files.hashes[i];
            end:
            unused 0;
        }
        
        for (u32 i = 0; i < tm; ++i) {
            if (!founds[i]) {
                print(str_concat(cs8("WARNING: no file registered in repo: "),
                                 from_cstr(argv[i + 2])));
                
            }
        }
        
        repo_header header    = {.riff = REPO_RIFF, .num_files = num_files};
        file_t      file      = open_file(0, cs8(".fvc/repo.fvc"), FILE_TRUNCATE);
        
        imm p = 0, l = 0;
        write_file(file, p, (l = size_of header), &header);     p += l;
        write_file(file, p, (l = num_files << 3), hashes);      p += l;
        write_file(file, p, (l = strings.len),    strings.str); p += l;
        close_file(file);
    } else if (str_equal(command, cs8("commit"))) {
        repo_files files = get_repo_files();
        file_t repo_file = open_file(0, cs8(".fvc/repo.fvc"), FILE_WRITE);
        
        if (argc == 2) {
            print(cs8("No commit message!"));
            goto end2;
        }
        
        if (!files.length) {
            print(cs8("No files in the repo, check whatever you added files or created repo!"));
            goto end2;
        }
        
        s8 message = from_cstr(argv[2]);
        print(str_concat(cs8("Commit message: "), message));
        
        s8 timestamp = get_timestamp();
        print(timestamp);
        
        for (u32 i = 0; i < files.length; ++i) {
            s8 path = files.names[i];
            
            if (get_file_hash(path) != files.hashes[i]) {
                print(str_concat(cs8("... "), path));
                
                imm  num_lines;
                u8   data = read_entire_file(path);
                s8  *lines = split_lines(&num_lines, *cast(s8*) &data);
                
                s8 *info_header    = 0;
                s8 *history_header = 0;
                
                imm info_header_index = -1;
                imm history_header_index = -1;
                
                for (imm i = 0; i < num_lines; ++i) {
                    s8 line = lines[i];
                    
                    if (is_line_info_header(line)) {
                        info_header = lines + i;
                        info_header_index = i;
                    }
                    
                    if (is_line_history_header(line)) {
                        history_header = lines + i;
                        history_header_index = i;
                    }
                }
                
                imm p = 0;
                file_t file = open_file(0, path, FILE_TRUNCATE);
                if (!info_header) {
                    p = write_str(file, p, cs8("/****************************************"
                                               "**************************************.\n"
                                               "**\n"
                                               "** FILE        : "));
                    p = write_str(file, p, path);
                    p = write_str(file, p, cs8("\n** PROJECT     : "));
                    p = write_str(file, p, user_sys_project);
                    p = write_str(file, p, cs8("\n** COMPANY     : "));
                    p = write_str(file, p, user_sys_company);
                    p = write_str(file, p, cs8("\n** AUTHOR      : "));
                    p = write_str(file, p, user_sys_user);
                    p = write_str(file, p, cs8("\n** CREATED     : "));
                    p = write_str(file, p, timestamp);
                    p = write_str(file, p, cs8("\n** EDITED      : "));
                    p = write_str(file, p, timestamp);
                    p = write_str(file, p, cs8("\n** DESCRIPTION : "
                                               "\n** NOTICE      : "));
                    p = write_str(file, p, user_sys_copyright);
                    p = write_str(file, p, cs8("\n**\n"
                                               "*****************************************"
                                               "**************************************/\n"));
                    
                    // Insert empty line so it would look nice and stuff
                    if (!is_line_empty(lines[0])) {
                        p = write_str(file, p, cs8("\n"));
                    }
                }
                
                bool patch_info_header = 0;
                
                for (imm i = 0; i < num_lines; ++i) {
                    s8 line = lines[i];
                    
                    if (patch_info_header) {
                        if (is_line_closing_header(line)) {
                            patch_info_header = 0;
                        } else {
                            imm len = is_line_last_edited(line);
                            if (len > 0) {
                                p = write_str(file, p, ms8(line.str, len));
                                p = write_str(file, p, cs8(" "));
                                p = write_str(file, p, timestamp);
                                p = write_str(file, p, cs8("\n"));
                                continue;
                            }
                        }
                    }
                    
                    if (i == info_header_index) {
                        patch_info_header = 1;
                    }
                    
                    p = write_str(file, p, line);
                    
                    if (i < num_lines - 1) {
                        p = write_str(file, p, cs8("\n"));
                    }
                    
                    if (i == history_header_index) {
                        // We know what those lines are
                        i += 3;
                        
                        p = write_str(file, p, cs8("**\n** HISTORY:\n**\n** At "));
                        p = write_str(file, p, timestamp);
                        p = write_str(file, p, cs8(" by "));
                        p = write_str(file, p, user_sys_user);
                        p = write_str(file, p, cs8("\n**    - "));
                        p = write_str(file, p, message);
                        p = write_str(file, p, cs8("\n**\n"));
                    }
                }
                
                if (!history_header) {
                    // Insert empty line so it would look nice and stuff
                    if (!is_line_empty(lines[num_lines - 1])) {
                        p = write_str(file, p, cs8("\n"));
                    }
                    
                    p = write_str(file, p, cs8("\n/****************************************"
                                               "**************************************,\n"
                                               "**\n"
                                               "** HISTORY:\n**\n** At "));
                    p = write_str(file, p, timestamp);
                    p = write_str(file, p, cs8(" by "));
                    p = write_str(file, p, user_sys_user);
                    p = write_str(file, p, cs8("\n**    - "));
                    p = write_str(file, p, message);
                    p = write_str(file, p, cs8("\n**\n"
                                               "*****************************************"
                                               "**************************************/\n"));
                } else if (!is_line_empty(lines[num_lines - 1])) {
                    p = write_str(file, p, cs8("\n"));
                }
                
                close_file(file);
                
                // Patch the hash only at the end, because we updating header is also updates it
                u64 hash = get_file_hash(path);
                write_file(repo_file, size_of(repo_header) + (i << 3), 8, &hash);
            }
        }
        
        end2:
        close_file(repo_file);
    } else if (str_equal(command, cs8("config"))) {
        print(str_concat(cs8("USER      : "), user_sys_user));
        print(str_concat(cs8("PROJECT   : "), user_sys_project));
        print(str_concat(cs8("COMPANY   : "), user_sys_company));
        print(str_concat(cs8("COPYRIGHT : "), user_sys_copyright));
    } else if (str_equal(command, cs8("diff"))) {
        repo_files files = get_repo_files();
        bool       x     = 0;
        
        for (u32 i = 0; i < files.length; ++i) {
            s8 path = files.names[i];
            
            if (get_file_hash(path) != files.hashes[i]) {
                if (!x) {
                    print(cs8("Changes detected:"));
                    x = 1;
                }
                
                print(str_concat(cs8("... "), path));
            }
        }
        
        if (!x) {
            print(cs8("No changed detected!"));
        }
    } else if (str_equal(command, cs8("fix"))) {
        repo_files files = get_repo_files();
        
        for (u32 i = 0; i < files.length; ++i) {
            files.hashes[i] = get_file_hash(files.names[i]);
        }
        
        repo_header header = {.riff = REPO_RIFF, .num_files = files.length};
        file_t      file   = open_file(0, cs8(".fvc/repo.fvc"), FILE_WRITE);
        
        write_file(file, 0, size_of header, &header);
        write_file(file, size_of header, files.length << 3, files.hashes);
        close_file(file);
    } else {
        print(str_concat(cs8("Unknown command: "), command));
    }
    
    return 0;
}

/******************************************************************************,
**
** HISTORY:
**
** At D10/15/2026T11:14:16-UTC by Lsalwestfox
**    - Added banner in help command
**
** At D10/15/2026T10:51:04-UTC by Lsalwestfox
**    - Fixed and improved handling of user.sys
**
** At D10/15/2026T10:37:38-UTC by Lsalwestfox
**    - Initial commit
**
*******************************************************************************/
