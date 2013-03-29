@echo off

echo Assembling code...
..\Debug\DAssembler.exe code.txt
move code.dexe ../DCPU-16/code.dexe

echo Assembly successful!
pause