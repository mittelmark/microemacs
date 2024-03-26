/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Append archive.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 09:17:36 2009
 *  Last Modified : <240326.1355>
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
bfs_append (int options, char *appendfile, char *outputfile, char *fileordir)
{
    int status = 0;
    struct stat stat_buf;
    
    /* If the output file is not NULL then create the output file. */
    if (outputfile != NULL)
    {
        if ((status = bfs_filecopy (options, appendfile, outputfile)) != 0)
            return status;
        /* Change the destination file. */
        appendfile = outputfile;
    }
    
    /* The file is copied strip any archive from the append file. */
    bfs_strip (options, appendfile);
    
    /* Determine the type of file. */
    if ((status = stat (fileordir, &stat_buf)) == 0)
    {
        /* Determine if this is a directory */
        if ((stat_buf.st_mode & S_IFDIR) != 0)
        {
            FILE *fp;
            
            /* This is a directory build an archive. Open the append file */
            if ((fp = fopen (appendfile, "rb+")) == NULL)
            {
                fprintf (stderr, "Error: Cannot open file \"%s\" for writing\n",
                         strnil (appendfile));
                status = -1;
            }
            else
            {
                /* Build the archive */
                status = bfs_build (options, fp, fileordir);
                fclose (fp);
            }
        }
        else if ((stat_buf.st_mode & S_IFREG) != 0)
            status = bfs_fileappend (options, fileordir, appendfile);
        else
        {
            fprintf (stderr, "Error: Cannot deal with archive file \"%s\"\n",
                     strnil (fileordir));
            status = -1;
        }
    }
            
    /* If the status is not good then delete any output file that we might
     * have created. */
    if ((status != 0) && (outputfile != NULL))
    {
        fprintf (stderr, "Deleting file \"%s\"\n", outputfile);  
        remove (outputfile);
    }
    
    /* Resturn the status to the caller. */
    return status;
}
