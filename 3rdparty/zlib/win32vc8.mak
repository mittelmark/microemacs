##############################################################################
#
#  Copyright (c) 2009 Maxexam Ltd.
# 
#  All Rights Reserved.
# 
#  This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
#  reproduced,  translated,  or  reduced to any  electronic  medium or machine
#  readable form without prior written consent from Maxexam Ltd.
#
##############################################################################

A        = .lib
EXE      = .exe

CC       = cl.exe /nologo
RC       = rc.exe
MT       = mt.exe -nologo
MK       = nmake.exe /NOLOGO
LD       = link /NOLOGO
AR       = lib /NOLOGO
RM       = del /F /Q
RMDIR    = rd /S /Q

BUILDID  = win32vc8
OUTDIRR  = .$(BUILDID)-release
OUTDIRD  = .$(BUILDID)-debug
TRDPARTY = ..

CCDEFS   = /DWIN32 /D_WIN32 /D_WIN32_WINNT=0x500 /W3 /Zi /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE
CCFLAGSR = /MD /O2 /GL /GS- /DNDEBUG=1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0
CCFLAGSD = /MDd /Od /RTC1 /D_DEBUG
LDDEFS   = /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MACHINE:X86 /MANIFEST
LDFLAGSR = /OPT:REF /OPT:ICF /LTCG
LDFLAGSD = /DEBUG
ARFLAGSR = /LTCG
ARFLAGSD =
RCFLAGS  =

!IF "$(BCFG)" == "debug"
OUTDIR   = $(OUTDIRD)
CCFLAGS  = $(CCFLAGSD)
LDFLAGS  = $(LDFLAGSD)
ARFLAGS  = $(ARFLAGSD)
!ELSE
OUTDIR   = $(OUTDIRR)
CCFLAGS  = $(CCFLAGSR)
LDFLAGS  = $(LDFLAGSR)
ARFLAGS  = $(ARFLAGSR)
!ENDIF

LIBNAME  = zlib 
LIBFILE  = $(LIBNAME)$(A)
LIBHDRS  = zutil.h zlib.h zconf.in.h zconf.h trees.h inftrees.h inflate.h \
	   inffixed.h inffast.h deflate.h crc32.h $(BUILDID).mak
LIBOBJS  = $(OUTDIR)\adler32.o $(OUTDIR)\compress.o $(OUTDIR)\crc32.o $(OUTDIR)\gzio.o $(OUTDIR)\uncompr.o \
	   $(OUTDIR)\deflate.o $(OUTDIR)\trees.o $(OUTDIR)\zutil.o $(OUTDIR)\inflate.o $(OUTDIR)\infback.o \
	   $(OUTDIR)\inftrees.o $(OUTDIR)\inffast.o

.SUFFIXES: .c .o

.c{$(OUTDIR)}.o:
	$(CC) $(CCDEFS) $(CCFLAGS) /Fd"$(OUTDIR)\vc80.pdb" /c $< /Fo"$@"

all: $(OUTDIR)\$(LIBFILE)

$(OUTDIR)\$(LIBFILE): $(OUTDIR) $(LIBOBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) /OUT:"$@" $(LIBOBJS)

$(LIBOBJS): $(LIBHDRS)

$(OUTDIR):
	if not exist $(OUTDIR)\ mkdir $(OUTDIR)

clean:
	if exist $(OUTDIRD)\ $(RMDIR) $(OUTDIRD)
	if exist $(OUTDIRR)\ $(RMDIR) $(OUTDIRR)

spotless: clean
	$(RM) *~
	$(RM) tags
