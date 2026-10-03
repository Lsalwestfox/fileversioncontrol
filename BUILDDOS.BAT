@echo off
if not exist BUILD md BUILD
set _32=BUILD\FVC
wcl386 /l=dos4g /mf /fe=%_32%DOS.ERR /fo=%_32%DOS.OBJ /fe=%_32%.EXE FVCDOS.C
SB /b /o BUILD\FVC.EXE


