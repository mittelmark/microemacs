# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# winmingwgcc.mak - Make file for native Windows build using MinGW/MSYS2.
#
# This makefile supports three build distributions controlled by BDIST:
#   BDIST=msys2      - MSYS2 executables (default), requires msys-2.0.dll
#   BDIST=msys2unix  - MSYS2 Unix-like build for mintty, uses ncurses
#   BDIST=mingw64    - Native Windows executables using MinGW-w64, static zlib
#
# For cross-compilation from Linux, see linuxmingwgcc.mak
#
# Copyright (C) 2007-2009 JASSPA (www.jasspa.com)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################
#
# Created:     Sat Jan 24 1998
# Synopsis:    Make file for native Windows/MSYS2 build.
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
#     To build from the command line using make & makefile. 
#
#	MSYS2 builds (default, requires msys-2.0.dll at runtime):
#	  make -f winmingwgcc.mak                              release GUI build
#	  make -f winmingwgcc.mak BCFG=debug                   debug GUI build
#	  make -f winmingwgcc.mak BTYP=c                       release console build
#	  make -f winmingwgcc.mak BCOR=ne                      release ne build
#
#	MSYS2 Unix-like builds (for mintty terminal, uses ncurses):
#	  make -f winmingwgcc.mak BDIST=msys2unix              release console build
#	  make -f winmingwgcc.mak BDIST=msys2unix BTYP=c       release console build
#	  make -f winmingwgcc.mak BDIST=msys2unix BTYP=w       release GUI build (Windows libs)
#
#	Native Windows builds (BDIST=mingw64, no runtime DLLs needed):
#	  make -f winmingwgcc.mak BDIST=mingw64                release GUI build
#	  make -f winmingwgcc.mak BDIST=mingw64 BCFG=debug     debug GUI build
#	  make -f winmingwgcc.mak BDIST=mingw64 BTYP=c         release console build
#
#	  make -f winmingwgcc.mak clean                        to clean source directory
#	  make -f winmingwgcc.mak spotless                     to clean source directory even more
#
#	Output directories:
#	  MSYS2 builds:       ./.msys64gcc-{release,debug}-me{c,w}/
#	  MSYS2 Unix builds:  ./.msys64unix-{release,debug}-mec/ (console) or -mew/ (GUI)
#	  Windows builds:     ./.mingw64gcc-{release,debug}-me{c,w}/
#
##############################################################################
#
# Installation Directory
INSTDIR	      = /c/emacs
INSTPROGFLAGS = 
#
# Local Definitions
A        = .a
EXE      = .exe

# BDIST controls build distribution:
#   msys2      - MSYS2 executables (default), requires msys-2.0.dll at runtime
#   msys2unix  - MSYS2 Unix-like build for mintty, uses ncurses
#   mingw64    - Native Windows executables, static zlib, standalone
BDIST    ?= msys2

# Auto-detect MSYSTEM and set OUTTAG + PKGPFX accordingly
# OUTTAG controls output directory name: .{env}{subsystem}-release-{type}
# PKGPFX is package prefix for pacman
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
PKGPFX   = mingw-w64-ucrt-x86_64
else ifeq "$(MSYSTEM)" "MINGW64"
PKGPFX   = mingw-w64-x86_64
else ifeq "$(MSYSTEM)" "MINGW32"
PKGPFX   = mingw-w64-i686
else
PKGPFX   = gcc
endif
else
# MSYSTEM not set (non-MSYS2 build)
endif

# Set OUTTAG based on BDIST and MSYSTEM
# Format: .{env}{subsystem}-release-{type}
#   msys subsystem:     needs msys-2.0.dll, dynamic zlib
#   msys2unix subsystem: Unix-like mintty build, uses ncurses
#   win subsystem:      standalone, static zlib
ifeq "$(BDIST)" "mingw64"
# Force native Windows build (static zlib)
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
OUTTAG   = ucrt64win
else ifeq "$(MSYSTEM)" "MINGW64"
OUTTAG   = mingw64win
else ifeq "$(MSYSTEM)" "MINGW32"
OUTTAG   = mingw32win
else
OUTTAG   = msys64win
endif
else
OUTTAG   = msys64win
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -Wl,-Bstatic -lz -Wl,-Bdynamic
else ifeq "$(BDIST)" "msys2unix"
# Unix-like build for mintty (uses ncurses, like Cygwin)
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
OUTTAG   = ucrt64unix
else ifeq "$(MSYSTEM)" "MINGW64"
OUTTAG   = mingw64unix
else ifeq "$(MSYSTEM)" "MINGW32"
OUTTAG   = mingw32unix
else
OUTTAG   = msys64unix
endif
else
OUTTAG   = msys64unix
endif
LDLIBSB  = -lncurses -lz
else
# Default MSYS2 build (dynamic zlib)
ifdef MSYSTEM
ifeq "$(MSYSTEM)" "UCRT64"
OUTTAG   = ucrt64msys
else ifeq "$(MSYSTEM)" "MINGW64"
OUTTAG   = mingw64msys
else ifeq "$(MSYSTEM)" "MINGW32"
OUTTAG   = mingw32msys
else
OUTTAG   = msys64msys
endif
else
OUTTAG   = msys64msys
endif
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lz
endif
CC       = gcc
RC       = windres
STRIP    = strip
AR       = ar

MK       = make
LD       = $(CC)
RM       = rm -f
RMDIR    = rm -r -f

TOOLKIT  = winmingwgcc
ifeq "$(BPRF)" "1"
BUILDID  = $(OUTTAG)p
else
BUILDID  = $(OUTTAG)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug

# Set compiler defines based on BDIST
# msys2unix console uses Cygwin-like defines for Unix terminal support
# msys2unix GUI uses Windows defines like mingw64 build
# msys2/mingw64 use Windows defines
CCDEFS   = -D_WIN32 -D_MINGW -Wall -I../bfs/win32/include
ifneq "$(BDIST)" "msys2unix"
else ifneq "$(BTYP)" "c"
# msys2unix GUI: override to Windows-style defines
CCDEFS   = -D_WIN32 -D_MINGW -Wall -I../bfs/win32/include
else
# msys2unix console: use Cygwin-like defines
CCDEFS   = -D_CYGWIN -D_USE_NCURSES -D_USETPARM -Wall -I.
endif
CCFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g -D_DEBUG
LDDEFS   = 
LDFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops
LDFLAGSD = -g

# When building with mingw64 from MSYS2, the native Windows gcc cannot resolve
# MSYS2 paths in TEMP/TMP environment variables. Override them with proper
# Windows paths so cc1.exe and the assembler can create temp files.
# Also ensure /mingw64/bin is on PATH so cc1.exe and as.exe can find their
# dependent DLLs (libisl, libmpc, libmpfr).
ifeq "$(BDIST)" "mingw64"
WINTEMP  := $(shell cygpath -w /tmp 2>/dev/null || echo C:\\msys64\\tmp)
export TEMP  = $(WINTEMP)
export TMP   = $(WINTEMP)
export PATH := /mingw64/bin:$(PATH)
endif

ARFLAGSR = rcs
ARFLAGSD = rcs
RCFLAGS  = --input-format rc --output-format coff -D_WIN32 -D_MINGW

ifeq "$(BCFG)" "debug"
BOUTDIR  = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
STRIP    = - echo No strip - debug 
else
BOUTDIR  = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
endif

ifeq "$(BCOR)" "ne"
BCOR_CDF = -D_NANOEMACS
PRGLIBS  = 
LDLIBS   = $(LDLIBSB)
else
BCOR     = me
BCOR_CDF = -D_SOCKET
LDLIBS   = -lws2_32 -lmpr $(LDLIBSB)
endif

ifeq "$(BPRF)" "1"
CCPROF = -D_ME_PROFILE -pg -no-pie
LDPROF = -pg -no-pie
STRIP  = - echo No strip - profile 
else
CCPROF = 
LDPROF = 
endif

ifeq "$(BTYP)" "cw"
BTYP_CDF = -D_ME_CONSOLE -D_CONSOLE -D_ME_WINDOW
BTYP_LDF = -Wl,-subsystem,console
else ifeq "$(BTYP)" "c"
BTYP_CDF = -D_ME_CONSOLE -D_CONSOLE
BTYP_LDF = -Wl,-subsystem,console
else
BTYP_CDF = -D_ME_WINDOW
BTYP_LDF = -Wl,-subsystem,windows
BTYP     = w
endif

# msys2unix GUI builds need Windows libraries (like mingw64), not ncurses
ifneq "$(BDIST)-$(BTYP)" "msys2unix-c"
ifeq "$(BDIST)" "msys2unix"
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lz
endif
endif

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)32
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def bfs.h \
	   $(TOOLKIT).mak

# Source objects differ based on BDIST and BTYP:
#   msys2unix console: Uses unixterm.o (Unix-like terminal, like Cygwin)
#   msys2unix GUI:     Uses winterm.o + winprint.o (Windows terminal)
#   msys2/mingw64:     Uses winterm.o + winprint.o (Windows terminal)
ifneq "$(BDIST)" "msys2unix"
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bfs.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/spawn.o \
	   $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/winterm.o $(OUTDIR)/winprint.o $(OUTDIR)/$(BCOR).coff \
	   $(OUTDIR)/win32dir.o $(OUTDIR)/access.o
else ifeq "$(BTYP)" "c"
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bfs.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/spawn.o \
	   $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/unixterm.o $(OUTDIR)/$(BCOR).coff
else
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bfs.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/spawn.o \
	   $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/winterm.o $(OUTDIR)/winprint.o $(OUTDIR)/$(BCOR).coff \
	   $(OUTDIR)/win32dir.o $(OUTDIR)/access.o
endif
#
# Rules
.SUFFIXES: .c .o .rc .coff

all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<

# win32dir.o and access.o are only needed for Windows GUI builds (not msys2unix console)
ifneq "$(BDIST)-$(BTYP)" "msys2unix-c"
$(OUTDIR)/win32dir.o : $(OUTDIR) ../bfs/win32/src/win32dir.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ ../bfs/win32/src/win32dir.c

$(OUTDIR)/access.o : $(OUTDIR) ../bfs/win32/src/access.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ ../bfs/win32/src/access.c
endif

$(OUTDIR)/%.coff : %.rc
	$(RC) $(RCFLAGS) -o $@ -i $<

$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	-$(RM) $@
	$(LD) $(LDDEFS) $(LDPROF) $(BTYP_LDF) $(LDFLAGS) -o $@ $(PRGOBJS) $(PRGLIBS) $(LDLIBS)
	$(STRIP) $@

$(PRGOBJS): $(PRGHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)


clean:
	$(RMDIR) $(OUTDIR)

spotless: clean
	$(RM) *~
	$(RM) tags

print-info:
	@echo "MSYSTEM=$(MSYSTEM)"
	@echo "BDIST=$(BDIST)"
	@echo "OUTTAG=$(OUTTAG)"
	@echo "PKGPFX=$(PKGPFX)"
	@echo "OUTDIR=$(OUTDIR)"
	@echo "CC=$(CC)"
	@echo "CCDEFS=$(CCDEFS)"
	@echo "LDLIBSB=$(LDLIBSB)"
