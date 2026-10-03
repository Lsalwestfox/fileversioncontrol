@echo off
if not exist BUILD md BUILD
clang -nostdlib -Oz -Wall -Wextra -Wdeclaration-after-statement -Wcomment -std=c89 -xc FVCNT.C -lKERNEL32 -o BUILD/FVC.EXE -Wl,-entry:_start

