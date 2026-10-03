/*****************************************************************************.
 *
 * FILE        : FVC.C
 * PROJECT     : File Version Control (FVC)
 * COMPANY     : 
 * AUTHOR      : Lsalwestfox
 * CREATED     : D09/26/2026T15:09:09-UTC
 * EDITED      : D09/28/2026T08:18:45-UTC
 * DESCRIPTION : Platform-indepedent portion of the tool
 * NOTICE      : Copyright (C) 2026 Lsalwestfox
 *
 *****************************************************************************/

#ifndef __cplusplus
typedef int bool;
#endif

#if defined _WIN32 || defined _WIN64
typedef unsigned long long ullong;
#else
typedef unsigned long ullong;
#endif

typedef void               *writer;

typedef struct string8 {
    char   *str;
    ullong  size;
} string8_t;

string8_t make_string8(char *data, ullong size)
{
   string8_t out = {0};
   out.str  = data;
   out.size = size;
   return(out);
}

#define STR_STATIC(x) make_string8((char*)(x), sizeof(x) - 1)
#define STR(x, l) make_string8((char*)(x), (l))
#define NULLSTR make_string8(0, 0)

/* Idealy should support UTF-8, but everyone uses only ASCII for programming anyway */
void      *talloc(ullong amount);
string8_t  read_entire_file(const char *path);
void       cprint(const char *str);
void       print(string8_t string);
int        get_errno();

writer open_writer(const char *path);
void   cwrite_to(writer write, const char *str);
void   write_to(writer write, string8_t content);

/* Must be "DMM/dd/YYYYThh:mm:ss-TIMEZONE" format, UTC timezone preferred */
string8_t timestamp();

bool directive_equals(string8_t str, string8_t directive)
{
    ullong i = 0;

    if (str.size != directive.size)
        return(0);

    for (i = 0; i < str.size; i++)
    {
        char a = str.str[i];
        char b = directive.str[i];

        if (a >= 'a' && a <= 'z') a -= 0x20;
        if (b >= 'a' && b <= 'z') b -= 0x20;
        if (a != b) return(0);
    }

    return(1);
}

string8_t trim(string8_t str)
{
    while (str.size && (str.str[0] == ' ' || str.str[0] == '\t'))
    {
        str.size--;
        str.str++;
    }

    while (str.size && (str.str[str.size - 1] == ' ' || str.str[str.size - 1] == '\t'))
    {
        str.size--;
    }

    return(str);
}

string8_t *get_lines(string8_t string)
{
    string8_t *out;
    string8_t  str;
    ullong     i;
    ullong     j;
    ullong     k;

    for (i = 0, j = 1; i < string.size; i++)
    {
        if (string.str[i] == '\n')
            j++;
    }

    out = (string8_t*)talloc((j + 1) * sizeof(*out));
    for (i = j = k = 0; i < string.size; i++)
    {
        if (string.str[i] == '\n')
        {
            str = STR(string.str + k, i - k);
            if (i && string.str[i - 1] == '\r')
                str.size--;
            out[j++] = str;
            k = i + 1;
        }
    }

    if (i != k)
    {
    str = STR(string.str + k, i - k);
    if (i && string.str[i - 1] == '\r')
        str.size--;
    out[j++] = str;
    }

    out[j++] = NULLSTR;
    return(out);
}

void print_help_page()
{
    print(STR_STATIC("Usage: fvc [flags]\n"
                     "Flags:\n\t"
                     "/f - provide files, each file being 1 argument\n\t"
                     "/m - commit to the history with message\n\t"
                     "/h - prints this message\n\t"
                     "/c - override company\n\t"
                     "/n - override notice\n\t"
                     "/p - override project\n\t"
                     "/e - override email\n\t"
                     "/u - override user\n\n"
                     "Examples:\n\t"
                     "fvc /h\n\t"
                     "fvc /f file1.c \"path with spaces/file2.c\" file3.c /m \"My commit message\"\n\t"
                     "fvc /f file.c /m My\\ commit\\ message\n"));
}

bool is_valid_last_edited_field(string8_t str, ullong *start, bool *do_extra_space)
{
    ullong i;

    if (str.str[0] != ' ' ||
        str.str[1] != '*' ||
        str.str[2] != ' ')
        return(0);

    i = 3;
    while (str.str[i] == ' ' && i < str.size)
        i++;

    if (str.str[i + 0] != 'E' ||
        str.str[i + 1] != 'D' ||
        str.str[i + 2] != 'I' ||
        str.str[i + 3] != 'T' ||
        str.str[i + 4] != 'E' ||
        str.str[i + 5] != 'D')
        return(0);

    i += 6;
    while (str.str[i] == ' ' && i < str.size)
        i++;

    if (str.str[i] != ':')
        return(0);

    i++;
    if (i >= str.size || str.str[i] != ' ')
    {
        if (do_extra_space)
            *do_extra_space = 1;
    }
    else
    {
        i++;
        if (do_extra_space)
            *do_extra_space = 0;
    }

    if (start)
        *start = i;

    return(1);
}

bool is_primal_header(string8_t str)
{
    ullong i;

    if (str.size != 79 || str.str[0] != '/' || str.str[78] != '.')
        return(0);

    for (i = 1; i < 78; i++)
    {
        if (str.str[i] != '*')
            return(0);
    }

    return(1);
}

bool is_history_header(string8_t str)
{
    ullong i;

    if (str.size != 79 || str.str[0] != '/' || str.str[78] != ',')
        return(0);

    for (i = 1; i < 78; i++)
    {
        if (str.str[i] != '*')
            return(0);
    }

    return(1);
}

bool is_closing_header(string8_t str)
{
    ullong i;

    if (str.size != 79 || str.str[0] != ' ' || str.str[78] != '/')
        return(0);

    for (i = 1; i < 78; i++)
    {
        if (str.str[i] != '*')
            return(0);
    }

    return(1);
}

void write_user(writer write, string8_t user, string8_t email)
{
            write_to(write, user);

            if (email.size > 0)
            {
                write_to(write, STR_STATIC(" ("));
                write_to(write, email);
                write_to(write, STR_STATIC(")"));
            }
}

int main(int argc, char **argv)
{
    string8_t    user      = NULLSTR;
    string8_t    notice    = NULLSTR;
    string8_t    project   = NULLSTR;
    string8_t    company   = NULLSTR;
    string8_t    email     = NULLSTR;
    string8_t    message   = NULLSTR;
    string8_t    last_line = NULLSTR;
    string8_t   *lines     = 0;
    char       **files     = 0;
    int          num_files = 0;
    bool         do_file   = 0;
    bool         do_commit = 0;
    int          i;

    if (argc <= 1)
    {
        print_help_page();
        return(0);
    }

    for (i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        if (arg[0] == '/')
        {
            do_file = 0;

            switch (arg[1])
            {
                case 'h':
                {
                    print_help_page();
                    return(0);
                } break;
        
                case 'f':
                {
                    files     = argv + i + 1;
                    num_files = 0;
                    do_file   = 1;
                } break;

                case 'm':
                {
                    char *param = message.str = argv[i + 1];
                    do_commit = 1;
                    for (message.size = 0; *param++; message.size++);
                } break;

                case 'u':
                {
                    char *param = user.str = argv[i + 1];
                    for (user.size = 0; *param++; user.size++);
                } break;

                case 'c':
                {
                    char *param = company.str = argv[i + 1];
                    for (company.size = 0; *param++; company.size++);
                } break;

                case 'n':
                {
                    char *param = notice.str = argv[i + 1];
                    for (notice.size = 0; *param++; notice.size++);
                } break;

                case 'p':
                {
                    char *param = project.str = argv[i + 1];
                    for (project.size = 0; *param++; project.size++);
                } break;

                case 'e':
                {
                    char *param = email.str = argv[i + 1];
                    for (email.size = 0; *param++; email.size++);
                } break;
            }
        }
        else if (do_file)
        {
            num_files++;
        }
    }

    lines = get_lines(read_entire_file("FVC.SYS"));
    while (lines->str)
    {
        string8_t key   = *lines++;
        string8_t value = NULLSTR;
        ullong    i;

        if (key.str[0] == ';')
            continue;

        for (i = 0; i < key.size; i++)
        {
            char ch = key.str[i];
            if (ch == '=' || ch == '\t' || ch == ' ')
            {
                value.str  = key.str  + i + 1;
                value.size = key.size - i - 1;
                key.size   = i;
                break;
            }
        }

        key   = trim(key);
        value = trim(value);

        if (key.size == 0 || directive_equals(key, STR_STATIC("REM")))
        {
            continue;
        }
        else if (directive_equals(key, STR_STATIC("USER")))
        {
            if (!user.str)
                user = value;
        }
        else if (directive_equals(key, STR_STATIC("COMPANY")))
        {
            if (!company.str)
                company = value;
        }
        else if (directive_equals(key, STR_STATIC("PROJECT")))
        {
            if (!project.str)
                project = value;
        }
        else if (directive_equals(key, STR_STATIC("NOTICE")))
        {
            if (!notice.str)
                notice = value;
        }
        else if (directive_equals(key, STR_STATIC("EMAIL")))
        {
            if (!email.str)
                email = value;
        }
        else
        {
            print(STR_STATIC("Unknown directive in FVC.SYS: "));
            print(key);
            print(STR_STATIC("\n"));
        }
    }

    if (!user.size)
    {
        user = STR_STATIC("<Unknown>");
    }

    if (do_commit)
    {
    string8_t time = timestamp();
    print(STR_STATIC("Commit message: "));
    print(message);
    print(STR_STATIC("\nTimestamp: "));
    print(time);
    print(STR_STATIC("\n"));
    for (i = 0; i < num_files; i++)
    {
        char      *file      = files[i];
        string8_t  content   = read_entire_file(file);
        string8_t *lines     = get_lines(content);
        writer     write;

        if (get_errno())
        {
            print(STR_STATIC("File not found: "));
            cprint(file);
            print(STR_STATIC("\n"));
            continue;
        }

        write = open_writer(file);
        if (!write)
        {
            print(STR_STATIC("Failed to open file for write: "));
            cprint(file);
            print(STR_STATIC("\n"));
            continue;
        }

        if (is_primal_header(*lines))
        {
            write_to(write, *lines++);
            write_to(write, STR_STATIC("\n"));
            while (lines->str && !is_closing_header(*lines))
            {
                ullong start;
                bool   do_space;
                if (is_valid_last_edited_field(*lines, &start, &do_space))
                {
                    write_to(write, STR(lines->str, start));
                    if (do_space)
                    {
                        write_to(write, STR_STATIC(" "));
                    }
                    write_to(write, time);
                    write_to(write, STR_STATIC("\n"));
                    lines++;
                }
                else
                {
                    write_to(write, *lines++);
                    write_to(write, STR_STATIC("\n"));
                }
            }
        }
        else
        {
            write_to(write, STR_STATIC("/*****************************************************************************.\n"
                                       " *\n"
                                       " * FILE        : "));
            cwrite_to(write, file);

            write_to(write, STR_STATIC("\n * PROJECT     : "));
            write_to(write, project);
            
            write_to(write, STR_STATIC("\n * COMPANY     : "));
            write_to(write, company);

            write_to(write, STR_STATIC("\n * AUTHOR      : "));
            write_user(write, user, email);
            
            write_to(write, STR_STATIC("\n * CREATED     : "));
            write_to(write, time);
            
            write_to(write, STR_STATIC("\n * EDITED      : "));
            write_to(write, time);

            write_to(write, STR_STATIC("\n * DESCRIPTION : "
                                       "\n * NOTICE      : "));

            write_to(write, notice);
            write_to(write, STR_STATIC("\n *\n"
                                       " *****************************************************************************/\n\n"));
        }

        last_line = NULLSTR;
        while (lines->str && !is_history_header(*lines)) {
            last_line = *lines++;
            write_to(write, last_line);
            write_to(write, STR_STATIC("\n"));
        }

        last_line = trim(last_line);
        if (!lines->str && last_line.size)
            write_to(write, STR_STATIC("\n"));

        if (is_history_header(*lines)) {
            write_to(write, *lines++);
            write_to(write, STR_STATIC("\n *\n * HISTORY:\n *\n * ["));
            write_to(write, time);
            write_to(write, STR_STATIC("] "));
            write_to(write, message);
            write_to(write, STR_STATIC("\n *    - "));
            write_user(write, user, email);
            write_to(write, STR_STATIC("\n"));
            lines += 2;
        } else {
            write_to(write, STR_STATIC("/*****************************************************************************,\n"
                                       " *\n"
                                       " * HISTORY:\n"
                                       " *\n"
                                       " * ["));
            write_to(write, time);
            write_to(write, STR_STATIC("] "));
            write_to(write, message);
            write_to(write, STR_STATIC("\n *    - "));
            write_user(write, user, email);
            write_to(write, STR_STATIC("\n *\n"
                                       " *****************************************************************************/\n\n"));
        }

        last_line = NULLSTR;
        while (lines->str) {
            last_line = *lines++;
            write_to(write, last_line);
            write_to(write, STR_STATIC("\n"));
        }
        
        last_line = trim(last_line);
        if (!lines->str && last_line.size)
            write_to(write, STR_STATIC("\n"));
    }
    }

    print(STR_STATIC("Done\n"));
    return(0);
}

/*****************************************************************************,
 *
 * HISTORY:
 *
 * [D09/28/2026T08:18:45-UTC] Added DOS and C89 support
 *    - Lsalwestfox
 *
 * [D09/26/2026T15:09:09-UTC] Initial commit
 *    - Lsalwestfox
 *
 *****************************************************************************/

