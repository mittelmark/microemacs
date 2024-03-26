##-*- makefile -*-############################################################
#
#  System        : Built-in File System
#  Module        : Make rules for the bfs command line utility.
#  Object Name   : $RCSfile$
#  Revision      : $Revision$
#  Date          : $Date$
#  Author        : $Author$
#  Created By    : Jon Green
#  Created       : Wed Oct 21 15:58:59 2009
#  Last Modified : <240326.1609>
#
#  Description   : Unix/Linux only
#
#  Notes
#
#  History
#
##############################################################################
#
#  Copyright (c) 2009 Jon Green.
#
#  License: GPL - see file license.txt in the same folder as this file
#
##############################################################################

#
# Define the build definitions.
#
CFLAGS=		-g -O0
LIBS=		-lz
#
# Some useful commands
#
RM=		rm -f
#
# Define the source and object files.
#
BFSSRC=		bfsutil.c \
		bfs.c \
		uappend.c \
		ubuild.c \
		ucopy.c \
		ucreate.c \
		uinfo.c \
		ulist.c \
		ustrip.c \
		utest.c \
		uxdir.c \
		uxfile.c
BFSOBJ=		$(BFSSRC:.c=.o)
BFS=		bfs
#
# Define the basic rules.
#
all:		$(BFS)
#
clean:
		$(RM) *.o
		$(RM) $(BFS)
#
spotless:	clean
		$(RM) *~
		$(RM) core
		$(RM) tags
#
#############################################################################
# Rules for the bfsbuild utility.
#
$(BFS):		$(BFSOBJ)
		$(CC) $(CFLAGS) -o $@ $(BFSOBJ) -lz
#
bfs.o:		bfs.h
bfsutil.o:	bfs.h bfsutil.h
uappend.o:	bfs.h bfsutil.h
ubuild.o:	bfs.h bfsutil.h
ucopy.o:	bfs.h bfsutil.h
ucreate.o:	bfs.h bfsutil.h
uinfo.o:	bfs.h bfsutil.h
ulist.o:	bfs.h bfsutil.h
ustrip.o:	bfs.h bfsutil.h
utest.o:	bfs.h bfsutil.h
uxdir.o:	bfs.h bfsutil.h
uxfile.o:	bfs.h bfsutil.h
