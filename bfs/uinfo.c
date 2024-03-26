/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Display the archive information.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 07:39:18 2009
 *  Last Modified : <240326.1356>
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
 *
 *  License: GPL - see file license.txt in the same folder as this file
 *
 ****************************************************************************/

#include "bfsutil.h"

/**
 * Get the archive information.
 * 
 * @param [in] options  The option mask.
 * @param [in] archpath The name of the archive to strip.
 * 
 * @return The status of the call, 0 is success.
 */
int
bfs_printinfo (int options, char *archpath)
{
    bfs_t bfs;
    int status = 0;
    
    /* Determine if we have access to the file. */
    status = access (archpath, F_OK | W_OK | R_OK);
    if (status != 0)
        fprintf (stderr, "Cannot access file \"%s\"\n", archpath);
    else
    {
        /* Mount the file system. */
        bfs = bfs_mount (archpath, BFS_CHECK_NONE);
        if (bfs == NULL)
            fprintf (stderr, "No archive found in \"%s\"\n", archpath);
        else
        {
            off_t offset;               /* Get the offset of the archive. */
            bfsinfo_t *minfo;
        
            /* Get the offset of the archive. */
            minfo = bfs_info (bfs);
            if (minfo == NULL)
            {
                fprintf (stderr, "Cannot get archive information \"%s\"\n", archpath);
                status = -1;
            }
            else 
            {
                /* Print the information. */
                fprintf (stdout, "Name: %s\n", strnil (archpath));
                fprintf (stdout, "Version: v%d.%d.%d\n", 
                         minfo->version_major,
                         minfo->version_minor,
                         minfo->version_micro);
                fprintf (stdout, "Created: %04d-%02d-%02d, %02d:%02d:%02d\n",
                         minfo->year, minfo->month, minfo->day, 
                         minfo->hour, minfo->minute, minfo->second);
                fprintf (stdout, "Start Addr: 0x%x/%d\n", 
                         (int)(minfo->file_offset), (int)(minfo->file_offset));
                fprintf (stdout, "Byte Size: 0x%x/%d\n", 
                         (int)(minfo->length), (int)(minfo->length));
            }
        }
    }
    
    /* Return the status to the caller. */
    return status;
}
                
        
