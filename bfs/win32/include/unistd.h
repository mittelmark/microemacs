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

/* File access constants for access() - only define if not already available */
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

#include <fcntl.h>
#include <io.h>

/* Stub functions for Windows - access declared here since MinGW might not have it */
#ifndef _ACCESS_
#define _ACCESS_
int access(const char *path, int mode);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __UNISTD_H */
