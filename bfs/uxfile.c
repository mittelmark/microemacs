/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Extraction to a file.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sat Nov 7 21:50:30 2009
 *  Last Modified : <091108.1756>
 *
 *  Description   : Opens archive and copies the archive to a new archive
 *                  file.
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
 * Extract an acrchive to the named file system file. The archive may be
 * appended to the end of the file.
 * 
 * @param [in] options  The option mask.
 * @param [in] fsysname The filename system name of the output archive.
 * @param [in] archpath The path to the archive to extract.
 * 
 * @return The status 0 on success otherwise an error.
 */
int
bfs_xfile (int options, char *fsysname, char *archpath)
{
    bfs_t bfs;
    FILE *rfp = NULL;
    FILE *wfp = NULL;
    off_t roffset;
    int status = 1;

    /* Mount the file system. */
    bfs = bfs_mount (archpath, BFS_CHECK_NONE);
    for (;;)
    {
        char buf [2048];
        int nread;
        int nwrite;
        int total = 0;
        int error = 0;
        bfsinfo_t *minfo;
        
        if (bfs == NULL)
        {
            fprintf (stderr, "Cannot find valid archive at \"%s\"\n", archpath);
            break;
        }

        /* Get the archive start position. */
        minfo = bfs_info (bfs);
        if (minfo == NULL)
            roffset = -1;
        else
            roffset = minfo->file_offset;
        
        /* Test the validity of the offset */
        if (roffset < 0)
        {
            fprintf (stderr, "Cannot locate archive in \"%s\"\n", archpath);
            break;
        }

        /* Open the archive for reading. */
        if ((rfp = fopen (archpath, "rb")) == NULL)
        {
            fprintf (stderr, "Cannot open archive \"%s\" for reading\n", archpath);
            break;
        }

        /* Seek to the position. */
        if ((roffset > 0) && (fseeko (rfp, roffset, SEEK_SET) != 0))
        {
            fprintf (stderr, "Cannot open archive \"%s\" for reading\n", archpath);
            break;
        }

        /* Open the archive for writing */
        if ((wfp = fopen (fsysname, "wb")) == NULL)
        {
            fclose (rfp);
            fprintf (stderr, "Cannot open archive \"%s\" for writing\n", fsysname);
            break;
        }

        do
        {
            /* Read in a chunk. */
            nread = fread (buf, 1, sizeof (buf), rfp);
            if (nread > 0)
            {
                nwrite = fwrite (buf, 1, sizeof (buf), wfp);
                if ((error == 0) && (nwrite != nread))
                    error = 1;
                total += nwrite;
            }
        } while (nread > 0);

        /* Print out the result. */
        if (error != 0)
            fprintf (stderr, "Error writing \"%s\"\n", fsysname);
        else
            status = 0;
        fprintf (stderr, "Extracted archive from \"%s\" to \"%s\" %d bytes.\n",
                 archpath, fsysname, (int)(total));
        break;
    }

    /* Close the file. */
    if (rfp != NULL)
        fclose (rfp);
    if (wfp != NULL)
        fclose (wfp);
    
    /* Unmount the file system */
    if (bfs != NULL)
        bfs_umount (bfs);
    
    /* Return the status */
    return status;
}
