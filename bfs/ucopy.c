/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Copy a file.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 09:24:30 2009
 *  Last Modified : <091109.2251>
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
 *  All Rights Reserved.
 *
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Jon Green.
 *
 ****************************************************************************/

#include "bfsutil.h"

/**
 * Copy a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to create.
 *
 * @return Status of the call, 0 is success.
 */
int
bfs_filecopy (int options, char *srcfile, char *destfile)
{
    int status;

    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Copying \"%s\" => \"%s\"\n", srcfile, destfile);

    /* Determine if we can read the source file. */
    status = access (srcfile, F_OK | R_OK);
    if (status != 0)
        fprintf (stderr, "Error: Cannot access file \"%s\"\n", srcfile);
    else
    {
        FILE *rfp;
        FILE *wfp;

        if ((rfp = fopen (srcfile, "rb")) == NULL)
        {
            fprintf (stderr, "Error: Cannot open file \"%s\" for reading\n", srcfile);
            status = -2;
        }
        else if ((wfp = fopen (destfile, "wb")) == NULL)
        {
            fprintf (stderr, "Error: Cannot open file \"%s\" for writing\n", destfile);
            fclose (rfp);
            status = -3;
        }
        else
        {
            unsigned char buf [2048];
            int nread;

            /* Copy the contents of the file across. */
            do
            {
                int nwrite;

                /* Read in a buffers worth of data */
                nread = fread (buf, 1, sizeof (buf), rfp);
                if (nread > 0)
                {
                    /* Write the buffers worth to the new file. */
                    nwrite = fwrite (buf, 1, nread, wfp);
                    if (nwrite != nread)
                    {
                        fprintf (stderr, "Error: Problem writing \"%s\"\n", destfile);
                        status = -1;
                    }
                }
            }
            while ((status == 0) && (nread > 0));

            /* Close the files. */
            fclose (rfp);
            fclose (wfp);

            /* If there is an error then erase the new file. */
            if (status != 0)
            {
                /* Delete the file we have just created. */
                remove (destfile);
            }
            
            /* On UNIX systems then propogate the execute status to the new
             * binary. On other systems such as Windows and DOS we do not
             * need to do this. */
#ifndef _WIN32 
            if ((status == 0) && (access (srcfile, F_OK | R_OK | X_OK) == 0))
            {
                struct stat stat_buf;
                /* The execute bit is set on the source file, propogate this
                 * across to the new file. */

                if (stat (destfile, &stat_buf) == 0)
                {
                    /* Modify the execute bits on the file. */
                    stat_buf.st_mode |= S_IXUSR | S_IXGRP | S_IXOTH;

                    /* Modify the permissions on the new file. */
                    if (chmod (destfile, stat_buf.st_mode) != 0)
                        fprintf (stderr, "Warn: Problem adding execute mode to \"%s\"\n", destfile);
                }
                else
                    fprintf (stderr, "Warn: Problem accessing \"%s\"\n", destfile);
            }
#endif
        }
    }

    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Copied \"%s\" => \"%s\" [%s]\n",
                 srcfile, destfile, strstatus (status));

    /* Return the status to the caller. */
    return status;
}

/**
 * Append a file.
 *
 * @param [in] options  The option mask.
 * @param [in] srcfile  The name of the file to be copied.
 * @param [in] destfile The name of the file to append.
 *
 * @return Status of the call, 0 is success.
 */
int
bfs_fileappend (int options, char *srcfile, char *destfile)
{
    int status = 0;
    FILE *rfp;
    FILE *wfp;

    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Appending \"%s\" => \"%s\"\n", srcfile, destfile);

    if ((rfp = fopen (srcfile, "rb")) == NULL)
    {
        fprintf (stderr, "Error: Cannot open file \"%s\" for reading\n", srcfile);
        status = -2;
    }
    else if ((wfp = fopen (destfile, "ab")) == NULL)
    {
        fprintf (stderr, "Error: Cannot open file \"%s\" for appending\n", destfile);
        fclose (rfp);
        status = -3;
    }
    else
    {
        unsigned char buf [2048];
        int nread;

        /* Copy the contents of the file across. */
        do
        {
            int nwrite;

            /* Read in a buffers worth of data */
            nread = fread (buf, 1, sizeof (buf), rfp);
            if (nread > 0)
            {
                /* Write the buffers worth to the new file. */
                nwrite = fwrite (buf, 1, nread, wfp);
                if (nwrite != nread)
                {
                    fprintf (stderr, "Error: Problem writing \"%s\"\n", destfile);
                    status = -1;
                }
            }
        }
        while ((status == 0) && (nread > 0));

        /* Close the files. */
        fclose (rfp);
        fclose (wfp);
    }

    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Appended \"%s\" => \"%s\" [%s]\n",
                 srcfile, destfile, strstatus (status));

    /* Return the status to the caller. */
    return status;
}
