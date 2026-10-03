# File version control

History is that I wanted one day old-style header that you see on old codebases, and I'm lazy to editing
them myself, so I wrote tool that I can treat like a git, and it also good for single-header libraries,
the ones that put history at the bottom and header at the top.

Also this tool converts CRLF to LF and makes sure file has empty line as last line (if that makes sense)
There 2 headers, header for info and header for history, both can be rearanged once generated.

Type fvc for more

The info about user (like name) can be done in FVC.SYS which has format that you expect

Example config (used for this project):
```
USER=Lsalwestfox
NOTICE=Copyright (C) 2026 Lsalwestfox
PROJECT=File Version Control (FVC)
```

Also there's COMPANY and EMAIL


## Building

BUILD.BAT or BUILDDOS.BAT if you want dos version

Windows version required clang and is CRT-less
DOS version is for MS-DOS 6.2 and requires WATCOM 10.x and CRT
