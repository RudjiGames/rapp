@echo off
set ACTION=vs2022
IF "%1"=="emscripten" set ACTION=--gcc=asmjs gmake
cd ..\genie
..\..\build\tools\bin\windows\genie.exe %ACTION%
cd ..\scripts
