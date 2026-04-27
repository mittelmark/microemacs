/* -*- C -*- ****************************************************************
 *
 *    			   Copyright 2002 Jon Green.
 *			      All Rights Reserved
 *
 *
 *  System        : 
 *  Module        : 
 *  Object Name   : $RCSfile: unistd.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2004/09/05 23:39:01 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Sun Mar 3 12:55:20 2002
 *  Last Modified : <091101.1246>
 *
 *  Description	
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2002 Jon Green.
 * 
 *  All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef __UNISTD_H
#define __UNISTD_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* File access constants - define if not from system headers.
 * These are standard POSIX values. */
#ifndef F_OK
#define F_OK    0x00
#endif

#ifndef R_OK
#define R_OK    0x04
#endif

#ifndef W_OK
#define W_OK    0x02
#endif

#ifndef X_OK
#define X_OK    0x01
#endif

/* File open constants - define if not from system headers */
#ifndef _O_RDWR
#define _O_RDWR    0x0002
#endif

#ifndef _O_CREAT
#define _O_CREAT   0x0040
#endif

#ifndef _O_TRUNC
#define _O_TRUNC   0x0200
#endif

#ifndef _S_IREAD
#define _S_IREAD   0x0100
#endif

#ifndef _S_IWRITE
#define _S_IWRITE  0x0080
#endif

/* Include system headers on native MinGW/MSYS2 to get mkdir(), etc.
 * This must be done BEFORE the function declarations.
 * Note: We only include <sys/stat.h> for mkdir(), NOT <unistd.h> to avoid
 * circular includes and ensure our access() declaration is used. */
#if defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include <sys/stat.h>
#endif

/* Stub functions for Windows.
 * These declarations are needed by the bfs utility functions.
 * Skip them when cross-compiling with system headers (-DNO_BFS_UNISTD_STUBS). */
#ifndef NO_BFS_UNISTD_STUBS

#ifndef __bfs_access_declared
#define __bfs_access_declared
int access(const char *path, int mode);
#endif

#ifndef __bfs_open_declared
#define __bfs_open_declared
int _open(const char *path, int oflags, int mode);
#endif

#ifndef __bfs_close_declared
#define __bfs_close_declared
int _close(int fd);
#endif

#ifndef __bfs_chsize_declared
#define __bfs_chsize_declared
int _chsize(int fd, long size);
#endif

/* Note: mkdir stub is NOT declared here because:
 * - MinGW system headers declare 1-arg mkdir(path)
 * - MSYS2 system headers declare 2-arg mkdir(path, mode)
 * - Our code handles both with conditional compilation in uxdir.c */

#endif /* NO_BFS_UNISTD_STUBS */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __UNISTD_H */
