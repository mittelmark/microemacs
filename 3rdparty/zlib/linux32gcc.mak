# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32vc10.mak - Make file for Linux v5 Kernel using gcc
#
# Copyright (C) 2007-2022 JASSPA (www.jasspa.com)
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

A        = .a
EXE      = 

CC       = cc
MK       = make
LD       = $(CC)
STRIP    = strip
AR       = ar
RM       = rm -f
RMDIR    = rm -rf

BUILDID  = linux32gcc
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = -D_LINUX -D_LINUX5 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I.
CCFLAGSR = -O3 -flto -DNDEBUG=1 -Wall -Wno-uninitialized
CCFLAGSD = -g -Wall
LDDEFS   = 
LDFLAGSR = -O3 -flto
LDFLAGSD = -g
ARFLAGSR = rcs
ARFLAGSD = rcs

ifeq "$(BCFG)" "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
else
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
endif

LIBNAME  = zlib
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)/adler32.o $(OUTDIR)/compress.o $(OUTDIR)/crc32.o $(OUTDIR)/gzio.o $(OUTDIR)/uncompr.o \
	   $(OUTDIR)/deflate.o $(OUTDIR)/trees.o $(OUTDIR)/zutil.o $(OUTDIR)/inflate.o $(OUTDIR)/infback.o \
	   $(OUTDIR)/inftrees.o $(OUTDIR)/inffast.o

.SUFFIXES: .c .o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCDEFS) $(CCFLAGS) -c -o $@ $<

all: $(OUTDIR)/$(LIBFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR):
	-mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRD)
	$(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
