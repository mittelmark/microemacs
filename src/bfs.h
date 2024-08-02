/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Public API for reading the structure in memory
 *  Object Name   : $RCSfile: bfs.h,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2009/12/13 19:53:10 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Fri Oct 23 22:01:33 2009
 *  Last Modified : <240802.0909>
 *
 *  Description   : This API provides access to the BFS file system when
 *                  compiled into a programme.
 *
 *  Notes         : Not tested with memory partitions > 2GB
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2009 Jon Green.
 *
 *  All Rights Reserved.
 *
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#ifndef __BFS_H
#define __BFS_H

#include <stdlib.h>

#ifndef _HPUX
#if (defined _HPUX11) || (defined _HPUX10)
#define _HPUX
#endif
#endif

#ifndef _SUNOS
#if (defined _SUNOS5) || (defined _SUNOS_X86)
#define _SUNOS
#endif
#endif

/* We include the windows definitions here becase we get alot of conflict with
 * our over-riding types with the Microsoft library. Unfortunately Microsoft
 * have made a lousy job of protecting the include file. Note that we
 * explicity test for _INC_WINDOWS - this is the Microsoft protection wrapper
 * from windows.h however not all of the include files are consistent so we
 * provide further protection here. */
#if (defined(_WIN32)) || (defined(WIN32))
#define WIN32_LEAN_AND_MEAN             /* Get rid of the usual Microsoft junk */
#include <windows.h>                    /* Standard windows API */
//#include <sys/stat.h>                   
#include <sys/types.h>                  /* I/O definitions of off_t */
#undef WIN32_LEAN_AND_MEAN
#endif

/* Assume POSIX */
#if (!defined(_WIN32)) && (!defined(WIN32))
#if (!defined _HPUX) && (!defined _SUNOS)
#include <stdint.h>
#endif
#include <time.h>
#include <sys/types.h>
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Define non-portable system types */
#ifdef _WIN32
typedef DWORD bfsuint32_t;
typedef FILETIME bfstime_t;
#else
#if (defined _HPUX) || (defined _SUNOS)
typedef unsigned int bfsuint32_t;
#else
typedef uint32_t bfsuint32_t;
#endif
typedef time_t bfstime_t;
#endif

/* Define the node types */
#define BFS_TYPE_UNDEF      0x0
#define BFS_TYPE_MOUNT      0x1
#define BFS_TYPE_FILE       0x2
#define BFS_TYPE_DIR        0x3

/* Define the checksum areas */
#define BFS_CHECK_NONE      0x0
#define BFS_CHECK_HEAD      0x1
#define BFS_CHECK_DATA      0x2
#define BFS_CHECK_ALL       (BFS_CRC_HEAD|BFS_CRC_DATA)

/* Define the encoding types */
#define BFS_ENCODE_NONE     0x0
#define BFS_ENCODE_ZLIB     0x1

/* Define the version of the BFS */
#define BFS_VERSION_MAJOR   0
#define BFS_VERSION_MINOR   1
#define BFS_VERSION_MICRO   2

/* Mount operation */
struct s_bfs;
typedef struct s_bfs *bfs_t;

/* The mount point information. */
typedef struct s_bfsinfo
{
    /** The filesystem name of the archive */
    char *name;
    /** The major version of the archive */
    unsigned char version_major;
    /** The minor version of the archive */
    unsigned char version_minor;
    /** The micro version of the archive */
    unsigned char version_micro;
    /** Reserved for future use. */
    unsigned char reserved;
    /** Creation time of the file converted to the system time_t format.*/
    bfstime_t utc_ctime;
    /** The year of creation as a UTC value (archive encoded value) */
    int year;
    /** The month of creation as a UTC value (archive encoded value) */
    int month;
    /** The day of creation as a UTC value (archive encoded value) */
    int day;
    /** The hour of creation as a UTC value (archive encoded value) */
    int hour;
    /** The minute of creation as a UTC value (archive encoded value) */
    int minute;
    /** The second of creation as a UTC value (archive encoded value) */
    int second;
    /** The root of the file system offset relative to the archive start*/
    off_t root_offset;
    /** The offset of the file system from the start of the file. */
    off_t file_offset;
    /** The first free node or 0 if there are no free blocks */
    off_t free_offset;
    /** The length of the archive in bytes */
    off_t length;
    /** The data CRC-32 */
    size_t data_crc32;
    /** The header CRC-32 */
    size_t head_crc32;
} bfsinfo_t;

/**
 * Check the crc32 checksums.
 *
 * @param [in] bfs  The BFS structure.
 * @param [in] type The CRC check to perform. Specified as a bitmask of
 *                  areas to check BFS_CRC_HEAD and BFS_CRC_DATA or
 *                  BFS_CRC_ALL
 *
 * @return Status of the check 0 if successful otherwise an error.
 */
extern int
bfs_check (bfs_t bfs, int type);

/**
 * Open access to the built-in file system.
 *
 * @param [in] name The name of the file to open.
 * @param [in] type The CRC check to perform. Specified as a bitmask of
 *                  areas to check BFS_CHECK_HEAD, BFS_CHECK_DATA,
 *                  BFS_CHECK_ALL or BFS_CHECK_NONE.
 */
extern bfs_t
bfs_mount (char *name, int check);

/**
 * Return the generic information of the mount point.
 *
 * @param [in] bfs  The BFS structure.
 *
 * @return A non NULL pointer to the archive information. The return
 *         data is valid whilst the file system is mounted.
 */
extern bfsinfo_t *
bfs_info (bfs_t bfs);

/**
 * Close access to the built-in file system.
 *
 * @param [in] bfs  The BFS structure.
 */
extern void
bfs_umount (bfs_t bfs);

/* File pointer structure. */
struct s_bfsfile;
typedef struct s_bfsfile *bfsfile_t;

/**
 * Open the built-in file system
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] name The name of the file to open.
 *
 * @return A file descriptor or NULL on an error.
 */
extern bfsfile_t
bfs_fopen (bfs_t bfs, char *name);

/**
 * Close the file descriptor.
 *
 * @param [in] fp The file descriptor to close.
 */
extern void
bfs_fclose (bfsfile_t fp);

/**
 * Read from the file descriptor.
 *
 * @param [in] fp The file descriptor to read from.
 *
 * @return The character or EOF if there are no more characters.
 */
extern int
bfs_fgetc (bfsfile_t fp);

/**
 * Read from the file descriptor.
 *
 * @param [in] fp The file descriptor to read from.
 *
 * @return The number of items that have been read.
 */
extern int
bfs_fread (void *dest, size_t size, size_t nitems, bfsfile_t fp);

/* Directory structure */
struct s_bfsdir;
typedef struct s_bfsdir *bfsdir_t;

/**
 * Open a directory.
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] name The name of the directory to open.
 *
 * @return A file descriptor or NULL on an error.
 */
extern bfsdir_t
bfs_opendir (bfs_t bfs, char *name);

/**
 * Close the directory
 *
 * @param [in] The directory reference from a bfs_opendir().
 *
 * @return The status of the close operation/
 * @retval 0 Operation was successful.
 * @retval !0 Operation faied.
 */
extern int
bfs_closedir (bfsdir_t dp);

/* Directory entry structure */
typedef struct s_bfsdireent
{
    /* The bno. */
    size_t bno;
    /* The length of the name in bytes */
    size_t len;
    /* The name of the object which is not nil terminated */
    char *name;
} bfsdirent_t;

/**
 * Read the next directory entry
 *
 * @param [in] The directory reference from a bfs_opendir().
 */
extern bfsdirent_t *
bfs_readdir (bfsdir_t dp);

/**
 * @struct s_bfsstat
 * Define the structure of the bfs_stat() command.
 */
typedef struct s_bfsstat
{
    /** The mode type */
    int st_mode;
    /** The bno identity */
    off_t st_bno;
    /** The identity of the file system */
    void *st_bfs;
    /** The size of the item in bytes (uncompressed) */
    off_t st_size;
    /** The archive size of the item in bytes (may be compressed size)*/
    off_t st_asize;
    /** The encoding of the archive */
    off_t st_encoding;
    /** The creation time as UTC */
    bfstime_t st_utc_ctime;
    /** The modification time as UTC */
    bfstime_t st_utc_mtime;
} bfsstat_t;

/**
 * Get the status of the node.
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] path The pathname of the object to determine.
 * @param [in] but  The returned status information.
 */
extern int
bfs_stat (bfs_t bfs, char *path, struct s_bfsstat *buf);

/**
 * Find the file type.
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] path The pathname of the object to determine.
 *
 * @return The status of the call.
 * @retval BFS_TYPE_UNDEF This is a undefined file type.
 * @retval BFS_TYPE_MOUNT This is the mount node.
 * @retval BFS_TYPE_FILE This is a regular file.
 * @retval BFS_TYPE_DIR This is a directory.
 */
extern int
bfs_type (bfs_t bfs, char *path);

/**
 * Initialise the CRC-32 calculation. When constructing a CRC-32 then the
 * first call to to the crc calculation should have a "crc" value of
 * 0xfffffff.
 *
 * @param [in] crc The starting value or continued CRC-32 value.
 * @param [in] data The data to be CRC32'ed
 * @param [in] len  The length of the data.
 */
extern bfsuint32_t
bfs_crc32 (bfsuint32_t crc, unsigned char *data, int len);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif /* __BFS_H */
