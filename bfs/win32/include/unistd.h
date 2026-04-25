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

/* File access constants - define if not from system headers */
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

/* Stub functions for Windows - only declare if not from system headers.
 * The system headers are included via sys/stat.h in bfsutil.h, which brings in io.h.
 * We use guards to avoid re-declaring if system already has them. */
#ifndef __BFS_UNISTD_ACCESS_DECLARED
#define __BFS_UNISTD_ACCESS_DECLARED
int access(const char *path, int mode);
#endif

/*
 * Note: _open, _close, _chsize are declared in system io.h when available.
 * We don't declare them here to avoid conflicts with MinGW's declarations.
 */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __UNISTD_H */
