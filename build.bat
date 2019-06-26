@echo off
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

rem includes
set includes= /I ../Includes/boost_1_70_0

rem asm, optional (disabled by default, to enable erase the prefix rem at the beginning of the next line)
rem set asm= /FAcsu /Faasm\

set compilerflags=%asm% /Ox /Zi /EHsc /std:c++latest %defines%
set sources= src/*.cpp
set linkerflags=/OUT:bin\main.exe
cl.exe %compilerflags% %sources% %includes% /link %linkerflags%
del bin\*.ilk *.obj *.pdb