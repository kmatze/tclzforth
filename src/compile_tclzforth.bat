@echo off
cls

REM *********************************
REM
REM TCL/TK mit MinGW
REM

set TCLVER=83
set FILE=tclzforth
set TCL=\win-apps\Tclscripts\MinGW\TCL%TCLVER
set TCLLIBS=-ltclstub%TCLVER
rem set WINLIBS=-lwinmm
set FLAGS=-shared -DUSE_TCL_STUBS -O3
set EXT=dll

echo *
echo * compile %FILE with tcl%TCLVER
echo *
gcc -c zforth.c
gcc -o %FILE.%EXT %FILE.c zforth.o %FLAGS -I%TCL\include -B%TCL\lib %TCLLIBS %WINLIBS
strip %FILE.%EXT
\win-apps\tools\upx391.exe --ultra-brute %FILE.%EXT