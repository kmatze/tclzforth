@echo off
cls

set BIN1=zforth.exe
set SRC1=main.c zforth.c

set BIN2=zftiny.exe
set SRC2=main_tiny.c zforth.c

set CC=tcc.exe
set CFLAGS= -DZF_ENABLE_TRACE

set PCK1=strip.exe
set PCK2=\win-apps\tools\upx393.exe --ultra-brute

rem all:
%CC -o %BIN1 %CFLAGS %SRC1
%PCK1 %BIN1
%PCK2 %BIN1

rem tiny:
%CC -o %BIN2 %CFLAGS %SRC2
%PCK1 %BIN2
%PCK2 %BIN2


