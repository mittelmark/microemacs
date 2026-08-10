# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# linuxmingwgcc.mak - Make file for cross-compiling to Windows using MinGW (Linux host).
#
# This makefile is for CROSS-COMPILATION from Linux to Windows.
# For native Windows/MSYS2 builds, see winmingwgcc.mak
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
# Synopsis:    Make file for cross-compiling to Windows using MinGW (Linux host).
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
#     To build from the command line using make & makefile.
#
#     Cross-compile toolchains (Linux host):
#       BDIST=mingw64 (default) - x86_64-w64-mingw32-gcc, static zlib, native Windows
#       BDIST=ucrt64            - x86_64-w64-mingw32-gcc (UCRT), static zlib, native Windows
#       BDIST=mingw32           - i686-w64-mingw32-gcc, static zlib, 32-bit Windows
#
#     Run "make -f linuxmingwgcc.mak"                for 64-bit release build
#     Run "make -f linuxmingwgcc.mak BCFG=debug"     for debug build
#     Run "make -f linuxmingwgcc.mak BTYP=c"         for console support
#     Run "make -f linuxmingwgcc.mak BDIST=ucrt64"   for UCRT 64-bit build
#     Run "make -f linuxmingwgcc.mak BDIST=mingw32"  for 32-bit build
#     Run "make -f linuxmingwgcc.mak BCOR=ne"        for ne build
#
#     Run "make -f linuxmingwgcc.mak clean"          to clean source directory
#     Run "make -f linuxmingwgcc.mak spotless"       to clean source directory even more
#
#     Output directories:
#       BDIST=mingw64: ./.linuxmingwgcc-mingw64-{release,debug}-me{c,w}/
#       BDIST=ucrt64:  ./.linuxmingwgcc-ucrt64-{release,debug}-me{c,w}/
#       BDIST=mingw32: ./.linuxmingwgcc-mingw32-{release,debug}-me{c,w}/
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

# BDIST controls the cross-compile target toolchain and runtime:
#   mingw64 (default) - x86_64-w64-mingw32-gcc, static zlib, native Windows
#   ucrt64            - x86_64-w64-mingw32-gcc (UCRT), static zlib, native Windows
#   mingw32           - i686-w64-mingw32-gcc, static zlib, 32-bit Windows
BDIST ?= mingw64

# Detect UCRT support: some older toolchains (e.g. Ubuntu 22.04 GCC 11.2)
# do not recognize -mcrtdll=ucrt. Test at configure time.
ifeq "$(BDIST)" "ucrt64"
UCRT_CFLAGS := $(shell echo 'int main(){return 0;}' | \
    $(CC) -mcrtdll=ucrt -x c - -o /dev/null - 2>/dev/null && echo "-mcrtdll=ucrt" || echo "")
ifeq "$(UCRT_CFLAGS)" ""
$(warning UCRT support not available in toolchain, falling back to msvcrt)
BDIST := mingw64
endif
CCFLAGSR_UCRT = $(UCRT_CFLAGS)
CCFLAGSD_UCRT = $(UCRT_CFLAGS)
LDFLAGSR_UCRT = $(UCRT_CFLAGS)
LDFLAGSD_UCRT = $(UCRT_CFLAGS)
endif

# Select cross-compile toolchain based on BDIST
ifeq "$(BDIST)" "mingw32"
CC       = i686-w64-mingw32-gcc
RC       = i686-w64-mingw32-windres
MK       = make
LD       = $(CC)
STRIP    = i686-w64-mingw32-strip
AR       = i686-w64-mingw32-ar
else
CC       = x86_64-w64-mingw32-gcc
RC       = x86_64-w64-mingw32-windres
MK       = make
LD       = $(CC)
STRIP    = x86_64-w64-mingw32-strip
AR       = x86_64-w64-mingw32-ar
endif
RM       = rm -f
RMDIR    = rm -r -f

TOOLKIT  = linuxmingwgcc
# BDTAG controls output directory name (e.g. .linuxmingwgcc-mingw64-release-...)
ifeq "$(BDIST)" "mingw32"
BDTAG = mingw32
else ifeq "$(BDIST)" "ucrt64"
BDTAG = ucrt64
else
BDTAG = mingw64
endif
ifeq "$(BPRF)" "1"
BUILDID  = $(TOOLKIT)p
else
BUILDID  = $(TOOLKIT)
endif
OUTDIRR  = .$(TOOLKIT)-$(BDTAG)-release
OUTDIRD  = .$(TOOLKIT)-$(BDTAG)-debug

CCDEFS   = -D_WIN32 -Wall
# UCRT target needs -mcrtdll=ucrt; this switches from msvcrt.dll to ucrtbase.dll
# -mfpmath=sse is only valid for x86_64; omit for i686 (mingw32)
ifeq "$(BDIST)" "mingw32"
CCFLAGSR = -O3 -Ofast -flto -march=native -funroll-loops -DNDEBUG=1 -Wno-uninitialized
LDFLAGSR = -O3 -Ofast -flto -funroll-loops
else
ifeq "$(BDIST)" "ucrt64"
CCFLAGSR = -O3 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -DNDEBUG=1 -Wno-uninitialized $(CCFLAGSR_UCRT)
LDFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops $(LDFLAGSR_UCRT)
else
CCFLAGSR = -O3 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -DNDEBUG=1 -Wno-uninitialized
LDFLAGSR = -O3 -mfpmath=sse -Ofast -flto -funroll-loops
endif
endif
CCFLAGSD = -g -D_DEBUG
LDDEFS   =
LDFLAGSD = -g
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -Wl,-Bstatic -lz -Wl,-Bdynamic

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

OUTDIR   = $(BOUTDIR)-$(BCOR)$(BTYP)
PRGNAME  = $(BCOR)$(BTYP)32
PRGFILE  = $(PRGNAME)$(EXE)
PRGHDRS  = ebind.h edef.h eextrn.h efunc.h emain.h emode.h eprint.h esearch.h eskeys.h estruct.h eterm.h evar.h evers.h eopt.h \
	   ebind.def efunc.def eprint.def evar.def etermcap.def emode.def eskeys.def bfs.h \
	   $(TOOLKIT).mak
PRGOBJS  = $(OUTDIR)/abbrev.o $(OUTDIR)/basic.o $(OUTDIR)/bfs.o $(OUTDIR)/bind.o $(OUTDIR)/buffer.o $(OUTDIR)/crypt.o $(OUTDIR)/dirlist.o $(OUTDIR)/display.o \
	   $(OUTDIR)/eval.o $(OUTDIR)/exec.o $(OUTDIR)/file.o $(OUTDIR)/fileio.o $(OUTDIR)/frame.o $(OUTDIR)/hilight.o $(OUTDIR)/history.o \
	   $(OUTDIR)/input.o $(OUTDIR)/isearch.o $(OUTDIR)/key.o $(OUTDIR)/line.o $(OUTDIR)/macro.o $(OUTDIR)/main.o $(OUTDIR)/narrow.o $(OUTDIR)/next.o \
	   $(OUTDIR)/osd.o $(OUTDIR)/print.o $(OUTDIR)/random.o $(OUTDIR)/regex.o $(OUTDIR)/region.o $(OUTDIR)/registry.o $(OUTDIR)/search.o $(OUTDIR)/spawn.o \
	   $(OUTDIR)/spell.o $(OUTDIR)/tag.o $(OUTDIR)/termio.o $(OUTDIR)/time.o $(OUTDIR)/undo.o $(OUTDIR)/window.o $(OUTDIR)/word.o \
	   $(OUTDIR)/winterm.o $(OUTDIR)/winprint.o $(OUTDIR)/$(BCOR).coff \
	   $(OUTDIR)/win32dir.o $(OUTDIR)/access.o
#
# Rules
.SUFFIXES: .c .o .rc .coff

all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<

$(OUTDIR)/win32dir.o : $(OUTDIR) ../bfs/win32/src/win32dir.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ ../bfs/win32/src/win32dir.c

$(OUTDIR)/access.o : $(OUTDIR) ../bfs/win32/src/access.c
	$(CC) $(CCDEFS) $(CCPROF) $(CCFLAGS) -c -o $@ ../bfs/win32/src/access.c

$(OUTDIR)/%.coff : %.rc
	$(RC) $(RCFLAGS) -o $@ -i $<


$(OUTDIR)/$(PRGFILE): $(OUTDIR) $(PRGOBJS) $(PRGLIBS)
	$(RM) $@
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
