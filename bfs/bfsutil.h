/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : BFS Utility
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 06:53:16 2009
 *  Last Modified : <240326.1354>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2009 Jon Green.
 *
 *  License: GPL - see file license.txt in the same folder as this file
 *
 ****************************************************************************/

#ifndef __BFSUTIL_H
#define __BFSUTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "bfs.h"

/* Handle the nil string when printing a string. */
#define strnil(s) (((s) == NULL) ? "" : (s))

/* Handle a status string */
#define strstatus(i) (((i) == 0) ? "ok" : "fail")

/* On Windows systems then there is no fseeko() and ftello() */
#ifdef _WIN32
#ifndef fseeko 
#define fseeko fseek
#endif
#ifndef ftello
#define ftello ftell
#endif
#endif

/* Define the utility options */
#define BFSUTIL_OPT_COMPRESS_LEVEL  0x00f  /* Compression level */
#define BFSUTIL_OPT_COMPRESS_FILE   0x010  /* Compress files */
#define BFSUTIL_OPT_COMPRESS_DIR    0x020  /* Compress directories */
#define BFSUTIL_OPT_SKIP_ZERO_LEN   0x040  /* Skip zero length files */
#define BFSUTIL_OPT_QUIET           0x080  /* Minimise status messages */
#define BFSUTIL_OPT_VERBOSE         0x100  /* Add informative messages */

/* Version numbers */
extern int bfs_version_major;
extern int bfs_version_minor;
extern int bfs_version_micro;

/**
 * List the archive on stdout.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to process.
 *
 * @return The status of the call, 0 is success.
 */
extern int
bfs_list (int options, char *archpath);

/**
 * Extract the archive to a directory.
 *
 * @param [in] options  The option mask.
 * @param [in] dirname The name of the output directory.
 * @param [in] archpath The name of the archive.
 *
 * @return The status of the call, 0 is success.
 */
extern int
bfs_xdir (int options, char *dirname, char *archpath);

/**
 * Extract an acrchive to the named file system file. The archive may be
 * appended to the end of the file.
 *
 * @param [in] options  The option mask.
 * @param [in] fsysname The filename system name of the output archive.
 * @param [in] archpath The path to the archive to extract.
 *
 * @return The status 0 on success otherwise an error.
 */
extern int
bfs_xfile (int options, char *fsysname, char *archpath);

/**
 * Get the archive information.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The path to the archive to extract.
 *
 * @return The status 0 on success otherwise an error.
 */
extern int
bfs_printinfo (int options, char *archpath);

/**
 * Strip the archive from the end of a file. The archive is removed from the
 * end of the file leaving the original file. No backup file is made.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 *
 * @return The status of the call, 0 is success.
 */
extern int
bfs_strip (int options, char *archpath);

/**
 * Test the integrity of the archive.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 *
 * @return The status of the call, 0 is success.
 */
extern int
bfs_test (int options, char *archpath);

/**
 * Construct a BFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] fp The file pointer to write.
 * @param [in] direname The directory to process.
 *
 * @return Status of the call, 0 is success.
 */
extern int
bfs_build (int options, FILE *fp, char *dirname);

/**
 * Construct a BFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] appendfile The name of the file to be appended.
 * @param [in] outputfile The name of the file to output, may be NULL if we are
 *                        rewriting the same file.
 * @param [in] fileordirename The name of the archive to append. This may be
 *                            and existing archive file or a directory.
 *
 * @return Status of the call, 0 is success.
 */
extern int
bfs_append (int options, char *appendfile, char *outputfile, char *fileordir);

/**
 * Construct a BFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] outputfile The name of the file to output.
 * @param [in] fileordirename The name of the archive to append. This may be
 *                            and existing archive file or a directory.
 *
 * @return Status of the call, 0 is success.
 */
extern int
bfs_create (int options, char *outputfile, char *fileordir);

/**
 * Copy a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to create.
 *
 * @return Status of the call, 0 is success.
 */
extern int
bfs_filecopy (int options, char *srcfile, char *destfile);

/**
 * Append a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to append.
 *
 * @return Status of the call, 0 is success.
 */
extern int
bfs_fileappend (int options, char *srcfile, char *destfile);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __BFSUTIL_H */
