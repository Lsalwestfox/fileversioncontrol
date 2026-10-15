# File Version Control

I saw headers in files that are in the old 90's projects and I really liked them, the problem is that you need to edit them manually, which I didn't like
so I wrote tool for myself. It works for single-header libraries best IMO because it's just 1 file and there's history and header anyway.

It also writes only to files where changes occured and tracks where they changed based on last edited time, because I found in practice manually typing each file where you edited in is lame.

Because of how code works, it converts CRLF to LF and inserts empty line at
the end of the file, you have freedom of moving headers or adding new values
or removing even some of them, it changes only EDITED field.

This executable is written in C99 for little-endian machines only.
Windows build does not require CRT or stdlib and also fast ony my machine,
which is fine because I wrote tool for myself but should be relativly fast I guess?

## Usage

First type fvc. This will print something like that:

```cmd
Usage: fvc [command] [flags]
   init              - initializes the repo
   add [files...]    - adds files to the repo to check
   remove [files...] - remove files from the repo to check
   commit [message]  - commit message to the repo
   config            - view repo's config
   diff              - show files that commit will change
   fix               - registers file changes without commiting
```

`fvc init` creates empty repo, this done by creating .fvc folder and repo.fvc inside of this folder, in repo.fvc all files were stored and their 64-bit hashes, you can see what files opened by just opening it in notepad and going to the end of the file.

The .fvc folder should not be part of the repo i.e. must be added to the .gitignore, it's local per-user.

`fvc add` adds files to the repo, it does not insert their hashes in the repo
so if you type `fvc diff` after `fvc add` it'll say that recent added files
were changes, this is because 99% of cases when you add file it does not have
header so you add header to it by just commiting.

Example usage: `fvc add fvc.c fvc_dos.c fvc_win.c fvc_linux.c`

`fvc remove` removes files from the repo, when you delete some source file
you should remove this file from the repo, so this is command for that.

Example usage: `fvc remove fvc_dos.c`

`fvc commit` commits message, updating file headers and writing new hashes
to the repo indicating the changes has been set.

CORRECT usage: `fvc commit "Commit message"`

WRONG usage: `fvc commit Commit message`

`fvc config` shows stuff about things like user and project name and stuff and it looks something like that:

```cmd
D:\Projects\FVC>fvc config
USER      : Lsalwestfox
PROJECT   : File Version Control (FVC)
COMPANY   :
COPYRIGHT : Copyright (C) 2026 Lsalwestfox
```

The file is .fvc\user.sys, that you can open with any text editor, here's
example user.sys:

```properties
user=MyName (myemail@example.com)
project=Not my project
company=Company name
copyright=Public domain or MIT Copyright (C) 2036 Company name
```

Default user name is Anon, which is short for "Anonymous"

`fvc diff` shows in which files the changes were made, those files will be affected by commit command and fix command.

`fvc fix` is like shadow commit, it does not update any files headers or even
reads them, the only thing it does is updates file hashes just like commit to
indicate they are updated. This is useful because imagine situation where you
want to edit description or change commit message because you made a type, the
solution is to go through all files that `fvc commit` listed for this exact reason beyond debugging purposes and just edit there, but the problem is that
when you edit last edited is changed so those files are changed, so after patching instead of commiting nonesense you just type `fvc fix`

## Notes

You are not required to call `fvc init`, you can just straigh up type `fvc add`
and when adding files it'll create the repo, just ensure .fvc folder is present. The reason this happens is because code made in such way it never fails and the way it gets repo files is that if it's failed to do it (corrupted or not present) it'll return empty repo.

.fvc\user.sys follows DOS-syntax this means this all mean the same:

```properties
user=Example
user=   Example
user Example
user    Example
UsEr=Example
UsEr=   Example
UsEr Example
UsEr    Example
USER=Example
USER=   Example
USER Example
USER    Example
```

and comments:

```properties
; Example comment
rem Other type of comment
```

And the REM directive does nothing as a comment

## Building

clang required and it works only on windows for now.

Run `build.bat`
