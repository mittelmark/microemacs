/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Archive listing
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sat Nov 7 21:02:27 2009
 *  Last Modified : <091108.1749>
 *
 *  Description   : Module lists the archive
 *
 *  Notes
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

#include "bfsutil.h"

/**
 * Utility method to list a directory. This method is recursive and calls
 * itself for each directory node that is encountered. On each recusion then
 * the pathname is appended with the new directory name found and then called
 * again to be evaluated.
 * 
 * @param [in] bfs The mounted file system.
 * @param [in] path The bfs pathname to process a nil terminated string of 
 *                  the form "/level1/level2/level3".
 */
static void
list_dir (bfs_t bfs, char *path)
{
    char pathbuf[2048];
    bfsdir_t dirp;
    int path_len;

    /* Create the path */
    strcpy (pathbuf, path);
    path_len = strlen (pathbuf);
    if (pathbuf[path_len-1] != '/')
    {
        strcat (pathbuf, "/");
        path_len++;
    }

    /* Iterate over the directories */
    if ((dirp = bfs_opendir (bfs, path)) != NULL)
    {
        bfsdirent_t *dirent;
        bfsstat_t statbuf;

        while ((dirent = bfs_readdir (dirp)) != NULL)
        {
            int status;
            off_t size = 0;
            off_t offset = 0;
            char type = '?';
            
            /* Skip the current directory. */
            if ((dirent->len == 1) &&
                (strncmp (".", dirent->name, dirent->len) == 0))
                continue;
            /* Skip the parent directory */
            if ((dirent->len == 2) &&
                (strncmp ("..", dirent->name, dirent->len) == 0))
                continue;
            /* Print the current directory. */
            strncpy (&pathbuf[path_len], dirent->name, dirent->len);
            pathbuf[path_len + dirent->len] = '\0';
            /* Stat the pathanme */
            if (bfs_stat (bfs, pathbuf, &statbuf) == 0)
            {
                if (statbuf.st_mode == BFS_TYPE_FILE)
                    type = 'f';
                else if (statbuf.st_mode == BFS_TYPE_DIR)
                    type = 'd';
                size = statbuf.st_size;
                offset = statbuf.st_bno;
            }
            printf ("%c %s\n", type, pathbuf);
            
            /* Recurse any directory */
            if (type == 'd')
                list_dir (bfs, pathbuf);
        }

        /* Close the directory */
        bfs_closedir (dirp);
    }
}

/**
 * List the archive on stdout.
 *
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to process.
 * 
 * @return The status of the call, 0 is success.
 */
int
bfs_list (int options, char *archpath)
{
    int status = 0;
    
    bfs_t bfs;

    /* Mount the file system. */
    bfs = bfs_mount (archpath, BFS_CHECK_NONE);
    if (bfs == NULL)
    {
        fprintf (stderr, "Cannot find valid archive at \"%s\"\n",
                 strnil (archpath));
        status = -1;
    }
    else
    {
        /* List the directory */
        /* TODO: Set status */
        list_dir (bfs, "/");
    }
    
    /* Return the status of the call to the caller. */
    return status;
}
