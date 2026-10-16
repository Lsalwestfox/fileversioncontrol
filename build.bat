@echo off
if not exist build md build
clang -Wall -nostdlib -Oz -mno-stack-arg-probe -fno-stack-protector -Wall -Wextra -std=c99 -xc -lkernel32 -lshell32 fvc_win.c -o build\fvc.exe -Wl,-entry:_start
