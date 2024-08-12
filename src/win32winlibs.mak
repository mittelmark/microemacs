# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32mingw.mak - Make file for Windws using MinGW development kit.
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
# Synopsis:    Make file for Windows using MinGW development kit.
# Notes:
#     Run ./build.sh to compile, ./build.sh -h for more information.
#
#     To build from the command line using make & makefile. 
#
#	Run "mingw32-make -f win32mingw.mak"            for optimised build produces ./.win32mingw-release-mew/mew32.exe
#	Run "mingw32-make -f win32mingw.mak BCFG=debug" for debug build produces     ./.win32mingw-debug-mew/mew32.exe
#	Run "mingw32-make -f win32mingw.mak BTYP=c"     for console support          ./.win32mingw-release-mec/mec32.exe
#	Run "mingw32-make -f win32mingw.mak BCOR=ne"    for ne build produces        ./.win32mingw-release-new/new32.exe
#
#	Run "mingw32-make -f win32mingw.mak clean"      to clean source directory
#	Run "mingw32-make -f win32mingw.mak spotless"   to clean source directory even more
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

CC       = CC
RC       = windres
MK       = mingw32-make
LD       = $(CC)
STRIP    = strip
AR       = i686-w64-mingw32-ar
RM       = rm -f
RMDIR    = rm -r -f

TOOLKIT  = win32winlibs
ifeq "$(BPRF)" "1"
BUILDID  = $(TOOLKIT)p
else
BUILDID  = $(TOOLKIT)
endif
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug

CCDEFS   = -D_MINGW -Wall -I/usr/i686-w64-mingw32/sys-root/mingw/include -I../bfs/win32/include
CCFLAGSR = -O3 -m32 -mfpmath=sse -Ofast -flto -march=native -funroll-loops -DNDEBUG=1 -Wno-uninitialized
CCFLAGSD = -g -D_DEBUG
LDDEFS   = 
LDFLAGSR = -O3 -m32 -mfpmath=sse -Ofast -flto -march=native -funroll-loops
LDFLAGSD = -g
LDLIBSB  = -lshell32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -L../bfs/win32/lib -lz

ARFLAGSR = rcs
ARFLAGSD = rcs
RCFLAGS  = --input-format rc --output-format coff -D_MINGW

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
	   $(OUTDIR)/winterm.o $(OUTDIR)/winprint.o $(OUTDIR)/$(BCOR).coff
#
# Rules
.SUFFIXES: .c .o .rc .coff

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCPROF) $(BCOR_CDF) $(BTYP_CDF) $(CCFLAGS) -c -o $@ $<

$(OUTDIR)/%.coff : %.rc
	$(RC) $(RCFLAGS) -o $@ -i $<

all: $(PRGLIBS) $(OUTDIR)/$(PRGFILE)

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
