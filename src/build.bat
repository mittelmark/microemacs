@echo off
rem JASSPA MicroEmacs - www.jasspa.com
rem build - JASSPA MicroEmacs build script for MS windows and dos platforms
rem Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
rem See the file main.c for copying and conditions.
set OPTIONS=
set LOGFILE=
set LOGFILEA=
set MAINTYPE=me
set MEDEBUG=
set METYPE=
set MAKEFILE=
:build_option
if "%1." == "."    goto build_cont
if "%1" == "-C"    set  OPTIONS=clean
if "%1" == "-d"    set  MEDEBUG=d
if "%1" == "-h"    goto build_help
if "%1" == "-l"    goto build_logf
if "%1" == "-la"   goto build_logfa
if "%1" == "-m"    goto build_mkfl
if "%1" == "-ne"   set  MAINTYPE=ne
if "%1" == "-S"    set  OPTIONS=spotless
if "%1" == "-t"    goto build_type
if "%1" == "-u"    echo Option -u no longer required - all versons have URL support
shift
goto build_option

:build_logf
shift
set LOGFILE=%1
shift
goto build_option

:build_logfa
shift
set LOGFILEA=%1
shift
goto build_option

:build_mkfl
shift
set MAKEFILE=%1
shift
goto build_option

:build_type
shift
set METYPE=%1
shift
goto build_option

:build_cont

if "%OPTIONS%." == "." set OPTIONS=%MAINTYPE%%MEDEBUG%%METYPE%

set MAKE=nmake
if "%MAKEFILE%." == "." goto build_auto
if "%MAKEFILE%" == "dosdj1.mak"  set MAKE=make
if "%MAKEFILE%" == "dosdj2.mak"  set MAKE=make
if "%MAKEFILE%" == "mingw32.gmk" set MAKE=mingw32-make

goto build

:build_auto

if "%DJGPP%." == "." goto build_win32

:build_dos

set MAKE=make
set MAKEFILE=dosdj1.mak

goto build

:build_win32

rem test for 4dos - no 4dos == no smarts
if NOT "%@LOWER[A]" == "a" goto build_msvc5

rem try to auto detect the build tools available
set lpath=%@LOWER[%PATH%]
set borland=%@INDEX[%lpath%,borland]
set msvc=%@INDEX[%lpath%,\vc]
if %borland% == -1 goto build_msvc
if %msvc% == -1 goto build_borland

if %borland% GT %msvc% goto build_msvc

:build_borland

set MAKE=make

if %@INDEX[%lpath%,\bcc55] == -1 goto build_borland_bc

set MAKEFILE=win32b55.mak
goto build

:build_borland_bc

set MAKEFILE=win32bc.mak
goto build

:build_msvc

set MAKE=nmake
if %@INDEX[%lpath%,\vc98] == %msvc% goto build_msvc6

:build_msvc5

set MAKEFILE=win32v5.mak
goto build

:build_msvc6

set MAKEFILE=win32v6.mak

:build

if "%LOGFILE%." == "." goto build_applog

echo %MAKE% -f %MAKEFILE% %OPTIONS% > %LOGFILE% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% > %LOGFILE% 2>&1

goto build_exit

:build_applog

if "%LOGFILEA%." == "." goto build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS% >> %LOGFILEA% 2>&1
%MAKE% -f %MAKEFILE% %OPTIONS% >> %LOGFILEA% 2>&1

goto build_exit

:build_nolog

echo %MAKE% -f %MAKEFILE% %OPTIONS%
%MAKE% -f %MAKEFILE% %OPTIONS%

goto build_exit

:build_help

echo Usage: build [options]
echo .
echo Where options can be:-
echo     -C   : Build clean.
echo     -d   : For debug build (output is med* or ned*)
echo     -h   : For this help page
echo     -l {logfile}
echo          : Set the compile log file
echo     -la {logfile}
echo          : Append the compile log to the given file
echo     -m {makefile}
echo            Sets the makefile to use where {makefile} can be:-
echo              dosdj1.mak   Dos build using djgpp version 1 
echo              dosdj2.mak   Dos build using djgpp version 2 
echo              mingw32.gmk  Win32 build using MinGW GNU GCC
echo              win32bc.mak  Win32 build using Borland C
echo              win32v2.mak  Win32 build using MS VC version 2
echo              win32v5.mak  Win32 build using MS VC version 5
echo              win32v6.mak  Win32 build using MS VC version 6 (or 98)
echo              win32sv2.mak Win32s build (for Win 3.xx) using MS VC version 2
echo              win32sv4.mak Win32s build (for Win 3.xx) using MS VC version 4
echo     -ne  : for NanoEmacs build (output is ne).
echo     -S   : Build clean spotless.
echo     -t {type}
echo          : Sets build type:
echo               c  Console support only
echo               w  Wondow support only (default)
echo               cw Console and window support

:build_exit

