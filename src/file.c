/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * file.c - File handling.
 *
 * Copyright (C) 1988-2009 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Unknown
 * Synopsis:    File handling.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:
 *     The routines in this file handle the reading, writing
 *     and lookup of disk files.  All of details about the
 *     reading and writing of the disk are in "fileio.c".
 */

#define __FILEC 1                  /* Define the filename */

#include "emain.h"
#include "efunc.h"
#include "esearch.h"
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include <errno.h>
#include <limits.h>                     /* Constant limit definitions */
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>                     /* Directory entries */
#else
#ifdef _DIRENT
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#endif
#include <time.h>
#endif

#ifdef _DOS
#include <sys/file.h>
#include <dir.h>
#include <errno.h>
#include <pc.h>
#include <dos.h>

/* attribute stuff */
#define A_RONLY         0x01
#define A_HIDDEN        0x02
#define A_SYSTEM        0x04
#define A_LABEL         0x08
#define A_DIR           0x10
#define A_ARCHIVE       0x20

/* dos call values */
#define DOSI_GETDRV     0x19
#define DOSI_GETDIR     0x47

#endif  /* _DOS */

#ifdef _UNIX
#include <utime.h>

#define statTestRead(st)                                                   \
((((st).st_uid == meUid) && ((st).st_mode & S_IRUSR)) ||                   \
 ((st).st_mode & S_IROTH) ||                                               \
 (((st).st_mode & S_IRGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#define statTestWrite(st)                                                  \
((((st).st_uid == meUid) && ((st).st_mode & S_IWUSR)) ||                   \
 ((st).st_mode & S_IWOTH) ||                                               \
 (((st).st_mode & S_IWGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#define statTestExec(st)                                                   \
((((st).st_uid == meUid) && ((st).st_mode & S_IXUSR)) ||                   \
 ((st).st_mode & S_IXOTH) ||                                               \
 (((st).st_mode & S_IXGRP) &&                                              \
  (((st).st_gid == meGid) || (meGidSize && meGidInGidList((st).st_gid)))))
#endif

/* Min length of root */
#ifdef _DRV_CHAR
#define _ROOT_DIR_LEN  3                /* 'c:\' */
#else
#define _ROOT_DIR_LEN  1                /* '/' */
#endif

int
getFileStats(meUByte *file, int flag, meStat *stats, meUByte *lname)
{
    if(lname != NULL)
        *lname = '\0' ;
    /* setup the default stat values */
    if(stats != NULL)
    {
        meFiletimeInit(stats->stmtime) ;
        stats->stsizeHigh = 0 ;
        stats->stsizeLow = 0 ;
#ifdef _UNIX
        stats->stuid  = meUid ;
        stats->stgid  = meGid ;
        stats->stdev  = -1 ;
        stats->stino  = -1 ;
#endif
    }
    if(isHttpLink(file))
        return meFILETYPE_HTTP ;
    if(isFtpLink(file))
        return meFILETYPE_FTP ;
#ifdef MEOPT_BINFS
    if (isBfsFile(file))
    {
        bfsstat_t bfs_statbuf;
        int file_type = meFILETYPE_NOTEXIST ;

        if (bfs_stat (bfsdev, (char *)(file+5), &bfs_statbuf) == 0)
        {
            if (bfs_statbuf.st_mode == BFS_TYPE_FILE)
                file_type = meFILETYPE_REGULAR ;
            else if (bfs_statbuf.st_mode == BFS_TYPE_DIR)
            {
                if(flag & 2)
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file) ;
                file_type = meFILETYPE_DIRECTORY ;
            }
            if (stats != NULL)
            {
                /* Get the file size. */
                stats->stsizeLow = (meUInt) bfs_statbuf.st_size ;
                /* Set the permissions. */
                stats->stmode = 0;
                if (file_type == meFILETYPE_DIRECTORY)
                {
#ifdef _DOS
                    stats->stmode = FA_DIR;
#endif
#ifdef _WIN32
                    stats->stmode = FILE_ATTRIBUTE_DIRECTORY;
#endif
#ifdef _UNIX
                    stats->stmode = S_IXUSR|S_IXGRP|S_IXOTH|0040000;
#endif
                }
#ifdef _WIN32
                stats->stmode |= FILE_ATTRIBUTE_READONLY;
                /* Convert the time to Windows format */
                stats->stmtime = bfs_statbuf.st_utc_mtime ;
#endif
#ifdef _DOS
                stats->stmode |= FA_RDONLY;
                stats->stmtime = bfs_statbuf.st_utc_mtime;
#endif
#ifdef _UNIX
                stats->stdev = meIntFromPtr(bfsdev);
                stats->stino = (meUInt) bfs_statbuf.st_bno;
                stats->stmode |= S_IRGRP|S_IROTH|S_IRUSR;
                if (file_type == meFILETYPE_REGULAR)
                    stats->stmode |= 0100000;
                stats->stmtime = bfs_statbuf.st_utc_mtime;
#endif
            }
        }
        return file_type;
    }
#endif
    if(isUrlFile(file))
        /* skip the file: */
        file += 5 ;

#ifdef _DOS
    {
        int ii ;

        if(((ii = meStrlen(file)) == 0) ||
           (strchr(file,'*') != NULL) ||
           (strchr(file,'?') != NULL))
        {
            if(flag & 1)
                mlwrite(MWABORT|MWPAUSE,"[%s illegal name]", file);
            return meFILETYPE_NOTEXIST ;
        }
        if((file[ii-1] == DIR_CHAR) || ((ii == 2) && (file[1] == _DRV_CHAR)))
            goto gft_directory ;

#ifdef __DJGPP2__
        ii = meFileGetAttributes(file) ;
        if(ii < 0)
            return meFILETYPE_NOTEXIST ;
#else
        {
            union REGS reg ;                /* cpu register for use of DOS calls */
            reg.x.ax = 0x4300 ;
            reg.x.dx = ((unsigned long) file) ;
            intdos(&reg, &reg);

            if(reg.x.cflag)
                return meFILETYPE_NOTEXIST ;
            ii = reg.x.cx ;
        }
#endif
        if(stats != NULL)
        {
            struct ffblk fblk ;
            stats->stmode = ii | meFILE_ATTRIB_ARCHIVE ;
            if(!findfirst(file, &fblk, FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC))
            {
                stats->stmtime = (((meUInt) fblk.ff_ftime) & 0x0ffff) | (((meUInt) fblk.ff_fdate) << 16) ;
                stats->stsizeLow = fblk.ff_fsize ;
            }
        }
        if(ii & meFILE_ATTRIB_DIRECTORY)
        {
gft_directory:
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,"[%s directory]", file) ;
            return meFILETYPE_DIRECTORY ;
        }
        return meFILETYPE_REGULAR ;
    }
#else
#ifdef _WIN32
    {
        DWORD status;
        int   len ;

        if(((len = meStrlen(file)) == 0) ||
           (strchr(file,'*') != NULL) || (strchr(file,'?') != NULL))
        {
            if(flag & 1)
                mlwrite(MWABORT|MWPAUSE,"[%s illegal name]", file);
            return meFILETYPE_NOTEXIST ;
        }
        if(stats != NULL)
        {
            HANDLE          *fh ;
            WIN32_FIND_DATA  fd ;

            if(file[len-1] == DIR_CHAR)
            {
                meUByte fn[meBUF_SIZE_MAX] ;
                strcpy(fn,file) ;
                fn[len] = '.' ;
                fn[len+1] = '\0' ;
                fh = FindFirstFile(fn,&fd) ;
            }
            else
                fh = FindFirstFile(file,&fd) ;
            if(fh == INVALID_HANDLE_VALUE)
            {
                if((file[len-1] == DIR_CHAR) || ((len == 2) && (file[1] == _DRV_CHAR)))
                    goto gft_directory ;
                return meFILETYPE_NOTEXIST ;
            }
            status = fd.dwFileAttributes ;
            stats->stsizeHigh = fd.nFileSizeHigh ;
            stats->stsizeLow = fd.nFileSizeLow ;
            stats->stmtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
            stats->stmtime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime ;
            FindClose(fh) ;
            stats->stmode = (meUShort) status | FILE_ATTRIBUTE_ARCHIVE ;
        }
        else if((file[len-1] == DIR_CHAR) || ((len == 2) && (file[1] == _DRV_CHAR)))
            goto gft_directory ;
        else if((status = GetFileAttributes(file)) == 0xFFFFFFFF)
            return meFILETYPE_NOTEXIST ;
        if (status & FILE_ATTRIBUTE_DIRECTORY)
        {
gft_directory:
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,"[%s directory]", file);
            return meFILETYPE_DIRECTORY ;
        }
        /* FILE_ATTRIBUTE_ARCHIVE
         * The file or directory is an archive file or directory.
         * Applications use this flag to mark files for backup or removal.
         *
         * FILE_ATTRIBUTE_HIDDEN
         * The file or directory is hidden.
         * It is not included in an ordinary directory listing.
         *
         * FILE_ATTRIBUTE_READONLY
         * The file or directory is read-only.
         * Applications can read the file but cannot write to it or delete it.
         * In the case of a directory, applications cannot delete it.
         *
         * FILE_ATTRIBUTE_SYSTEM
         * The file or directory is part of, or is used exclusively by,
         * the operating system.
         *
         * FILE_ATTRIBUTE_NORMAL
         * The file or directory has no other attributes set.
         * This attribute is valid only if used alone. */
        return meFILETYPE_REGULAR ;
    }
#else
#ifdef _UNIX
    {
        struct stat statbuf;
        long stmtime = -1 ;

        if((lname == NULL) && (stats == NULL))
        {
            if(stat((char *)file, &statbuf) != 0)
                return meFILETYPE_NOTEXIST ;
        }
        else if(lstat((char *)file, &statbuf) != 0)
            return meFILETYPE_NOTEXIST ;
        else if(S_ISLNK(statbuf.st_mode))
        {
            meUByte lbuf[meBUF_SIZE_MAX], buf[meBUF_SIZE_MAX], *ss ;
            size_t ii, jj ;
            int maxi=10 ;

            ii = meStrlen(file);
            //meStrncpy(lbuf,file,ii) ;
            memcpy(lbuf,file,ii);
            do {
                if(file[ii-1] == DIR_CHAR)
                    ii-- ;
                lbuf[ii] = '\0' ;
                if(statbuf.st_mtime > stmtime)
                    stmtime = statbuf.st_mtime ;
                if((jj=readlink((char *)lbuf,(char *)buf, meBUF_SIZE_MAX)) <= 0)
                {
                    if(flag & 1)
                        mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s symbolic link problems]", file);
                    return meFILETYPE_NASTY ;
                }
                buf[jj] = '\0' ;
                if((buf[0] == DIR_CHAR) || ((ss=meStrrchr(lbuf,DIR_CHAR)) == NULL))
                    meStrcpy(lbuf,buf) ;
                else
                {
                    ss++ ;
                    ii = (size_t)(ss - lbuf) ;
                    meStrcpy(ss,buf) ;
                    ii += jj ;
                }
            } while(((jj=lstat((char *)lbuf, &statbuf)) == 0) && (--maxi > 0) && S_ISLNK(statbuf.st_mode)) ;

            if(lname != NULL)
            {
                fileNameCorrect(lbuf,lname,NULL) ;
                if(S_ISDIR(statbuf.st_mode))
                {
                    /* make sure that a link to a dir has a trailing '/' */
                    meUByte *ss=lname+meStrlen(lname) ;
                    if(ss[-1] != DIR_CHAR)
                    {
                        ss[0] = DIR_CHAR ;
                        ss[1] = '\0' ;
                    }
                }
            }
            if(jj)
            {
                if(flag & 1)
                    mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s dangling symbolic link]",lbuf);
                if(stats != NULL)
                    stats->stmtime = stmtime ;
                return meFILETYPE_NOTEXIST ;
            }
        }

        if(stats != NULL)
        {
            stats->stmode = statbuf.st_mode ;
            stats->stuid  = statbuf.st_uid ;
            stats->stgid  = statbuf.st_gid ;
            stats->stdev  = statbuf.st_dev ;
            stats->stino  = statbuf.st_ino ;
#ifdef _LARGEFILE_SOURCE
            stats->stsizeHigh = (meUInt) (statbuf.st_size >> 32) ;
            stats->stsizeLow = (meUInt) statbuf.st_size ;
#else
            stats->stsizeLow = statbuf.st_size ;
#endif
            if(statbuf.st_mtime > stmtime)
                stats->stmtime= statbuf.st_mtime ;
            else
                stats->stmtime= stmtime ;
        }
        if(S_ISREG(statbuf.st_mode))
        {
            return meFILETYPE_REGULAR ;
        }
        if(S_ISDIR(statbuf.st_mode))
        {
            if(flag & 2)
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s directory]", file);
            return meFILETYPE_DIRECTORY ;
        }
        if(flag & 1)
        {
#ifdef S_IFIFO
            if(S_ISFIFO(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is a FIFO]", file);
#endif
#ifdef S_IFCHR
            else if(S_ISCHR(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is character special]", file);
#endif
#ifdef S_IFBLK
            else if(S_ISBLK(statbuf.st_mode))
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s is block special]", file);
#endif
            else
                mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s not a regular file]", file);
        }
        return meFILETYPE_NASTY ;
    }
#else
    return meFILETYPE_REGULAR ;
#endif /* _UNIX */
#endif /* _WIN32 */
#endif /* _DOS */
}

int
fnamecmp(meUByte *f1, meUByte *f2)
{
    if((f1==NULL) || (f2==NULL))
        return 1 ;
#ifdef _INSENSE_CASE
    return meStricmp(f1,f2) ;
#else
    return meStrcmp(f1,f2) ;
#endif
}

/* Search the directory and subdirectories for MicroEmacs macro directories */
int
mePathAddSearchPath(int index, meUByte *path_name, meUByte *path_base, int *gotUserPath)
{
    /* Common sub-directories of JASSPAs MicroEmacs */
    static meUByte *subdirs[] =
    {
        (meUByte *) "",                 /* Include the root directory first */
        (meUByte *) "company",          /* Company wide files */
        (meUByte *) "macros",           /* Standard distribution macros */
        (meUByte *) "spelling",         /* Spelling dictionaries */
        NULL
    } ;
    meUByte *pp, *ss, base_name[meBUF_SIZE_MAX] ;
    int ii, ll ;

    /* Iterate over all of the paths */
    while(*path_base != '\0')
    {
        /* Construct the base name */
        pp = base_name ;
        while((*pp = *path_base) != '\0')
        {
            path_base++ ;
            if (*pp == mePATH_CHAR)
            {
                *pp = '\0' ;
                break;
            }
            pp++;
        }

        /* Clean up any trailing directory characters */
        if((ll = meStrlen(base_name)) < _ROOT_DIR_LEN)
            continue;
        if(base_name[ll-1] == DIR_CHAR)
            ll-- ;

        /* check for base_name/$user-name first */
        if(meUserName != NULL)
        {
            base_name[ll] = DIR_CHAR ;
            meStrcpy(base_name+ll+1,meUserName) ;
            if(getFileStats(base_name,0,NULL,NULL) == meFILETYPE_DIRECTORY)
            {
                /* it exists, add it to the front if we haven't got a user
                 * path yet or to the end otherwise */
                ii = meStrlen(base_name) ;
                if(*gotUserPath <= 0)
                {
                    *gotUserPath = 1 ;
                    if(index)
                    {
                        base_name[ii++] = mePATH_CHAR ;
                        meStrcpy(base_name+ii,path_name) ;
                    }
                    meStrcpy(path_name,base_name) ;
                }
                else
                {
                    if(index)
                        path_name[index++] = mePATH_CHAR ;
                    meStrcpy(path_name+index,base_name) ;
                }
                index += ii ;
            }
        }

        /* Append the search paths if necessary. We construct the standard
         * JASSPA MicroEmacs paths and then test for the existance of the
         * directory. If the directory exists then we add it to the search
         * path. We do not add any directories to the search path that do not
         * exist. */
        for(ii=0; (ss = subdirs[ii]) != NULL; ii++)
        {
            if(*ss != '\0')
            {
                base_name[ll] = DIR_CHAR ;
                meStrcpy(base_name+ll+1,ss) ;
            }
            else
                base_name[ll] = '\0' ;

            /* Test the directory for existance, if it does not exist
             * then do not add it as we do not want to search any
             * directory unecessarily. */
            if(getFileStats(base_name,0,NULL,NULL) == meFILETYPE_DIRECTORY)
            {
                /* it exists, add it */
                if(index)
                    path_name[index++] = mePATH_CHAR ;
                meStrcpy(path_name+index,base_name) ;
                index += meStrlen(base_name) ;
            }
            else if(*ss == '\0')
                /* if the root dir does not exist there's no point continuing */
                break;
        }
    }
    return index;
}

/* Look up the existance of a file along the normal or PATH
 * environment variable. Look first in the HOME directory if
 * asked and possible
 */

int
fileLookup(meUByte *fname, meUByte *ext, meUByte flags, meUByte *outName)
{
    register meUByte *path;  /* environmental PATH variable */
    register meUByte *sp;    /* pointer into path spec */
    register int   ii;     /* index */
    meUByte nname[meBUF_SIZE_MAX] ;
    meUByte buf[meBUF_SIZE_MAX] ;

    if(ext != NULL)
    {
        if((sp=meStrrchr(fname,DIR_CHAR)) == NULL)
            sp = fname ;
        /* Jon 990608 - If the extension is the same then we do not
         * concatinate it, otherwise we do !!. Compare the complete extension
         * with the end of the string, allowing double extensions to be
         * handled properly. */
        if (((ii = meStrlen (sp) - meStrlen(ext)) < 0) ||
#ifdef _INSENSE_CASE
            meStricmp(&sp[ii], ext)
#else
            meStrcmp(&sp[ii], ext)
#endif
            )
        {
            meStrcpy(nname,fname) ;
            meStrcat(nname,ext);
            fname = nname ;
        }
    }
    if(flags & meFL_CHECKDOT)
    {
        fileNameCorrect(fname,outName,NULL) ;
#ifdef _UNIX
        if(flags & meFL_EXEC)
        {
            if(!meTestExec(outName) &&
               (getFileStats(outName,0,NULL,NULL) == meFILETYPE_REGULAR))
            return 1 ;
        }
        else
#endif
            if(!meTestExist(outName))
                return 1 ;
    }
    if(flags & meFL_USEPATH)
        path = meGetenv("PATH") ;
    else if(flags & meFL_USESRCHPATH)
        path = searchPath ;
    else
        return 0 ;
    while((path != NULL) && (*path != '\0'))
    {
        /* build next possible file spec */
        sp = path ;
        if((path = meStrchr(path,mePATH_CHAR)) != NULL)
        {
            ii = path++ - sp ;
            meStrncpy(buf,sp,ii) ;
        }
        else
        {
            meStrcpy(buf,sp) ;
            ii = meStrlen(buf) ;
        }
        /* Check for zero length path */
        if (ii <= 0)
            continue;
        /* Add a directory separator if missing */
        if (buf[ii-1] != DIR_CHAR)
            buf[ii++] = DIR_CHAR ;
        meStrcpy(buf+ii,fname) ;               /* Concatinate the name */

        /* and try it out */
        fileNameCorrect(buf,outName,NULL) ;
#if MEOPT_BINFS
        /* Try to determine the file type. */
        if (bfs_type (bfsdev, (char *)(outName+5)) == BFS_TYPE_FILE)
            return 1;
#endif
#ifdef _UNIX
        if(flags & meFL_EXEC)
        {
            if(!meTestExec(outName) &&
               (getFileStats(outName,0,NULL,NULL) == meFILETYPE_REGULAR))
            return 1 ;
        }
        else
#endif
            if(!meTestExist(outName))
                return 1 ;
    }
    return 0 ; /* no such luck */
}

int
executableLookup(meUByte *fname, meUByte *outName)
{
#if (defined _WIN32) || (defined _DOS)
    meUByte *ss ;
    int ll=meStrlen(fname) ;
    int ii ;

    if(ll > 4)
    {
        ss = fname+ll-4 ;
        ii = noExecExtensions ;
        while(--ii >= 0)
#ifdef _INSENSE_CASE
            if(!meStrnicmp(ss,execExtensions[ii],4))
#else
            if(!meStrncmp(ss,execExtensions[ii],4))
#endif
            {
                if(fileLookup(fname,NULL,meFL_CHECKDOT|meFL_USEPATH,outName))
                    return ii+1 ;
                return 0 ;
            }
    }
    ii=noExecExtensions ;
    while(--ii >= 0)
        if(fileLookup(fname,execExtensions[ii],meFL_CHECKDOT|meFL_USEPATH,outName))
            return ii+1 ;
#endif
#ifdef _UNIX
    meUByte flags ;

    flags = (meStrchr(fname,DIR_CHAR) != NULL) ? meFL_CHECKDOT|meFL_EXEC:meFL_USEPATH|meFL_EXEC ;
    if(fileLookup(fname,NULL,flags,outName))
        return 1 ;
#endif
    return 0 ;
}

int
bufferOutOfDate(meBuffer *bp)
{
    meStat stats ;
    if((bp->fileName != NULL) && getFileStats(bp->fileName,0,&stats,NULL) &&
       meFiletimeIsModified(stats.stmtime,bp->stats.stmtime))
        return meTRUE ;
    return meFALSE ;
}

/* get current working directory
*/
meUByte *
gwd(meUByte drive)
{
    /*
     * Get the current working directory into a static area and return
     * a pointer to it.
     *
     * This routine accounts for the differences between BSD getwd() and
     * System V getcwd().
     *
     * Return NULL on error or if we dont have a routine for the current
     * system (true for VMS, MS-DOS, etc etc until someone writes them).
     */
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)

    meUByte dir[meBUF_SIZE_MAX] ;
    register int ll ;

#ifdef _UNIX
    meGetcwd(dir,meBUF_SIZE_MAX-2) ;
#endif  /* _UNIX */

#ifdef _WIN32
    /* Change drive to the destination drive.
     * remeber so that we can get back. Note we
     * do no test for fails on the drive. We will
     * pick up the current working directory if
     * it fails. */
    if (drive != 0)
    {
        int curDrive;

        curDrive = _getdrive ();
        drive = toLower (drive) - 'a' + 1;
        if ((drive == curDrive) || (_chdrive (drive) != 0))
            drive = 0;             /* Failed drive change or same drive */
        else
            drive = curDrive;      /* Save to restore */
    }

    /* Pick up the directory information */
    GetCurrentDirectory (meBUF_SIZE_MAX, dir);
    if (meStrlen(dir) > 2)
    {
        meUByte *p;                   /* Local character pointer */

        /* convert all '\\' to '/' */
        p = dir+2 ;
        while((p=meStrchr(p,'\\')) != NULL)    /* got a '\\', -> '/' */
            *p++ = DIR_CHAR ;
    }
    /* change the drive back - dont care if it fails cos theres nothing we can do! */
    if (drive != 0)
        _chdrive (drive) ;
#endif /* _WIN32 */

#ifdef _DOS
#ifdef __DJGPP2__
    unsigned int curDrive=0, newDrive, availDrives ;

    if(drive != 0)
    {
        _dos_getdrive(&curDrive) ;
        newDrive = toLower(drive) - 'a' + 1 ;
        if(newDrive == curDrive)
            _dos_setdrive(newDrive,&availDrives) ;
        else
            /* already current drive */
            curDrive = 0;
    }

    if(getcwd(dir,meBUF_SIZE_MAX) == NULL)
        dir[0] = '\0' ;
    if(curDrive != 0)
        /* change the drive back */
        _dos_setdrive(curDrive,&availDrives) ;
    fileNameConvertDirChar(dir) ;
    makestrlow(dir) ;
#else
    union REGS reg ;                /* cpu register for use of DOS calls */

    if(drive == 0)
    {
        reg.h.ah = DOSI_GETDRV ;
        intdos (&reg, &reg);
        drive = reg.h.al + 'a' ;
    }
    else if(drive < 'a')
        drive += 'a' - 'A' ;
    dir[0] = drive ;
    dir[1] = ':' ;
    dir[2] = DIR_CHAR ;

    reg.h.ah = DOSI_GETDIR ;
    reg.h.dl = drive - 'a' + 1 ;
    reg.x.si = (unsigned long) (dir+3) ;
    (void) intdos(&reg, &reg);
#endif
#endif  /* _DOS */

    if((ll = meStrlen(dir)) > 0)
    {
        if(dir[ll-1] != DIR_CHAR)
        {
            dir[ll] = DIR_CHAR ;
            dir[ll+1] = '\0' ;
        }
        return meStrdup(dir) ;
    }

#endif  /* UNKNOWN */
    /*
    ** An unknown system to me - dont know what to do here.
    */
    return NULL ;

}

meUByte *
getFileBaseName(meUByte *fname)
{
    meUByte cc, *p, *p1 ;
    p = p1 = fname ;
    while((cc=*p1++) != '\0')
    {
        if((cc == DIR_CHAR) && (*p1 != '\0'))
            p = p1 ;
    }
    return p ;
}

void
getFilePath(meUByte *fname, meUByte *path)
{
    if(fname != NULL)
    {
        meUByte *ss ;
        meStrcpy(path,fname) ;
        if((ss=meStrrchr(path,DIR_CHAR)) == NULL)
            path[0] = '\0' ;
        else
            ss[1] = '\0' ;
    }
    else
        meStrcpy(path,curdir) ;
}

int
inputFileName(meUByte *prompt, meUByte *fn, int corFlag)
{
    meUByte tmp[meBUF_SIZE_MAX], *buf ;
    int s, ll ;

    buf = (corFlag) ? tmp:fn ;

    getFilePath(frameCur->bufferCur->fileName,buf) ;
    s = meGetString(prompt,(MLFILECASE|MLNORESET|MLMACNORT), 0, buf, meBUF_SIZE_MAX) ;
    if(s > 0)
    {
        if(((ll = meStrlen(buf)) > 0) && (buf[ll-1] == '.') &&
           ((ll == 1) || (buf[ll-2] == '/') || (buf[ll-2] == '\\') ||
            ((buf[ll-2] == '.') &&
             ((ll == 2) || (buf[ll-3] == '/') || (buf[ll-3] == '\\')))))
        {
            buf[ll] = '/' ;
            buf[ll+1] = '\0' ;
        }
        if(corFlag)
            fileNameCorrect(tmp,fn,NULL) ;
    }
    return s ;
}

#if MEOPT_CRYPT
int
resetkey(meBuffer *bp) /* reset the encryption key if needed */
{
    /* if we are in crypt mode */
    if(meModeTest(bp->mode,MDCRYPT))
    {
        if((bp->cryptKey == NULL) &&
           (setBufferCryptKey(bp,NULL) <= 0))
            return meFALSE ;

        /* check crypt is still enabled (user may have removed the key) and set up the key to be used! */
        if(meModeTest(bp->mode,MDCRYPT))
        {
            meCrypt(NULL, 0);
            meCrypt(bp->cryptKey, meStrlen(bp->cryptKey));
        }
    }
    return meTRUE ;
}
#endif

#if (defined _WIN32) || (defined _DOS)
int
meTestExecutable(meUByte *fileName)
{
    int ii ;
    if((ii = strlen(fileName)) > 4)
    {
        meUByte *ee ;
        ee = fileName+ii-4 ;
        ii = noExecExtensions ;
        while(--ii >= 0)
#ifdef _INSENSE_CASE
            if(!meStrnicmp(ee,execExtensions[ii],4))
#else
            if(!meStrncmp(ee,execExtensions[ii],4))
#endif
                return 1 ;
    }
    return 0 ;
}
#endif

#if MEOPT_DIRLIST
#define meFINDFILESINGLE_GFS_OPT 1
#define FILENODE_ATTRIBLEN 4

typedef struct FILENODE {
    struct FILENODE *next;              /* Next node in the list */
    meUByte  *fname;                        /* Name of the file */
    meUByte  *lname;                        /* Linkname */
    meUByte   attrib[FILENODE_ATTRIBLEN] ;
    meUInt    sizeHigh ;
    meUInt    sizeLow ;
#ifdef _UNIX
    time_t  mtime ;
#endif
#ifdef _DOS
    time_t  mtime ;
#endif
#ifdef _WIN32
    FILETIME mtime ;
#endif
} FILENODE;

FILENODE *curHead ;

static FILENODE *
addFileNode(FILENODE *cfh, FILENODE *cf)
{
    FILENODE *pf, *nf ;
    if(cfh == NULL)
    {
        cf->next = NULL ;
        return cf ;
    }
    pf = NULL ;
    nf = cfh ;
    if(cf->attrib[0] != 'd')
    {
        /* skip all the directories */
        while((nf != NULL) && (nf->attrib[0] == 'd'))
        {
            pf = nf ;
            nf = nf->next ;
        }
        /* now find the correct alpha place */
        while((nf != NULL) && (meStrcmp(nf->fname,cf->fname) < 0))
        {
            pf = nf ;
            nf = nf->next ;
        }
    }
    else
    {
        /* now find the correct alpha place */
        while((nf != NULL) && (nf->attrib[0] == 'd') &&
              (meStrcmp(nf->fname,cf->fname) < 0))
        {
            pf = nf ;
            nf = nf->next ;
        }
    }
    cf->next = nf ;
    if(pf == NULL)
        return cf ;
    pf->next = cf ;
    return cfh ;
}

static FILENODE *
getDirectoryInfo(meUByte *fname)
{
    meUByte bfname[meBUF_SIZE_MAX];                 /* Working file name buffer */
    meUByte *fn;
    FILENODE *curFile ;
    FILENODE *cfh=NULL ;
    int noFiles=0;                        /* No files count */

#if MEOPT_BINFS
    if (isBfsFile (fname))
    {
        bfsdir_t dirp;
        bfsdirent_t *dp;
        bfsstat_t sbuf;
        meUByte *ff;

        /* Render the list of files. */
        curHead = NULL ;
        meStrcpy(bfname,fname) ;
        fn = bfname+meStrlen(bfname) ;

        if((dirp = bfs_opendir(bfsdev, (char *)fname+5)) != NULL)
        {
            while((dp = bfs_readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                    return NULL ;
                curFile = &(curHead[noFiles++]) ;
                if((ff = malloc ((dp->len + 1) * sizeof (char))) == NULL)
                    return NULL ;
                meStrncpy(ff,(meUByte *)dp->name,dp->len);
                ff[dp->len] = '\0';
                curFile->lname = NULL ;
                curFile->fname = ff ;
                meStrcpy(fn,ff) ;
                if (bfs_stat(bfsdev,(char *)bfname+5,&sbuf) != 0)
                {
                    sbuf.st_mode = BFS_TYPE_DIR;
                    sbuf.st_size = 0;
                }
                /* construct attribute string */
                curFile->attrib[0] = (sbuf.st_mode == BFS_TYPE_DIR) ? 'd' : '-' ;
                curFile->attrib[1] = 'r';
                curFile->attrib[2] = '-' ;
                curFile->attrib[3] = (sbuf.st_mode == BFS_TYPE_DIR) ? 'x' : '-' ;
                curFile->sizeHigh = 0 ;
                curFile->sizeLow = (meUInt) sbuf.st_size ;
                curFile->mtime = sbuf.st_utc_mtime;
#ifdef _DOS
                curFile->mtime = 0;
#endif
            }
            bfs_closedir(dirp) ;
        }
    }
    else
#endif
    {
#ifdef _UNIX
    DIR    *dirp ;
#if (defined _DIRENT)
    struct  dirent *dp ;
#else
    struct  direct *dp ;
#endif
    struct stat sbuf ;
    meUByte *ff;
    int llen;

    /* Render the list of files. */
    curHead = NULL ;
    meStrcpy(bfname,fname) ;
    fn = bfname+meStrlen(bfname) ;

    if((dirp = opendir((char *)fname)) != NULL)
    {
        while((dp = readdir(dirp)) != NULL)
        {
            if(((noFiles & 0x0f) == 0) &&
               ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                return NULL ;
            curFile = &(curHead[noFiles++]) ;
            if((ff = meStrdup((meUByte *) dp->d_name)) == NULL)
                return NULL ;
            curFile->lname = NULL ;
            curFile->fname = ff ;
            meStrcpy(fn,dp->d_name) ;
            stat((char *)bfname,&sbuf) ;
            if(sbuf.st_uid != meUid)
            {
                /* remove the user modes */
                sbuf.st_mode &= ~00700 ;
                if(sbuf.st_gid != meGid)
                    /* replace with group */
                    sbuf.st_mode |= ((sbuf.st_mode & 00070) << 3)  ;
                else
                    /* replace with other */
                    sbuf.st_mode |= ((sbuf.st_mode & 00007) << 6)  ;
            }
            {
                /* Check if its a symbolic link */
                meUByte link[1024];
                if((llen=readlink((char *)bfname,(char *)link,1024)) > 0)
                {
                    link[llen] = '\0' ;
                    curFile->lname = meStrdup(link) ;
                }
            }
            /* construct attribute string */
            if(S_ISREG(sbuf.st_mode))
                curFile->attrib[0] = '-' ;
            else if(S_ISDIR(sbuf.st_mode))
                curFile->attrib[0] = 'd' ;
            else
                curFile->attrib[0] = 's' ;
            curFile->attrib[1] = (statTestRead(sbuf))  ? 'r' : '-' ;
            curFile->attrib[2] = (statTestWrite(sbuf)) ? 'w' : '-' ;
            curFile->attrib[3] = (statTestExec(sbuf))  ? 'x' : '-' ;
#ifdef _LARGEFILE_SOURCE
            curFile->sizeHigh = (meUInt) (sbuf.st_size >> 32) ;
            curFile->sizeLow = (meUInt) sbuf.st_size ;
#else
            curFile->sizeHigh = 0 ;
            curFile->sizeLow = (meUInt) sbuf.st_size ;
#endif
            curFile->mtime = sbuf.st_mtime ;
        }
        closedir(dirp) ;
    }
#endif
#ifdef _DOS
    static meUByte *searchString = "*.*";
    struct ffblk fblk ;
    meUByte *ff ;
    int done ;

    /* Render the list of files. */
    curHead = NULL ;
    meStrcpy(bfname,fname) ;
    fn = bfname+meStrlen(bfname) ;
    meStrcpy(fn,searchString) ;
    done = findfirst(bfname,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
    while(!done)
    {
        if(((noFiles & 0x0f) == 0) &&
           ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
            return NULL ;
        curFile = &(curHead[noFiles++]) ;
        if((ff = meStrdup(fblk.ff_name)) == NULL)
            return NULL ;
        makestrlow(ff) ;
        curFile->fname = ff ;
        curFile->lname = NULL ;
        curFile->sizeHigh = 0 ;
        curFile->sizeLow  = (meUInt) fblk.ff_fsize ;
        curFile->mtime = (((meUInt) fblk.ff_ftime) & 0x0ffff) | (((meUInt) fblk.ff_fdate) << 16) ;
        /* construct attribute string */
        if(fblk.ff_attrib & FA_DIREC)
            meStrncpy(curFile->attrib,"drwx",4) ;
        else
        {
            curFile->attrib[0] = '-' ;
            curFile->attrib[1] = 'r' ;
            curFile->attrib[2] = (fblk.ff_attrib & FA_RDONLY) ? '-' : 'w' ;
            curFile->attrib[3] = '-' ;
            if(meTestExecutable(ff))
                curFile->attrib[3] = 'x' ;
        }
        done = findnext(&fblk) ;
    }
#endif
#ifdef _WIN32
    static meUByte *searchString = "*.*";
    HANDLE          *fh ;
    WIN32_FIND_DATA  fd ;
    meUByte           *ff ;

    /* Render the list of files. */
    curHead = NULL ;
    meStrcpy(bfname,fname) ;
    fn = bfname+meStrlen(bfname) ;
    meStrcpy(fn,searchString) ;
    if((fh = FindFirstFile(bfname,&fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(((noFiles & 0x0f) == 0) &&
               ((curHead = (FILENODE *) meRealloc(curHead,sizeof(FILENODE)*(noFiles+16))) == NULL))
                return NULL ;
            curFile = &(curHead[noFiles++]) ;
            if((ff = meStrdup(fd.cFileName)) == NULL)
                return NULL ;
            curFile->fname = ff ;
            curFile->lname = NULL ;
            curFile->mtime.dwLowDateTime  = fd.ftLastWriteTime.dwLowDateTime ;
            curFile->mtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
            /* construct attribute string */
            meStrncpy(curFile->attrib,"-rwx",4) ;
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                curFile->attrib[0] = 'd' ;
                /* On network drives the size is sometimes invalid. Clear
                 * it just to make sure that this is not the case */
                curFile->sizeHigh = 0;
                curFile->sizeLow = 0;
            }
            else
            {
                if(fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                    curFile->attrib[2] =  '-' ;
                if(!meTestExecutable(ff))
                    curFile->attrib[3] = '-' ;
                curFile->sizeHigh = (meUInt) fd.nFileSizeHigh ;
                curFile->sizeLow = (meUInt) fd.nFileSizeLow ;
            }
        } while (FindNextFile(fh, &fd));
        FindClose(fh);
    }
#endif
    }
    curFile = curHead ;
    /* Build a profile of the current directory. */
    while(--noFiles >= 0)
    {
        cfh = addFileNode(cfh,curFile) ;
        curFile++ ;
    }
    return cfh ;
}

static int
readDirectory(meUByte *fname, meBuffer *bp, meLine *blp, meUInt flags)
{
    FILENODE *fnode, *fn ;
#ifdef _UNIX
    struct tm *tmp;
#endif
#ifdef _WIN32
    SYSTEMTIME tmp;
    FILETIME ftmp ;
#endif
    meUByte buf[meBUF_SIZE_MAX];                  /* Working line buffer */
    meUInt totSizeHigh=0, totSizeLow=0, ui ;
    int len, dirs=0, files=0 ;

    if((fnode=getDirectoryInfo(fname)) == NULL)
        return meABORT ;
    meStrcpy(buf,"Directory listing of: ") ;
    /* use the buffer's filename in preference to the given fname as the
     * buffer's filename is what the user asked for, fname may be what the
     * symbolic link points to */
    meStrcat(buf,(bp->fileName == NULL) ? fname:bp->fileName) ;
    bp->lineCount += addLine(blp,buf) ;
    fn = fnode ;
    while(fn != NULL)
    {
        totSizeHigh += fn->sizeHigh ;
        ui = totSizeLow + fn->sizeLow ;
        if((ui >> 16) < ((totSizeLow >> 16) + (fn->sizeLow >> 16)))
            totSizeHigh++ ;
        totSizeLow = ui ;
        if(fn->attrib[0] == 'd')
            dirs++ ;
        else
            files++ ;
        fn = fn->next ;
    }
    buf[0] = ' ' ;
    buf[1] = ' ' ;
    len = 2 ;
    if (totSizeHigh > 0)
    {
        ui = (totSizeHigh << 12) | (totSizeLow >> 20) ;
        len += sprintf((char *)buf+len, "%7dM ",ui) ;
    }
    else if (totSizeLow > 9999999)
        len += sprintf((char *)buf+len, "%7dK ",totSizeLow >> 10) ;
    else
        len += sprintf((char *)buf+len, "%7d  ",totSizeLow) ;
    sprintf((char *)buf+len,"used in %d files and %d dirs\n",files,dirs) ;
    bp->lineCount += addLine(blp,buf) ;
    while(fnode != NULL)
    {
        len = 0;                /* Reset to the start of the line */
        buf[len++] = ' ';
        meStrncpy(buf+len,fnode->attrib,FILENODE_ATTRIBLEN) ;
        len += FILENODE_ATTRIBLEN ;
        buf[len++] = ' ' ;
        /* Add the file statistics */
        if(fnode->sizeHigh > 0)
        {
            ui = (fnode->sizeHigh << 12) | (fnode->sizeLow >> 20) ;
            len += sprintf((char *)buf+len, "%7dM ",ui) ;
        }
        else if (fnode->sizeLow > 9999999)
            len += sprintf((char *)buf+len, "%7dK ", fnode->sizeLow >> 10);
        else
            len += sprintf((char *)buf+len, "%7d  ", fnode->sizeLow);

#ifdef _UNIX
        if ((tmp = localtime(&fnode->mtime)) != NULL)
            len += sprintf((char *)buf+len, "%4d/%02d/%02d %02d:%02d:%02d ",
                           tmp->tm_year+1900, tmp->tm_mon+1, tmp->tm_mday,
                           tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

        else
#endif
#ifdef _DOS
        if((fnode->mtime & 0x0ffff) != 0x7fff)
            len += sprintf(buf+len, "%4d/%02d/%02d %02d:%02d:%02d ",
                           (int) ((fnode->mtime >> 25) & 0x007f)+1980,
                           (int) ((fnode->mtime >> 21) & 0x000f),
                           (int) ((fnode->mtime >> 16) & 0x001f),
                           (int) ((fnode->mtime >> 11) & 0x001f),
                           (int) ((fnode->mtime >>  5) & 0x003f),
                           (int) ((fnode->mtime & 0x001f)  << 1)) ;
        else
#endif
#ifdef _WIN32
        if(FileTimeToLocalFileTime(&fnode->mtime,&ftmp) &&
           FileTimeToSystemTime(&ftmp,&tmp))
            len += sprintf(buf+len, "%4d/%02d/%02d %02d:%02d:%02d ",
                           tmp.wYear,tmp.wMonth,tmp.wDay,tmp.wHour,tmp.wMinute,tmp.wSecond) ;
        else
#endif
            len += sprintf((char *)buf+len, "--/--/-- --:--:-- ") ;

        len += sprintf ((char *)buf+len, "%s", fnode->fname);
        if(fnode->attrib[0] == 'd')
            buf[len++] = '/' ;

        if(fnode->lname != NULL)
        {
            sprintf((char *)buf+len, " -> %s", fnode->lname);
            free(fnode->lname) ;
        }
        else
            buf[len] = '\0' ;

        bp->lineCount += addLine(blp,buf) ;
        free(fnode->fname) ;
        fnode = fnode->next ;
    }
    free(curHead) ;
    if((flags & meRWFLAG_PRESRVFMOD) == 0)
    {
        bp->fileFlag = meBFFLAG_DIR ;
        meModeSet(bp->mode,MDVIEW) ;
        meModeClear(bp->mode,MDAUTOSV) ;
        meModeClear(bp->mode,MDUNDO) ;
    }
    return meTRUE ;
}

#else
#define meFINDFILESINGLE_GFS_OPT 3
#endif

/*
 * Read file "fname" into the current
 * buffer, blowing away any text found there. Called
 * by both the read and find commands. Return the final
 * status of the read. Also called by the mainline,
 * to read in a file specified on the command line as
 * an argument. If the filename ends in a ".c", CMODE is
 * set for the current buffer.
 */
/* Note for Dynamic file names
 * readin sets the buffer file name to point to fname (freeing old if not
 * NULL and bp->fileName != fname) so fname should be dynamically allocated
 * or the file name reset after readin.
 */
/* bp        buffer to load into */
/* fname     name of file to read */
/* lockfl    check for file locks? */
/* frompipe  input coming from pipe. Dont open file */
int
readin(register meBuffer *bp, meUByte *fname)
{
    int   ss=meABORT ;
    meUByte lfn[meBUF_SIZE_MAX], afn[meBUF_SIZE_MAX], *fn=fname ;

#if MEOPT_CRYPT
    if(resetkey(bp) <= 0)
        return meABORT ;
#endif
    if(bp->fileName != fname)
    {
        meNullFree(bp->fileName) ;
        bp->fileName = fname ;
    }
    if(fn != NULL)
    {
#if MEOPT_EXTENDED
        meUInt ui ;
#endif
        meStat stats ;
        int ft ;
        if((ft=getFileStats(fn,1,&(bp->stats),lfn)) == meFILETYPE_HTTP)
            meModeSet(bp->mode,MDVIEW) ;
        else if(ft != meFILETYPE_FTP)
        {
            if(ft == meFILETYPE_NASTY)
                goto error_end ;
            if(lfn[0] != '\0')
                /* something was found, don't want to do RCS on this,
                 * the link may be dangling which returns 3 */
                fn = lfn ;
#if MEOPT_RCS
            else if((ft == meFILETYPE_NOTEXIST) &&       /* file not found */
                    (rcsFilePresent(fn) > 0))
            {
                if(doRcsCommand(fn,rcsCoStr) <= 0)
                    goto error_end ;
                if((ft=getFileStats(fn,1,&(bp->stats),lfn)) == meFILETYPE_NASTY)
                    goto error_end ;
            }
#endif
            if((autoTime > 0) && !createBackupName(afn,fn,'#',0) &&
               (getFileStats(afn,0,&stats,NULL) == meFILETYPE_REGULAR) &&
               meFiletimeIsModified(stats.stmtime,bp->stats.stmtime))
            {
                meUByte prompt[200] ;
                meUByte *tt ;
                tt = getFileBaseName(afn) ;
                sprintf((char *)prompt,"Recover newer %s",tt) ;
                if(mlyesno(prompt) > 0)
                {
                    fn = afn ;
                    ft = meFILETYPE_REGULAR ;
                    memcpy(&bp->stats,&stats,sizeof(meStat)) ;
                }
            }
            if(ft == meFILETYPE_DIRECTORY)
            {
#if MEOPT_DIRLIST
                if(readDirectory(fn,bp,bp->baseLine,0) <= 0)
                    goto error_end ;
                ss = meTRUE ;
                goto newfile_end;
#else
                goto error_end ;
#endif
            }
            if(ft == meFILETYPE_NOTEXIST)
            {   /* File not found.      */
                meUByte dirbuf [meBUF_SIZE_MAX];

                /* See if we can write to the directory. */
                getFilePath (fn, dirbuf);
                if((getFileStats(dirbuf,0,NULL,NULL) != meFILETYPE_DIRECTORY)
#ifdef _UNIX
                   || meTestWrite(dirbuf)
#endif
                   )
                {
                    /* READ ONLY DIR */
                    mlwrite(MWPAUSE,(meUByte *)"%s: %s", dirbuf, strerror(errno));
                    /* Zap the filename - it is invalid.
                       We only want a buffer */
                    mlwrite (0,(meUByte *)"[New buffer %s]", getFileBaseName(fname));
                    meNullFree(bp->fileName) ;
                    bp->fileName = NULL;
                    ss = meABORT;
                }
                else
                {
                    mlwrite(0,(meUByte *)"[New file %s]", fname);
                    ss = meTRUE ;
                }
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }

            /* Make sure that we can read the file. If we are root then
             * we do not test the 'stat' bits. Root is allowed to read
             * anything */
            if ((
#if MEOPT_BINFS
                (!isBfsFile (fname)) &&
#endif
                (meTestRead (fn))) ||
                (
#ifdef _UNIX
                 (meUid != 0) &&
#endif
                 (!meStatTestRead(bp->stats))))
            {
                /* We are not allowed to read the file */
#if ((defined _UNIX) || (defined _DOS))
                mlwrite(MWABORT,(meUByte *)"[%s: %s]", fn, strerror(errno)) ;
#else
                mlwrite(MWABORT,"[cannot read: %s]", fn) ;
#endif
                /* Zap the filename - it is invalid.
                   We only want a buffer */
                meNullFree(bp->fileName) ;
                bp->fileName = NULL;
                /* its not linked to a file so change the flag */
                bp->intFlag &= ~BIFFILE ;
                goto newfile_end;
            }
#if MEOPT_EXTENDED
            if((fileSizePrompt > 0) && ((ui=(bp->stats.stsizeHigh << 12) | (bp->stats.stsizeLow >> 20)) > fileSizePrompt))
            {
                meUByte prompt[200] ;
                meUByte *tt ;
                tt = getFileBaseName(fn) ;
                sprintf((char *)prompt,"File %s is %dMb, continue",tt,ui) ;
                if(mlyesno(prompt) <= 0)
                    goto error_end ;
            }
#endif
#ifdef _WIN32
            if(!meStatTestSystem(bp->stats))
            {
                /* if windows system file read in a readonly */
                meModeSet(bp->mode,MDVIEW) ;
                mlwrite(MWCURSOR|MWCLEXEC,"[Reading %s (system file)]", fn);
            }
            else
#endif
            /* Depending on whether we have write access set the buffer into
             * view mode. Again root gets the privelidge of being able to
             * write if possible. */
            if ((meTestWrite (fn)) ||
                (
#ifdef _UNIX
                 (meUid != 0) &&
#endif
                 (!meStatTestWrite(bp->stats))))
                meModeSet(bp->mode,MDVIEW) ;
        }
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[Reading %s%s]",fn,
                meModeTest(bp->mode,MDVIEW) ? " (readonly)" : "");
    }
    ss = ffReadFile(fn,0,bp,bp->baseLine,0,0,0) ;

    /*
    ** Set up the modification time field of the buffer structure.
    */
    if(ss != meABORT)
    {
        mlwrite(MWCLEXEC,(meUByte *)"[Read %d line%s]",bp->lineCount,(bp->lineCount==1) ? "":"s") ;
        if(fn == afn)
            /* this is a recovered file so flag the buffer as changed */
            meModeSet(bp->mode,MDEDIT) ;
        ss = meTRUE ;
    }
    else
        meModeSet(bp->mode,MDVIEW) ;

newfile_end:

    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotLineNo = 0 ;
    bp->dotOffset = 0 ;

error_end:
    return ss ;
}

/*
 * Insert file "fname" into the current
 * buffer, Called by insert file command. Return the final
 * status of the read.
 */
/* must have the buffer line no. correct */
int
meBufferInsertFile(meBuffer *bp, meUByte *fname, meUInt flags,
                   meUInt uoffset, meUInt loffset, meInt length)
{
    register meWindow *wp ;
    register meLine   *lp ;
    register int     ss ;
    register long    nline ;

    meModeSet(bp->mode,MDEDIT) ;              /* we have changed	*/

#if MEOPT_CRYPT
    if(resetkey(bp) <= 0)
        return meFALSE ;
#endif
    if(!(flags & meRWFLAG_SILENT))
        mlwrite(MWCURSOR|MWCLEXEC,(meUByte *)"[Inserting file]");

    nline = bp->lineCount ;
    lp = bp->dotLine ;
#if MEOPT_DIRLIST
    if(flags & meRWFLAG_MKDIR)
    {
        /* slight miss-use of this flag to inform this function that a dir
         * listing is to be inserted */
        ss = readDirectory(fname,bp,lp,flags|meRWFLAG_INSERT) ;
    }
    else
#endif
        ss = ffReadFile(fname,flags|meRWFLAG_INSERT,bp,lp,uoffset,loffset,length) ;
    nline = bp->lineCount-nline ;

    if(ss != meABORT)
    {
        ss = meTRUE ;
        if(!(flags & meRWFLAG_SILENT))
           mlwrite(MWCLEXEC,(meUByte *)"[inserted %d line%s]",nline,(nline==1) ? "":"s") ;
    }
    meFrameLoopBegin() ;
    for (wp=loopFrame->windowList; wp!=NULL; wp=wp->next)
    {
        if (wp->buffer == bp)
        {
            if(wp->dotLineNo >= bp->dotLineNo)
                wp->dotLineNo += nline ;
            if(wp->markLineNo >= bp->dotLineNo)
                wp->markLineNo += nline ;
            wp->updateFlags |= WFMAIN|WFMOVEL ;
        }
    }
    meFrameLoopEnd() ;
#if MEOPT_UNDO
    if((bp == frameCur->bufferCur) && meModeTest(bp->mode,MDUNDO))
    {
        int ii = 0 ;
        frameCur->windowCur->dotOffset = 0 ;
        while(nline--)
        {
            lp = meLineGetPrev(lp) ;
            ii += meLineGetLength(lp) + 1 ;
        }
        meUndoAddInsChars(ii) ;
    }
#endif
    return ss ;
}

/*
 * Insert a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "insert a file into the current buffer" code.
 * Bound to "C-X C-I".
 */
int
insertFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX] ;
    meStat stats ;
    meUInt uoffset, loffset ;
    meInt length=0 ;
    int s, flags=0 ;

    if((s=inputFileName((meUByte *)"Insert file",fname,1)) <= 0)
        return s ;
    /* Allow existing or url files if not doing a partial insert */
    if(((s=getFileStats(fname,meFINDFILESINGLE_GFS_OPT,&stats,NULL)) != meFILETYPE_REGULAR) &&
       ((n & 4) || ((s != meFILETYPE_HTTP) && (s != meFILETYPE_FTP)
#if MEOPT_DIRLIST
                    && (s != meFILETYPE_DIRECTORY)
#endif
                    )))
        return meABORT ;
#if MEOPT_EXTENDED
    if((fileSizePrompt > 0) && ((n & 4) == 0) && ((uoffset=(stats.stsizeHigh << 12) | (stats.stsizeLow >> 20)) > fileSizePrompt))
    {
        meUByte prompt[200] ;
        meUByte *tt ;
        tt = getFileBaseName(fname) ;
        sprintf((char *)prompt,"File %s is %dMb, continue",tt,uoffset) ;
        if(mlyesno(prompt) <= 0)
            return meABORT ;
    }
#endif
#if MEOPT_DIRLIST
    if(s == meFILETYPE_DIRECTORY)
        flags |= meRWFLAG_MKDIR ;
#endif
    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;
    /* set mark to previous line so it doesnt get moved down */
    frameCur->windowCur->markLine = meLineGetPrev(frameCur->windowCur->dotLine) ;
    frameCur->windowCur->markOffset = 0 ;
    frameCur->windowCur->markLineNo = frameCur->windowCur->dotLineNo-1 ;

    /* store current line in buffer */
    frameCur->bufferCur->dotLine = frameCur->windowCur->dotLine ;
    frameCur->bufferCur->dotLineNo = frameCur->windowCur->dotLineNo ;   /* must have the line no. correct */

    if((n & 2) == 0)
        flags |= meRWFLAG_PRESRVFMOD ;
    if(n & 4)
    {
        meUByte arg[meSBUF_SIZE_MAX] ;

        if(meGetString((meUByte *)"UOffest",0,0,arg,meSBUF_SIZE_MAX) <= 0)
            return meABORT ;
        uoffset = meAtoi(arg) ;
        if(meGetString((meUByte *)"LOffest",0,0,arg,meSBUF_SIZE_MAX) <= 0)
            return meABORT ;
        loffset = meAtoi(arg) ;
        if((meGetString((meUByte *)"Length",0,0,arg,meSBUF_SIZE_MAX) <= 0) ||
           ((length = meAtoi(arg)) == 0))
            return meABORT ;
    }

    if(((s = meBufferInsertFile(frameCur->bufferCur,fname,flags,uoffset,loffset,length)) > 0) && (n & 2))
        getFileStats(fname,meFINDFILESINGLE_GFS_OPT,&(frameCur->bufferCur->stats),NULL) ;

    /* move the mark down 1 line into the correct place */
    frameCur->windowCur->markLine = meLineGetNext(frameCur->windowCur->markLine);
    frameCur->windowCur->markLineNo++ ;
    return s ;
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * I suppose that this information could be put in
 * a better place than a line of code.
 */
void
makename(meUByte *bname, meUByte *fname)
{
    register meUByte *cp1 ;
    register meUByte *cp2 ;
    int             i ;

    cp1 = getFileBaseName(fname) ;
    cp2 = bname ;
    while((*cp2++=*cp1++) != '\0')
        ;
    cp2-- ;
    /*
     * Now we've made our buffer name, check that it is unique.
     */
    for(i = 1 ; (bfind(bname, 0) != NULL) ; i++)
    {
        /*
         * There is another buffer with this name. Try sticking a
         * numeric "i" on the end of the buffer.
         */
        sprintf((char *)cp2, "<%d>", i);
    }
}

#define fileNameWild(fileName)                                                        \
((strchr((char *)fileName,'*') != NULL) || (strchr((char *)fileName,'?') != NULL) ||  \
 (strchr((char *)fileName,'[') != NULL))

static int
findFileSingle(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    meBuffer *bp ;
    int   gft ;
#if MEOPT_SOCKET
    int fnlen ;
#endif
#ifdef _UNIX
    meStat  stats ;
    gft = getFileStats(fname,meFINDFILESINGLE_GFS_OPT,&stats,NULL) ;
#else
    gft = getFileStats(fname,meFINDFILESINGLE_GFS_OPT,NULL,NULL) ;
#endif
    if((gft == meFILETYPE_NASTY) ||
#if (MEOPT_DIRLIST == 0)
       (gft == meFILETYPE_DIRECTORY) ||
#endif
       ((gft == meFILETYPE_NOTEXIST) && !(bflag & BFND_CREAT)))
        return 0 ;

    bflag |= BFND_CREAT ;

    /* if this is a directory then add the '/' */
    if(gft == meFILETYPE_DIRECTORY)
    {
        meUByte *ss=fname+meStrlen(fname) ;
        if(ss[-1] != DIR_CHAR)
        {
            ss[0] = DIR_CHAR ;
            ss[1] = '\0' ;
        }
    }
#if MEOPT_SOCKET
    if((gft != meFILETYPE_FTP) ||
       ((fnlen = meStrlen(fname)),(fname[fnlen-1] == DIR_CHAR)))
        fnlen = 0 ;
#endif
    for (bp=bheadp; bp!=NULL; bp=bp->next)
    {
        if((bp->fileName != NULL) && (bp->name != NULL) && (bp->name[0] != '*'))
        {
            if(
#ifdef _UNIX
               !fnamecmp(bp->fileName,fname) ||
               ((stats.stdev != (dev_t)(-1)) &&
                (bp->stats.stdev == stats.stdev) &&
                (bp->stats.stino == stats.stino) &&
                (bp->stats.stsizeHigh == stats.stsizeHigh) &&
                (bp->stats.stsizeLow == stats.stsizeLow)))
#else
               !fnamecmp(bp->fileName,fname))
#endif
                break ;
#if MEOPT_SOCKET
            /* at this point the type of an ftp file (i.e. reg of dir)
             * is unknown, the filename will be changed,but the comparison
             * of ftp: file names must allow for this */
            if(fnlen && !meStrncmp(bp->fileName,fname,fnlen) &&
               (bp->fileName[fnlen] == DIR_CHAR) && (bp->fileName[fnlen+1] == '\0'))
                break ;
#endif
        }
    }
    if(bp == NULL)
    {
        bp = bfind(fname,bflag);
        bp->fileName = meStrdup(fname) ;
        bp->intFlag |= BIFFILE ;
        if(gft == meFILETYPE_DIRECTORY)
        {
            /* if its a directory then add dir flag and remove binary */
            bp->fileFlag = meBFFLAG_DIR ;
            meModeClear(bp->mode,MDBINARY) ;
            meModeClear(bp->mode,MDRBIN) ;
        }
        bp->dotLineNo = lineno ;
        bp->dotOffset = colno ;
    }
    else if(!meModeTest(bp->mode,MDNACT))
        bp->intFlag |= BIFLOAD ;
    bp->histNo = bufHistNo ;
    return 1 ;
}

static void
fileMaskToRegex(meUByte *dfname, meUByte *sfname)
{
    meUByte *ss=sfname, *dd=dfname, cc ;

    while((cc = *ss++) != '\0')
    {
        switch(cc)
        {
        case '*':
            *dd++ = '.' ;
            *dd++ = '*' ;
            break ;
        case '?':
            *dd++ = '.' ;
            break ;
        case '[':
            *dd++ = '[' ;
            *dd++ = *ss++ ;
            while((cc = *ss++) != ']')
                if((*dd++ = cc) == '\0')
                    return ;
            *dd++ = ']' ;
            break ;
        case '\\':
        case '+':
        case '.':
        case '$':
        case '^':
            *dd++ = '\\' ;
        default:
            *dd++ = cc ;
        }
    }
    *dd = '\0' ;
}

int
findFileList(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    register int nofiles=0, ii ;
    meUByte fileName[meBUF_SIZE_MAX], *baseName, cc ;

    bufHistNo++ ;
    fileNameCorrect(fname,fileName,&baseName) ;

    if(!isUrlLink(fileName) && fileNameWild(baseName) &&
       ((isBfsFile(fileName) || meTestRead(fileName))))
    {
        /* if the base name has a wild card letter (i.e. *, ? '[')
         * and a file with that exact name does not exist then load
         * any files which match the wild card mask */
        meUByte mask[meBUF_SIZE_MAX] ;

        fileMaskToRegex(mask,baseName) ;
        cc = *baseName ;
        *baseName = '\0' ;
        getDirectoryList(fileName,&curDirList) ;
        for(ii=0 ; ii<curDirList.size ; ii++)
        {
            meUByte *ss = curDirList.list[ii] ;
#ifdef _INSENSE_CASE
            if(regexStrCmp(ss,mask,meRSTRCMP_ICASE|meRSTRCMP_WHOLE))
#else
            if(regexStrCmp(ss,mask,meRSTRCMP_WHOLE))
#endif
            {
                meStrcpy(baseName,ss) ;
                nofiles += findFileSingle(fileName,bflag,lineno,colno) ;
            }
        }
        *baseName = cc ;
    }
    if(nofiles == 0)
        nofiles += findFileSingle(fileName,bflag,lineno,colno) ;
    return nofiles ;
}

int
findSwapFileList(meUByte *fname, int bflag, meInt lineno, meUShort colno)
{
    meBuffer *bp ;
    int     ret ;

    bufHistNo += 2 ;
    if(!findFileList(fname,bflag,lineno,colno))
        return mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[Failed to find file %s]",fname);
    for(bp=bheadp ; bp->histNo!=bufHistNo ; bp=bp->next)
        ;
    bufHistNo -= 2 ;
    ret = swbuffer(frameCur->windowCur,bp) ;  /* make buffer BP current */
    bufHistNo++ ;
    return ret ;
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * fine in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 * Bound to C-X C-F.
 */
int
findFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX], prompt[16], *ss ;

    ss = prompt ;
    *ss++ = 'f' ;
    *ss++ = 'i' ;
    *ss++ = 'n' ;
    *ss++ = 'd' ;
    *ss++ = '-' ;
    if(n & BFND_BINARY)
        *ss++ = 'b' ;
    if(n & BFND_CRYPT)
        *ss++ = 'c' ;
    if(n & BFND_RBIN)
        *ss++ = 'r' ;
    *ss++ = 'f' ;
    *ss++ = 'i' ;
    *ss++ = 'l' ;
    *ss++ = 'e' ;
    *ss   = '\0' ;
    if(inputFileName(prompt,fname,0) <= 0)
        return meABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM ;
    return findSwapFileList(fname,n,0,0) ;
}

#if MEOPT_EXTENDED
/*
 * Find file into other window. This is trivial since all the hard stuff is
 * done by the routines windowSplitDepth() and filefind().
 *
 * The count is used to put the "found file" into the upper or lower window.
 *
 * Normally mapped to ^X4. (or C-X 4 if you prefer).
 */
int
nextWndFindFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */

    if(inputFileName((meUByte *)"Find file",fname,0) <= 0)
        return meABORT ;
    if(meWindowPopup(NULL,WPOP_MKCURR,NULL) == NULL)
        return meFALSE ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM ;
    return findSwapFileList(fname,n,0,0) ;
}
#endif

int
readFile(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */
    register int s;		/* status return */

    if(inputFileName((meUByte *)"Read file", fname,0) <= 0)
        return meABORT ;
    if(n & 0x20)
	frameCur->bufferCur->intFlag |= BIFBLOW ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM ;
    if((s=zotbuf(frameCur->bufferCur,clexec)) > 0)
        s = findSwapFileList(fname,n,0,0) ;
    return s ;
}

int
viewFile(int f, int n)	/* visit a file in VIEW mode */
{
    meUByte fname[meBUF_SIZE_MAX];	/* file user wishes to find */
    register int ss, vv;	/* status return */

    if (inputFileName((meUByte *)"View file", fname,0) <= 0)
        return meABORT ;
    n = (n & (BFND_CREAT|BFND_BINARY|BFND_CRYPT|BFND_RBIN|BFND_NOHOOK)) | BFND_MKNAM ;
    /* Add view mode globally so any new buffers will be created
     * with view mode */
    vv = meModeTest(globMode,MDVIEW) ;
    meModeSet(globMode,MDVIEW) ;
    ss = findSwapFileList(fname,n,0,0) ;
    /* if view mode was not set globally restore it */
    if(!vv)
        meModeClear(globMode,MDVIEW) ;
    return ss ;
}

#define meWRITECHECK_CHECK    0x001
#define meWRITECHECK_BUFFER   0x002
#define meWRITECHECK_NOPROMPT 0x004

/*
 * writeCheck
 * This performs some simple access checks to determine if we
 * can write the file.
 */
static int
writeCheck(meUByte *pathname, int flags, meStat *statp)
{
    meUByte dirbuf [meBUF_SIZE_MAX];
#if MEOPT_SOCKET
    if(isFtpLink(pathname))
        return meTRUE ;
#endif
    if(isUrlLink(pathname))
        return mlwrite (MWABORT,(meUByte *)"Cannot write to: %s",pathname);
    if(isBfsFile(pathname))
        return mlwrite (MWABORT,(meUByte *)"Cannot write to: %s",pathname);

    /* Quick test for read only. */
#ifdef _UNIX
    /* READ ONLY directory test
       See if we can write to the directory. */
    getFilePath(pathname,dirbuf) ;
    if (meTestWrite (dirbuf))
        return mlwrite (MWABORT,(meUByte *)"Read Only Directory: %s", dirbuf);
#endif
    /* See if there is an existing file */
    if((flags & meWRITECHECK_CHECK) == 0)       /* Validity check enabled ?? */
        return meTRUE;                          /* No - quit. */
    if(meTestExist(pathname))                   /* Does it exist ?? */
        return meTRUE;                          /* No - quit */
    if((statp != NULL) && !meStatTestWrite(*statp))
    {
        /* No - advised read only - see if the users wants to write */
        sprintf((char *)dirbuf, "%s is read only, overwrite", pathname);
        if((flags & meWRITECHECK_NOPROMPT) || (mlyesno(dirbuf) <= 0))
            return ctrlg(meFALSE,1);
    }
    return meTRUE;
}

static meUByte *
writeFileChecks(meUByte *dfname, meUByte *sfname, meUByte *lfname, int flags)
{
    register int s;
    meStat stats ;
    meUByte *fn ;

    if((sfname != NULL) &&
       (((s=getFileStats(dfname,0,NULL,NULL)) == meFILETYPE_DIRECTORY) ||
        ((s == meFILETYPE_FTP) && (dfname[meStrlen(dfname)-1] == DIR_CHAR))))
    {
        s = meStrlen(dfname) ;
        if(dfname[s-1] != DIR_CHAR)
            dfname[s++] = DIR_CHAR ;
        meStrcpy(dfname+s,getFileBaseName(sfname)) ;
    }
    if (((s=getFileStats(dfname,3,&stats,lfname)) != meFILETYPE_REGULAR) && (s != meFILETYPE_NOTEXIST)
#if MEOPT_SOCKET
        && (s != meFILETYPE_FTP)
#endif
        )
        return NULL ;
    fn = (lfname[0] == '\0') ? dfname:lfname ;
    if(flags & meWRITECHECK_CHECK)
    {
        meUByte prompt[meBUF_SIZE_MAX+48];

        /* Check for write-out filename problems */
        if(s == meFILETYPE_REGULAR)
        {
            sprintf((char *)prompt,"%s already exists, overwrite",fn) ;
            if((flags & meWRITECHECK_NOPROMPT) || (mlyesno(prompt) <= 0))
            {
                ctrlg(meFALSE,1);
                return NULL ;
            }
        }
        /* Quick check on the file write condition */
        if(writeCheck(fn,flags,&stats) <= 0)
            return NULL ;

        if(flags & meWRITECHECK_CHECK)
        {
            /*
             * Check to see if the new filename conflicts with the filenames for any
             * other buffer and produce a warning if so.
             */
            meBuffer *bp = bheadp ;

            while(bp != NULL)
            {
                if((bp != frameCur->bufferCur) &&
#ifdef _UNIX
                   (!fnamecmp(bp->fileName,fn) ||
                    ((stats.stdev != (dev_t)(-1)) &&
                     (bp->stats.stdev == stats.stdev) &&
                     (bp->stats.stino == stats.stino) &&
                     (bp->stats.stsizeHigh == stats.stsizeHigh) &&
                     (bp->stats.stsizeLow == stats.stsizeLow))))
#else
                   !fnamecmp(bp->fileName,fn))
#endif
                {
                    sprintf((char *)prompt, "Buffer %s is the same file, overwrite",bp->name) ;
                    if(mlyesno(prompt) <= 0)
                    {
                        ctrlg(meFALSE,1);
                        return NULL ;
                    }
                }
                bp = bp->next ;
            }
        }
    }
    return fn ;
}

#if MEOPT_EXTENDED

#define meFILEOP_CHECK     0x001
#define meFILEOP_BACKUP    0x002
#define meFILEOP_NOPROMPT  0x004
#define meFILEOP_PRESRVTS  0x008
#define meFILEOP_FTPCLOSE  0x010
#define meFILEOP_DELETE    0x020
#define meFILEOP_MOVE      0x040
#define meFILEOP_COPY      0x080
#define meFILEOP_MKDIR     0x100
#define meFILEOP_CHMOD     0x200
#define meFILEOP_TOUCH     0x400
#define meFILEOP_WC_MASK   0x005

int
fileOp(int f, int n)
{
    meUByte sfname[meBUF_SIZE_MAX], dfname[meBUF_SIZE_MAX], lfname[meBUF_SIZE_MAX], *fn=NULL ;
    int rr=meTRUE, dFlags=0, fileMask=-1 ;

    if((n & (meFILEOP_FTPCLOSE|meFILEOP_DELETE|meFILEOP_MOVE|meFILEOP_COPY|meFILEOP_MKDIR|meFILEOP_CHMOD|meFILEOP_TOUCH)) == 0)
        rr = mlwrite(MWABORT,(meUByte *)"[No operation set]") ;
    else if(n & meFILEOP_DELETE)
    {
        if (inputFileName((meUByte *)"Delete file", sfname,1) <= 0)
            rr = 0 ;
        else if (isHttpLink(sfname) || isBfsFile(sfname))
            rr = mlwrite(MWABORT,(meUByte *)"[Cannot delete %s]",sfname);
        else if((n & meFILEOP_CHECK) && !isFtpLink(sfname))
        {
            if(meTestExist(sfname))
            {
                mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s: No such file]",sfname);
                rr = -2 ;
            }
            else if(meTestWrite(sfname))
            {
                sprintf((char *)lfname, "%s is read only, delete",sfname) ;
                if((n & meFILEOP_NOPROMPT) || (mlyesno(lfname) <= 0))
                {
                    ctrlg(meFALSE,1);
                    rr = -8 ;
                }
            }
        }
        dFlags = meRWFLAG_DELETE ;
    }
    else if(n & (meFILEOP_MOVE|meFILEOP_COPY))
    {
        static meUByte prompt[]="Copy file" ;
        if(n & meFILEOP_MOVE)
        {
            prompt[0] = 'M' ;
            prompt[2] = 'v' ;
            prompt[3] = 'e' ;
            dFlags = meRWFLAG_DELETE ;
        }
        else
        {
            prompt[0] = 'C' ;
            prompt[2] = 'p' ;
            prompt[3] = 'y' ;
        }
        if((inputFileName(prompt, sfname,1) <= 0) ||
           (inputFileName((meUByte *)"To", dfname,1) <= 0))
            rr = 0 ;
        else if((fn=writeFileChecks(dfname,sfname,lfname,(n & meFILEOP_WC_MASK))) == NULL)
            rr = -6 ;
        else
            fileMask = meFileGetAttributes(sfname) ;
        dFlags |= meRWFLAG_NODIRLIST ;
        if(n & meFILEOP_PRESRVTS)
            dFlags |= meRWFLAG_PRESRVTS ;
    }
    else if(n & meFILEOP_MKDIR)
    {
        int s ;
        if (inputFileName((meUByte *)"Create dir", sfname,1) <= 0)
            rr = 0 ;
        /* check that nothing of that name currently exists */
        else if (((s=getFileStats(sfname,0,NULL,NULL)) != meFILETYPE_NOTEXIST)
#if MEOPT_SOCKET
            && (s != meFILETYPE_FTP)
#endif
                 )
        {
            mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s already exists]",sfname);
            rr = -7 ;
        }
        else
            dFlags = meRWFLAG_MKDIR ;
    }
    else if(n & (meFILEOP_CHMOD|meFILEOP_TOUCH))
    {
        int s ;
        if((inputFileName((meUByte *) ((n & meFILEOP_CHMOD) ? "Chmod":"Touch"), sfname,1) <= 0) ||
           (meGetString((meUByte *)"To",0,0,dfname,meBUF_SIZE_MAX) <= 0))
            rr = 0 ;
        /* check that nothing of that name currently exists */
        else if(((s=getFileStats(sfname,0,NULL,NULL)) != meFILETYPE_REGULAR) &&
                (s != meFILETYPE_DIRECTORY))
        {
            mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s not a file or directory]",sfname);
            rr = -6 ;
        }
        else if(n & meFILEOP_CHMOD)
        {
            meFileSetAttributes(sfname,meAtoi(dfname)) ;
            return meTRUE ;
        }
        else
        {
#ifdef _UNIX
            struct utimbuf fileTimes ;

            fileTimes.actime = fileTimes.modtime = time(NULL) + meAtoi(dfname) ;
            utime((char *) sfname,&fileTimes) ;
#endif
#ifdef _WIN32
            HANDLE fp ;
            FILETIME ft ;
            SYSTEMTIME st ;
            if((fp=CreateFile(sfname,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,NULL)) == INVALID_HANDLE_VALUE)
            {
                mlwrite(MWABORT|MWCLEXEC,(meUByte *)"[%s: Cannot access file]",sfname);
                rr = -6 ;
            }
            else
            {
                GetSystemTime(&st) ;
                SystemTimeToFileTime(&st,&ft) ;
                rr = meAtoi(dfname) ;
                if(rr != 0)
                {
                    ULARGE_INTEGER uli ;
                    uli.LowPart = ft.dwLowDateTime ;
                    uli.HighPart = ft.dwHighDateTime ;
                    uli.QuadPart += ((ULONGLONG) rr) * 10000000 ;
                    ft.dwLowDateTime = uli.LowPart ;
                    ft.dwHighDateTime = uli.HighPart ;
                }
                f = SetFileTime(fp,NULL,NULL,&ft) ;
                CloseHandle(fp) ;
            }
#endif
            return meTRUE ;
        }
    }
    if(rr > 0)
    {
        if(n & meFILEOP_BACKUP)
            dFlags |= meRWFLAG_BACKUP ;
        if(n & meFILEOP_FTPCLOSE)
            dFlags |= meRWFLAG_FTPCLOSE ;
        if((rr = ffFileOp(sfname,fn,dFlags,fileMask)) > 0)
            return meTRUE ;
    }
    resultStr[0] = '0' - rr ;
    resultStr[1] = '\0' ;
    return meABORT ;
}
#endif

/*
 * This function performs an auto writeout to disk for the given buffer
 */
void
autowriteout(register meBuffer *bp)
{
    meUByte fn[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *ff ;

    bp->autoTime = -1 ;

    if(bp->fileName != NULL)
    {
        getFileStats(bp->fileName,0,NULL,lname) ;
        ff = (lname[0] == '\0') ? bp->fileName:lname ;
        if(!createBackupName(fn,ff,'#',1) &&
           (ffWriteFileOpen(fn,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp) > 0))
        {
            meLine *lp ;

            lp = meLineGetNext(bp->baseLine);            /* First line.          */
            while((lp != bp->baseLine) &&
                  (ffWriteFileWrite(meLineGetLength(lp),meLineGetText(lp),
                                    !(lp->flag & meLINE_NOEOL)) > 0))
            {
                lp = meLineGetNext(lp) ;
                if(TTbreakTest(1))
                {
                    ctrlg(1,1) ;
                    return ;
                }
            }
            if(ffWriteFileClose(fn,meRWFLAG_WRITE|meRWFLAG_AUTOSAVE,bp) > 0)
            {
                mlerase(MWCLEXEC) ;
                return ;
            }
        }
    }
    mlwrite(0,(meUByte *)"[Auto-writeout failure for buffer %s]",bp->name) ;
}

/*
 * This function removes an auto writeout file for the given buffer
 */
void
autowriteremove(register meBuffer *bp)
{
    meUByte fn[meBUF_SIZE_MAX] ;

    if((autoTime > 0) && bufferNeedSaving(bp) &&
       !createBackupName(fn,bp->fileName,'#',0) &&
       !meTestExist(fn))
        /* get rid of any tempory file */
        meUnlink(fn) ;
    bp->autoTime = -1 ;
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a meLine; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
int
writeOut(register meBuffer *bp, meUInt flags, meUByte *fn)
{
    if(meModeTest(bp->mode,MDBACKUP))
        flags |= meRWFLAG_BACKUP ;
#if MEOPT_TIMSTMP
    set_timestamp(bp);			/* Perform time stamping */
#endif
    if(ffWriteFile(fn,flags,bp) <= 0)
        return meFALSE ;
    /* No write error.      */
    /* must be done before buffer is flagged as not changed */
#if MEOPT_UNDO
    if(meSystemCfg & meSYSTEM_KEEPUNDO)
    {
        /* go through undo list removing any unedited flags */
        meUndoNode *nn ;
        nn = bp->undoHead ;
        while(nn != NULL)
        {
            if(meUndoIsSetEdit(nn))
                nn->type |= meUNDO_UNSET_EDIT ;
            nn = nn->next ;
        }
    }
    else
        meUndoRemove(bp) ;
#endif
    autowriteremove(bp) ;
    meModeClear(bp->mode,MDEDIT) ;

    if(fn != NULL)
    {
#ifndef _WIN32
        /* set the right file attributes */
        if(bp->stats.stmode != meUmask)
            meFileSetAttributes(fn,bp->stats.stmode) ;
#ifdef _UNIX
        /* If we are the root then restore the existing settings on the
         * file. This should really be done using a $system flag - but
         * one assumes that this is the default action for the super user. */
        if (meUid == 0)
        {
            if((bp->stats.stuid != meUid) || (bp->stats.stgid != meGid))
                chown ((char *)fn, bp->stats.stuid, bp->stats.stgid);
        }
        /* else restore the group for normal users if possible */
        else if((bp->stats.stgid != meGid) && meGidSize && meGidInGidList(bp->stats.stgid))
            chown ((char *)fn, meUid, bp->stats.stgid);
#endif
#endif
        /* Read in the new stats */
        getFileStats(fn,0,&bp->stats,NULL) ;
        /* Change the view status back to the file's permissions.
         * For root we let the super user do as they wish so no
         * read protection is added. */
        if(
#if (defined _UNIX)
           (meUid != 0) &&
#endif
           (!meStatTestWrite(bp->stats)))
            meModeSet(frameCur->bufferCur->mode,MDVIEW) ;
        else
            meModeClear(frameCur->bufferCur->mode,MDVIEW) ;
    }
    return meTRUE ;
}

static meUByte fileHasBinary[]="File had binary characters which will be lost, write" ;
static meUByte fileHasInLnEn[]="File had inconsistent line endings which will be corrected, write" ;
/*
 * This function performs the details of file writing. Uses the file
 * management routines in the "fileio.c" package. The number of lines written
 * is displayed. Sadly, it looks inside a meLine; provide a macro for this. Most
 * of the grief is error checking of some sort.
 */
static int
writeout(register meBuffer *bp, int flags)
{
    meUByte lname[meBUF_SIZE_MAX], *fn ;

    if(!meStrcmp(bp->name,"*stdin*"))
        fn = NULL ;
    else if((bp->name[0] == '*') || (bp->fileName == NULL))
        return mlwrite(MWABORT,(meUByte *)"[No file name set for buffer %s]",bp->name);
    else
    {
        meStat stats ;
        /*
         * Check that the file has not been modified since it was read it in. This
         * is a bit of a problem since the info we require is hidden in a buffer
         * structure which may not necessarily be the current buffer. If it is not
         * in the current buffer then dont bother to look for it.
         */
        getFileStats(bp->fileName,0,&stats,lname) ;
        if(flags & 1)
        {
            if(meFiletimeIsModified(stats.stmtime,bp->stats.stmtime))
            {
                meUByte prompt[meBUF_SIZE_MAX] ;
                sprintf((char *) prompt,"%s modified, write",bp->fileName) ;
                if(mlyesno(prompt) <= 0)
                    return ctrlg(meFALSE,1) ;
            }
            if(bp->fileFlag & (meBFFLAG_BINARY|meBFFLAG_LTDIFF))
            {
                if(mlyesno((bp->fileFlag & meBFFLAG_BINARY) ? fileHasBinary:fileHasInLnEn) <= 0)
                    return ctrlg(meFALSE,1) ;
                bp->fileFlag &= ~(meBFFLAG_BINARY|meBFFLAG_LTDIFF) ;
            }
        }
        fn = (lname[0] == '\0') ? bp->fileName:lname ;
        /* Quick check on the file write condition */
        if(writeCheck(fn,flags,&stats) <= 0)
            return meABORT;
    }

    return writeOut(bp,((flags & 0x02) ? meRWFLAG_IGNRNRRW:0),fn) ;
}

void
resetBufferNames(meBuffer *bp, meUByte *fname)
{
    if(fnamecmp(bp->fileName,fname))
    {
        meUByte buf[meBUF_SIZE_MAX], *bb ;
        meStrrep(&bp->fileName,fname) ;
        unlinkBuffer(bp) ;
        makename(buf, fname) ;
        if((bb = meStrdup(buf)) != NULL)
        {
            meFree(bp->name) ;
            bp->name = bb ;
        }
        linkBuffer(bp) ;
    }
}

/*
 * Ask for a file name, and write the contents of the current buffer to that
 * file. Update the remembered file name and clear the buffer changed flag.
 * This handling of file names is different from the earlier versions, and is
 * more compatable with Gosling EMACS than with ITS EMACS. Bound to "C-X C-W".
 */
int
writeBuffer(int f, int n)
{
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn ;

    if(inputFileName((meUByte *)"Write file",fname,1) <= 0)
        return meABORT ;

    if(frameCur->bufferCur->fileName != NULL)
        fn = frameCur->bufferCur->fileName ;
    else if(frameCur->bufferCur->name[0] != '*')
        fn = frameCur->bufferCur->name ;
    else
        fn = NULL ;

    if((n & 0x01) && (frameCur->bufferCur->fileFlag & (meBFFLAG_BINARY|meBFFLAG_LTDIFF)) &&
       (mlyesno((frameCur->bufferCur->fileFlag & meBFFLAG_BINARY) ? fileHasBinary:fileHasInLnEn) <= 0))
        return ctrlg(meFALSE,1);
    if((fn=writeFileChecks(fname,fn,lname,(n & 0x01)|meWRITECHECK_BUFFER)) == NULL)
        return meABORT ;

    if(!writeOut(frameCur->bufferCur,((n & 0x02) ? meRWFLAG_IGNRNRRW:0),fn))
        return meFALSE ;

    resetBufferNames(frameCur->bufferCur,fname) ;
    frameAddModeToWindows(WFMODE) ; /* and update ALL mode lines */

    return meTRUE ;
}

/*
 * Save the contents of the current buffer in its associatd file. No nothing
 * if nothing has changed (this may be a bug, not a feature). Error if there
 * is no remembered file name for the buffer. Bound to "C-X C-S". May get
 * called by "C-Z".
 */
int
saveBuffer(int f, int n)
{
    register int    s;

    /* Note that we check for existance here just incase sombody has
     * deleted it under our feet. There is nothing more annoying than the
     * editor bitching there are no changes when it's been zapped !! */
    /* Further note: the file name can be NULL, e.g. theres no file name
     * so this must be tested before meTestExist, BUT the file name can be
     * NULL and still be saved, e.g. the buffer name is *stdin* - so be careful */
    if((n & 0x01) && (frameCur->bufferCur->fileName != NULL) &&  /* Are we Checking ?? */
       (meTestExist (frameCur->bufferCur->fileName) == 0) &&     /* Does file actually exist ? */
       !meModeTest(frameCur->bufferCur->mode,MDEDIT))         /* Have we edited buffer ? */
        /* Return, no changes.  */
        return mlwrite(0,(meUByte *)"[No changes made]") ;
    if((s=writeout(frameCur->bufferCur,n)) > 0)
        frameAddModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return (s);
}

/* save-some-buffers, query saves all modified buffers */
int
saveSomeBuffers(int f, int n)
{
    register meBuffer *bp;    /* scanning pointer to buffers */
    register int status=meTRUE ;
    meUByte prompt[meBUF_SIZE_MAX] ;

    bp = bheadp;
    while (bp != NULL)
    {
        if(bufferNeedSaving(bp))
        {
            if(n & 1)
            {
                if(bp->fileName != NULL)
                    sprintf((char *)prompt, "%s: Save file (?/y/n/a/o/g) ? ", bp->fileName) ;
                else
                    sprintf((char *)prompt, "%s: Save buffer (?/y/n/a/o/g) ? ", bp->name) ;
                if((status = mlCharReply(prompt,mlCR_LOWER_CASE,(meUByte *)"ynaog",(meUByte *)"(Y)es, (N)o, Yes to (a)ll, N(o) to all, (G)oto, (C-g)Abort ? ")) < 0)
                    return ctrlg(meFALSE,1) ;
                else if(status == 'g')
                {
                    swbuffer(frameCur->windowCur,bp) ;
                    /* return abort to halt any calling macro (e.g. compile) */
                    return meABORT ;
                }
                else if(status == 'o')
                    break ;
                else if(status == 'n')
                    status = meFALSE ;
                else
                {
                    if(status == 'a')
                        n ^= 1 ;
                    status = meTRUE ;
                }
            }
            if((status > 0) && (writeout(bp,0) <= 0))
                return meABORT ;
        }
        bp = bp->next;            /* on to the next buffer */
    }
    frameAddModeToWindows(WFMODE) ;  /* and update ALL mode lines */
    return meTRUE ;
}

#if MEOPT_EXTENDED
int
appendBuffer(int f, int n)
{
    register meUInt flags ;
    register int ss ;
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn ;

    if(inputFileName((meUByte *)"Append to file",fname,1) <= 0)
        return meABORT ;

    if(((ss=getFileStats(fname,3,NULL,lname)) != meFILETYPE_REGULAR) && (ss != meFILETYPE_NOTEXIST))
        return meABORT ;
    fn = (lname[0] == '\0') ? fname:lname ;
    if(ss == meFILETYPE_NOTEXIST)
    {
        if(n & 0x01)
        {
            meUByte prompt[meBUF_SIZE_MAX];
            sprintf((char *)prompt, "%s does not exist, create",fn) ;
            if(mlyesno(prompt) <= 0)
                return ctrlg(meFALSE,1);
        }
        flags = 0 ;
    }
    else if(n & 0x04)
        flags = meRWFLAG_OPENTRUNC ;
    else
        flags = meRWFLAG_OPENEND ;
    if(n & 0x02)
        flags |= meRWFLAG_IGNRNRRW ;
    return ffWriteFile(fname,flags,frameCur->bufferCur) ;
}

/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the meBuffer structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
int
changeFileName(int f, int n)
{
    register int s, ft ;
    meUByte fname[meBUF_SIZE_MAX], lname[meBUF_SIZE_MAX], *fn ;

    if ((s=inputFileName((meUByte *)"New file name",fname,1)) == meABORT)
        return s ;

    if(((ft=getFileStats(fname,3,NULL,lname)) != meFILETYPE_REGULAR) && (ft != meFILETYPE_NOTEXIST)
#if MEOPT_SOCKET
        && (ft != meFILETYPE_FTP)
#endif
       )
        return meABORT ;
    fn = (lname[0] == '\0') ? fname:lname ;
    meNullFree(frameCur->bufferCur->fileName) ;

    if (s == meFALSE)
        frameCur->bufferCur->fileName = NULL ;
    else
        frameCur->bufferCur->fileName = meStrdup(fname) ;

#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
    if(meTestWrite(fn))
        meModeSet(frameCur->bufferCur->mode,MDVIEW) ;
    else
        meModeClear(frameCur->bufferCur->mode,MDVIEW) ;	/* no longer read only mode */
#endif
    frameAddModeToWindows(WFMODE) ;  /* and update ALL mode lines */

    return (meTRUE);
}
#endif

#ifdef _DOS
int
_meChdir(meUByte *path)
{
    register int s ;
    int len ;

    len = strlen (path)-1 ;

#ifndef __DJGPP2__
    if((len > 1) && (path[1] == _DRV_CHAR))
    {
        union REGS reg ;		/* cpu register for use of DOS calls */

        reg.h.ah = 0x0e ;
        reg.h.dl = path[0] - 'a' ;
        intdos(&reg, &reg);
    }
#endif
    if((len > 3) && (path[len] == DIR_CHAR))
        path[len] = '\0' ;
    else
        len = -1 ;
    s = chdir(path) ;
    if(len > 0)
        path[len] = DIR_CHAR ;
    return s ;
}
#endif

/************************ New file routines *****************************/
#ifdef _CONVDIR_CHAR
void
fileNameConvertDirChar(meUByte *fname)
{
    /* convert all '\\' to '/' for dos etc */
    while((fname=meStrchr(fname,_CONVDIR_CHAR)) != NULL)  /* got a '\\', -> '/' */
        *fname++ = DIR_CHAR ;
}
#endif

void
fileNameSetHome(meUByte *ss)
{
    int ll = meStrlen(ss) ;
    meNullFree(homedir) ;
    homedir = meMalloc(ll+2) ;
    meStrcpy(homedir,ss) ;
    fileNameConvertDirChar(homedir) ;
    if(homedir[ll-1] != DIR_CHAR)
    {
        homedir[ll++] = DIR_CHAR ;
        homedir[ll] = '\0' ;
    }
}

void
pathNameCorrect(meUByte *oldName, int nameType, meUByte *newName, meUByte **baseName)
{
    register meUByte *p, *p1 ;
    meUByte *urls, *urle ;
    int flag ;
#if (defined _DOS) || (defined _WIN32)
    meUByte *gwdbuf ;
#endif

    fileNameConvertDirChar(oldName) ;
    flag = 0 ;
    p = p1 = oldName ;
    /* search for
     * 1) set to root,  xxxx/http:// -> http://  (for urls)
     * 2) set to root,  xxxx/ftp://  -> ftp://   (for urls)
     * 2) set to root,  xxxx/file:yy -> yy       (for urls)
     * 3) set to root,  xxxx///yyyyy -> //yyyyy  (for network drives)
     * 4) set to root,  xxxx//yyyyy  -> /yyyyy
     * 5) set to Drive, xxxx/z:yyyyy -> z:yyyyy
     * 6) set to home,  xxxx/~yyyyy  -> ~yyyyy
     */
    for(;;)
    {
        if(isHttpLink(p1))
        {
            flag = 2 ;
            urls = p1 ;
            if((p=meStrchr(p1+7,DIR_CHAR)) == NULL)
                p = p1 + meStrlen(p1) ;
            p1 = p ;
        }
        else if(isFtpLink(p1))
        {
            flag = 3 ;
            urls = p1 ;
            if((p=meStrchr(p1+6,DIR_CHAR)) == NULL)
                p = p1 + meStrlen(p1) ;
            urle = p ;
            p1 = p ;
        }
#ifdef MEOPT_BINFS
        else if(isBfsFile(p1))
        {
            flag = 3 ;
            urls = p1 ;
            if((p=meStrchr(p1+5,DIR_CHAR)) == NULL)
                p = p1 + meStrlen(p1) ;
            urle = p ;
            p1 = p ;
        }
#endif
        else if(isUrlFile(p1))
        {
            flag = 0 ;
            p1 += 5 ;
            p = p1 ;
            /* loop here as if at the start of the file name, this
             * is so it handles "file:ftp://..." correctly */
            continue ;
        }
        else if(flag != 2)
        {
            /* note that ftp://... names are still processed, ftp://.../xxx//yyy -> ftp://.../yyy etc */
            if(p1[0] == DIR_CHAR)
            {
#ifndef _UNIX
                if(p1[1] == DIR_CHAR)
                {
                    /* Got a xxxx///yyyyy -> //yyyyy must not optimise further */
                    flag = 1 ;
                    while(p1[2] == DIR_CHAR)
                        p1++ ;
                    urls = p1 ;
                    if((p=strchr(p1+2,DIR_CHAR)) == NULL)
                        p = p1 + strlen(p1) ;
                    urle = p ;
                    p1 = p ;
                }
                else
#endif
                    /* Got a xxxx//yyyyy -> /yyyyy */
                    p = p1 ;
            }
            else if(p1[0] == '~')
            {
                /* Got a home, xxx/~/yyy -> ~/yyy, xxx/~yyy  -> ~yyy, ftp://.../xxx/~/yyy -> ftp://.../~/yyy */
                if(flag == 3)
                {
                    if(p1[1] == '/')
                    {
                        /* for ftp ~/ only, allow ftp://.../~/../../../yyy */
                        p = p1-1 ;
                        while(!meStrncmp(p1+2,"../",3))
                            p1 = p1+3 ;
                    }
                }
                else
                    p = p1 ;
            }
#ifdef _DRV_CHAR
            else if(isAlpha(p1[0]) && (p1[1] == _DRV_CHAR))
            {
                /* got a Drive, xxxx/z:yyyyy -> z:yyyyy - remove ftp:// or //yyyy/... */
                flag = 0 ;
                p = p1 ;
            }
#endif
        }
        if((p1=meStrchr(p1,DIR_CHAR)) == NULL)
            break ;
        p1++ ;
    }

    p1 = newName ;
    if(flag == 2)
        meStrcpy(p1,urls) ;
    else
    {
        if(flag)
        {
            int ll= (size_t) urle - (size_t) urls ;
            meStrncpy(p1,urls,ll) ;
            p1 += ll ;
        }
        urle = p1 ;
        if(p[0] == '\0')
        {
            *p1++ = DIR_CHAR ;
            *p1 = '\0' ;
        }
        else if((flag == 0) && (p[0] == '~'))
        {
            p++ ;
            {
#if MEOPT_REGISTRY
                meRegNode *reg=NULL ;
                meUByte *pe ;
                int ll ;

                if((nameType == PATHNAME_PARTIAL) && (meStrchr(p,DIR_CHAR) == NULL))
                {
                    /* special case when user is entering a file name and uses complete with 'xxxx/~yy' */
                    *p1++ = '~' ;
                    meStrcpy(p1,p) ;
                    if(baseName != NULL)
                        *baseName = p1 ;
                    return ;
                }
                if((p[0] != '\0') && (p[0] != DIR_CHAR) && ((reg = regFind(NULL,(meUByte *)"history/alias-path")) != NULL) &&
                   ((reg = regGetChild(reg)) != NULL))
                {
                    /* look for an alias/abbrev path */
                    if((pe = meStrchr(p,DIR_CHAR)) == NULL)
                        ll = meStrlen(p) ;
                    else
                        ll = (int) (((size_t) pe) - ((size_t) (p))) ;

                    while((reg != NULL) && (((int) meStrlen(reg->name) != ll) || meStrncmp(p,reg->name,ll)))
                        reg = regGetNext(reg) ;
                }
                if(reg != NULL)
                {
                    if(reg->value != NULL)
                    {
                        meStrcpy(p1,reg->value) ;
                        p1 += meStrlen(p1) - 1 ;
                        if(p1[0] != DIR_CHAR)
                            p1++ ;
                    }
                    p += ll ;
                }
                else
#endif
                    if(homedir != NULL)
                {
                    meStrcpy(p1,homedir) ;
                    p1 += meStrlen(p1) - 1 ;
                    if((p[0] != '\0') && (p[0] != DIR_CHAR))
                    {
                        *p1++ = DIR_CHAR ;
                        *p1++ = '.' ;
                        *p1++ = '.' ;
                        *p1++ = DIR_CHAR ;
                    }
                }
                else
                    p-- ;
                meStrcpy(p1,p) ;
            }
        }
	else if(flag)
            meStrcpy(p1,p) ;
#ifdef _DRV_CHAR
        /* case for /xxxx... */
        else if(p[0] == DIR_CHAR)
        {
            if((p != oldName) && (oldName[1] == _DRV_CHAR))
                /* file name was D:xxxxxx//yyyyyy, convert to D:/yyyyyy */
                p1[0] = oldName[0] ;
            else
                /* file name is /yyyyyy, convert to D:/yyyyyy */
                p1[0] = curdir[0] ;
            p1[1] = _DRV_CHAR ;
            meStrcpy(p1+2,p) ;
        }
        /* case for xxxx... */
        else if(p[1] != _DRV_CHAR)
#else
        /* case for xxxx... */
        else if(p[0] != DIR_CHAR)
#endif
        {
            meStrcpy(p1,curdir) ;
            meStrcat(p1,p) ;
        }
#ifdef _DRV_CHAR
        /* case for D:xxxxxx */
        else if((p[1] == _DRV_CHAR) && (p[2] != DIR_CHAR) &&
                ((gwdbuf = gwd(p[0])) != NULL))
        {
            meStrcpy(p1,gwdbuf) ;
            meStrcat(p1,p+2) ;
            meFree(gwdbuf) ;
        }
#endif
        else
            meStrcpy(p1,p) ;
    }
    if(flag != 2)
    {
        p = NULL ;
        p1 = urle ;
        if((flag == 3) && !meStrncmp(p1,"/~/",3))
        {
            /* for ftp ~/ only, allow ftp://xxx/~/../../../yyy */
            p1 += 2 ;
            while(!meStrncmp(p1+1,"../",3))
                p1 = p1+3 ;
        }
        while((p1=meStrchr(p1,DIR_CHAR)) != NULL)
        {
            if((p1[1] == '.') && (p1[2] == '.') &&         /* got /../YYYY */
               ((p1[3] == DIR_CHAR) || ((nameType == PATHNAME_COMPLETE) && (p1[3] == '\0'))))
            {
                if(p == NULL)        /* got /../YYYY */
                {
#ifdef _DRV_CHAR
                    /* check for X:/../YYYY */
                    if(urle[1] == _DRV_CHAR)
                        p = urle+2 ;
                    else
#endif
                        p = urle ;
                }
                /* else got /XXX/../YYYY */

                p1 += 3 ;
                while((*p++ = *p1++) != '\0')
                    ;
                p = NULL ;
                p1 = urle ;
            }
            else if((p1[1] == '.') &&                      /* got /./YYYY */
                    ((p1[2] == DIR_CHAR) || ((nameType == PATHNAME_COMPLETE) && (p1[2] == '\0'))))
            {
                meUByte *tt ;
                tt = p1+2 ;
                while((*p1++ = *tt++) != '\0')
                    ;
                if(p == NULL)
                    p1 = urle ;
                else
                    p1 = p ;
            }
            else
            {
                p = p1 ;
                p1++ ;
            }
        }
#ifdef _SINGLE_CASE
        if(flag == 0)
            makestrlow(newName) ;
#endif
    }
    if(baseName != NULL)
    {
        meUByte cc ;
        p = p1 = newName ;
        while((cc=*p1++) != '\0')
        {
            if((cc == DIR_CHAR) && (*p1 != '\0'))
                p = p1 ;
            else if((cc == '[') || (cc == '?') || (cc == '*'))
                /* this is base of a wild file base name break */
                break ;
        }
        *baseName = p ;
    }
}

#ifdef _WIN32
void
fileNameCorrect(meUByte *oldName, meUByte *newName, meUByte **baseName)
{
    meUByte *bn ;

    pathNameCorrect(oldName,PATHNAME_COMPLETE,newName,&bn) ;
    if(baseName != NULL)
        *baseName = bn ;

    if(isUrlLink(newName))
        return ;
#if MEOPT_BINFS
    if(isBfsFile(newName))
        return ;
#endif

    /* ensure the drive letter is stable, make it lower case */
    if((newName[1] == _DRV_CHAR) && isUpper(newName[0]))
        newName[0] = toLower(newName[0]) ;

    if(!fileNameWild(bn))
    {
        /* with windows naff file name case insensitivity we must get
         * the correct a single name letter case by using FindFirstFile*/
        HANDLE *handle;
        WIN32_FIND_DATA fd;
        if((handle = FindFirstFile(newName,&fd)) != INVALID_HANDLE_VALUE)
        {
            strcpy(bn,fd.cFileName) ;
            /* If a directory that append the '/' */
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bn += strlen(bn) ;
                *bn++ = DIR_CHAR ;
                *bn = '\0' ;
            }
            FindClose (handle);
        }
    }
}
#endif

void
getDirectoryList(meUByte *pathName, meDirList *dirList)
{
#if MEOPT_REGISTRY
    meUByte upb[meBUF_SIZE_MAX] ;
#endif
    meUByte **fls ;
    int noFiles ;
#ifdef _UNIX
    struct stat statbuf;
    meFiletime stmtime ;
#endif
#ifdef _WIN32
    meFiletime stmtime ;
    WIN32_FIND_DATA fd;
    HANDLE *handle;
#endif

    if(isUrlLink(pathName))
    {
        if(dirList->path != NULL)
        {
            if(!meStrcmp(dirList->path,pathName))
                return ;
            meNullFree(dirList->path) ;
            freeFileList(dirList->size,dirList->list) ;
        }
        dirList->path = NULL ;
        dirList->size = 0 ;
        dirList->list = NULL ;
#if MEOPT_SOCKET
        if(pathName[0] == 'f')
        {
            meLine hlp, *lp ;
            meUByte *ss ;
            int ii ;
            hlp.next = &hlp ;
            hlp.prev = &hlp ;
            if(ffReadFile(pathName,meRWFLAG_SILENT,NULL,&hlp,0,0,0) >= 0)
            {
                noFiles = 0 ;
                lp = hlp.next;
                while(lp != &hlp)
                {
                    noFiles++ ;
                    lp = lp->next;
                }
                if(noFiles &&
                   ((fls = meMalloc(sizeof(meUByte *) * noFiles)) != NULL) &&
                   (meRegexComp(&meRegexStrCmp,(meUByte *) "\\([-\\a]+ +\\d+ \\S+\\( +\\S+\\)?\\( +\\d+,\\)? +\\d+\\( +0[xX]\\h+\\)? +\\a\\a\\a +\\d\\d? +\\(\\d\\d\\d\\d\\|\\d\\d?:\\d\\d\\)\\|\\d\\d-\\d\\d-\\d\\d +\\d\\d?\\:\\d\\d[aApP][mM] +\\(\\d+\\|<DIR>\\)\\) +\\(.*\\)",0) == meREGEX_OKAY))
                {
                    noFiles = 0 ;
                    lp = hlp.next;
                    while (lp != &hlp)
                    {
                        if(meRegexMatch(&meRegexStrCmp,lp->text,meLineGetLength(lp),0,meLineGetLength(lp),meRSTRCMP_WHOLE))
                        {
                            ii = meRegexStrCmp.group[7].end - meRegexStrCmp.group[7].start ;
                            if((fls[noFiles] = meMalloc(ii+2)) == NULL)
                                break ;
                            meStrncpy(fls[noFiles],lp->text+meRegexStrCmp.group[7].start,ii) ;
                            if(lp->text[0] == 'd')
                                fls[noFiles][ii++] = '/' ;
                            else if((lp->text[0] == 'l') && ((ss = meStrstr(fls[noFiles]," -> ")) != NULL))
                                ii = ((size_t) ss) - ((size_t) fls[noFiles]) ;
                            else if(isDigit(lp->text[0]) && (lp->text[meRegexStrCmp.group[6].start+1] == 'D'))
                                fls[noFiles][ii++] = '/' ;
                            fls[noFiles++][ii] = '\0' ;
                        }
                        lp = lp->next;
                    }
                    if((lp == &hlp) && noFiles)
                    {
                        dirList->size = noFiles ;
                        dirList->list = fls ;
#ifdef _INSENSE_CASE
                        sortStrings(noFiles,fls,0,meStridif) ;
#else
                        sortStrings(noFiles,fls,0,(meIFuncSS) strcmp) ;
#endif
                    }
                    else
                        meFree(fls) ;
                }
                meLineLoopFree(&hlp,0) ;
            }
            dirList->path = meStrdup(pathName) ;
        }
#endif
        return ;
    }

#if MEOPT_REGISTRY
    if((pathName[0] == '~') && (pathName[1] == '\0') && (homedir != NULL))
    {
        meUByte *ss ;
        int len ;

        if((dirList->path != NULL) && !meStrcmp(dirList->path,"~"))
           return ;
        meStrcpy(upb,homedir) ;
        len = meStrlen(upb) ;
        upb[len-1] = '\0' ;
        if((ss = meStrrchr(upb,DIR_CHAR)) != NULL)
        {
            ss[1] = '\0' ;
            len = meStrlen(upb) ;
        }
        else
            upb[len-1] = DIR_CHAR ;
        pathName = upb ;
        meFiletimeInit(stmtime) ;
    }
    else
#endif
    {
#ifdef _UNIX
        if(!stat((char *)pathName,&statbuf))
            stmtime = statbuf.st_mtime ;
        else
            meFiletimeInit(stmtime) ;
#endif
#ifdef _WIN32
        int len ;

        meFiletimeInit(stmtime) ;
        if(((len = strlen(pathName)) > 0) && (pathName[len-1] == DIR_CHAR))
        {
            pathName[len-1] = '\0';
            if((handle = FindFirstFile(pathName,&fd)) != INVALID_HANDLE_VALUE)
            {
                stmtime.dwHighDateTime = fd.ftLastWriteTime.dwHighDateTime ;
                stmtime.dwLowDateTime = fd.ftLastWriteTime.dwLowDateTime ;
                FindClose(handle) ;
            }
            pathName[len-1] = DIR_CHAR ;
        }
#endif

        if((dirList->path != NULL) &&
           !meStrcmp(dirList->path,pathName) &&
#if (defined _UNIX) || (defined _WIN32)
           !meFiletimeIsModified(stmtime,dirList->stmtime)
#else
           !dirList->stmtime
#endif
           )
            return ;
    }

    /* free off the old */
    meNullFree(dirList->path) ;
    freeFileList(dirList->size,dirList->list) ;
    fls = NULL ;
    noFiles = 0 ;

#ifdef _DOS
    if(pathName[0] == '\0')
    {
        union REGS reg ;		/* cpu register for use of DOS calls */
        meUByte *ff ;
        int    ii ;

        for (ii = 1; ii <= 26; ii++)    /* Drives are a-z (1-26) */
        {
            reg.x.ax = 0x440e ;
            reg.h.bl = ii ;
            intdos(&reg,&reg) ;
            if((reg.x.cflag == 0) || (reg.x.ax != 0x0f))
            {
                if(((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(meUByte))) == NULL))
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                *ff++ = ii + 'a' - 1 ;
                *ff++ = _DRV_CHAR ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
        }
    }
    else (!isBfsFile(pathName))
    {
        struct ffblk fblk ;
        meUByte *ff, *ee, es[4] ;
        int done ;

        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + strlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        done = findfirst(pathName,&fblk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        while(!done)
        {
            if(((noFiles & 0x0f) == 0) &&
               ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
            {
                noFiles = 0 ;
                break ;
            }
            if((ff = meMalloc(strlen(fblk.ff_name)+2)) == NULL)
            {
                fls = NULL ;
                noFiles = 0 ;
                break ;
            }
            fls[noFiles++] = ff ;
            strcpy(ff,fblk.ff_name) ;
            makestrlow(ff) ;
            if(fblk.ff_attrib & FA_DIREC)
            {
                ff += strlen(ff) ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
            done = findnext(&fblk) ;
        }
    }
    dirList->stmtime = 0 ;
#endif
#ifdef _WIN32
    if(pathName[0] == '\0')
    {
        meUByte *ff ;
        int ii ;
        DWORD  dwAvailableDrives, dwMask;

        /* Get the drives */
        dwAvailableDrives = GetLogicalDrives ();

        /* Drives are a-z (bit positions 0-25) */
        for (ii = 1, dwMask = 1; ii <= 26; ii++, dwMask <<= 1)
        {
            if((ii == 1) || (dwAvailableDrives & dwMask)) /* Drive exists ?? */
            {
                /* Yes - add to the drive list */
                if(((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+1))) == NULL) ||
                   ((ff = meMalloc(4*sizeof(meUByte))) == NULL))
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                *ff++ = ii + 'a' - 1;
                *ff++ = _DRV_CHAR ;
                *ff++ = DIR_CHAR ;
                *ff   = '\0' ;
            }
        }
    }
    else if (!isBfsFile(pathName))
    {
        meUByte *ff, *ee, es[4] ;

        /* append the *.* - Note that this function assumes the pathName has a '/' and
         * its an array with 3 extra char larger than the string size */
        ee = pathName + strlen(pathName) ;
        ee[0] = '*' ;
        es[1] = ee[1] ;
        ee[1] = '.' ;
        es[2] = ee[2] ;
        ee[2] = '*' ;
        es[3] = ee[3] ;
        ee[3] = '\0' ;
        handle = FindFirstFile(pathName,&fd) ;
        ee[0] = '\0' ;
        ee[1] = es[1] ;
        ee[2] = es[2] ;
        ee[3] = es[3] ;
        if(handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                if((ff = meMalloc(meStrlen(fd.cFileName)+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                strcpy(ff,fd.cFileName) ;
                if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    ff += strlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
            } while (FindNextFile (handle, &fd));
            FindClose (handle);
        }
    }
    dirList->stmtime.dwHighDateTime = stmtime.dwHighDateTime ;
    dirList->stmtime.dwLowDateTime = stmtime.dwLowDateTime ;
#endif
#ifdef _UNIX
    if (!isBfsFile(pathName))
    {
        DIR    *dirp ;

        if((dirp = opendir((char *)pathName)) != NULL)
        {
#if (defined _DIRENT)
            struct  dirent *dp ;
#else
            struct  direct *dp ;
#endif
            struct stat statbuf;
            meUByte *ff, *bb, fname[meBUF_SIZE_MAX] ;

            meStrcpy(fname,pathName) ;
            bb = fname + meStrlen(fname) ;

            while((dp = readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                if((ff = meMalloc(meStrlen(dp->d_name)+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrcpy(ff,dp->d_name) ;
                if((ff[0] == '.') && ((ff[1] == '\0') || ((ff[1] == '.') && (ff[2] == '\0'))))
                {
                    ff += (ff[1] == '\0') ? 1:2 ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
#ifdef DT_DIR
                else if(dp->d_type == DT_DIR)
                {
                    ff += meStrlen(ff) ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
                else if(dp->d_type == DT_UNKNOWN)
#else
                else
#endif
                {
                    meStrcpy(bb,dp->d_name) ;
                    if(!stat((char *)fname,&statbuf) && S_ISDIR(statbuf.st_mode))
                    {
                        ff += meStrlen(ff) ;
                        *ff++ = DIR_CHAR ;
                        *ff   = '\0' ;
                    }
                }
            }
            closedir(dirp) ;
        }
        dirList->stmtime = stmtime ;
    }
#endif  /* _UNIX */

#ifdef MEOPT_BINFS
    if (isBfsFile(pathName))
    {
        bfsdir_t dirp;

        if((dirp = bfs_opendir(bfsdev, (char *)pathName+5)) != NULL)
        {
            bfsdirent_t *dirent;
            meUByte *ff, *bb, fname[meBUF_SIZE_MAX] ;

            meStrcpy(fname,pathName) ;
            bb = fname + meStrlen(fname) ;

            while((dirent = bfs_readdir(dirp)) != NULL)
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                if((ff = meMalloc(dirent->len+3)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrncpy(ff,dirent->name,dirent->len) ;
                ff[dirent->len] = '\0';
                if((ff[0] == '.') && ((ff[1] == '\0') || ((ff[1] == '.') && (ff[2] == '\0'))))
                {
                    ff += (ff[1] == '\0') ? 1:2 ;
                    *ff++ = DIR_CHAR ;
                    *ff   = '\0' ;
                }
                else
                {
                    meStrcpy(bb,ff) ;
                    if (bfs_type (bfsdev, (char *)fname+5) == BFS_TYPE_DIR)
                    {
                        ff += meStrlen(ff) ;
                        *ff++ = DIR_CHAR ;
                        *ff   = '\0' ;
                    }
                }
            }
            bfs_closedir(dirp) ;
        }
        dirList->stmtime = stmtime ;
    }
#endif  /* MEOPT_BINFS */

#if MEOPT_REGISTRY
    if(pathName == upb)
    {
        meRegNode *reg ;
        meUByte *ff ;
        int len ;

        /* add the alias/abbrev paths to the list */
        if(((reg = regFind(NULL,(meUByte *)"history/alias-path")) != NULL) &&
           ((reg = regGetChild(reg)) != NULL))
        {
            do
            {
                if(((noFiles & 0x0f) == 0) &&
                   ((fls = meRealloc(fls,sizeof(meUByte *) * (noFiles+16))) == NULL))
                {
                    noFiles = 0 ;
                    break ;
                }
                len = meStrlen(reg->name) ;
                if((ff = meMalloc(len+2)) == NULL)
                {
                    fls = NULL ;
                    noFiles = 0 ;
                    break ;
                }
                fls[noFiles++] = ff ;
                meStrcpy(ff,reg->name) ;
                ff[len] = DIR_CHAR ;
                ff[len+1] = '\0' ;
            } while((reg = regGetNext(reg)) != NULL) ;
        }
        pathName = (meUByte *) "~" ;
    }
#endif

    dirList->path = meStrdup(pathName) ;
    dirList->size = noFiles ;
    dirList->list = fls ;
    if(noFiles > 1)
#ifdef _INSENSE_CASE
        sortStrings(noFiles,fls,0,meStridif) ;
#else
        sortStrings(noFiles,fls,0,(meIFuncSS) strcmp) ;
#endif
}

/*
** frees the file list created by getFileList
*/
void
freeFileList(int noStr, meUByte **files)
{
    if(files == NULL)
        return ;

    while (--noStr >= 0)
        meFree(files[noStr]) ;

    meFree((void *) files) ;
}
