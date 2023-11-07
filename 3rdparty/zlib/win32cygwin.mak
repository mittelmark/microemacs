# -!- makefile -!-
#
# JASSPA MicroEmacs - www.jasspa.com
# win32v6.mak - Make file for Windows using Microsoft MSVC v6.0 development kit.
#
# Copyright (C) 2001-2009 JASSPA (www.jasspa.com)
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

BUILDID  = win32cygwin
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
A	 = .a
EXE	 = .exe
CC       = gcc
CCDEFS   = -D_CYGWIN
LD       = $(CC)
LDDEFS   = 
AR       = ar
ARFLAGS  = rcs
STRIP    = strip
RM       = rm -f
RMDIR    = rm -f -r

ifeq "$(BCFG)" "release"
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCDEFS) -O3 -DNDEBUG=1 -Wall -Wno-uninitialized
LDFLAGS  = $(LDDEFS)
LDLIBS   = 
else
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCDEFS) -g -Wall
LDFLAGS  = $(LDDEFS)
LDLIBS   = 
endif

LIBNAME  = zlib
LIBFILE  = lib$(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h win32cygwin.mak
LIBOBJS  = $(OUTDIR)/adler32.o $(OUTDIR)/compress.o $(OUTDIR)/crc32.o $(OUTDIR)/gzio.o $(OUTDIR)/uncompr.o \
	   $(OUTDIR)/deflate.o $(OUTDIR)/trees.o $(OUTDIR)/zutil.o $(OUTDIR)/inflate.o $(OUTDIR)/infback.o \
	   $(OUTDIR)/inftrees.o $(OUTDIR)/inffast.o

$(OUTDIR)/%.o : %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

all: $(OUTDIR)/$(LIBFILE)

$(OUTDIR)/$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR):
	@mkdir $(OUTDIR)

clean:
	$(RMDIR) $(OUTDIRR) $(OUTDIRD)

spotless:	clean
	$(RM) *~
	$(RM) tags
