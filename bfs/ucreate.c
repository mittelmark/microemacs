/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Create an archive.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Sun Nov 8 09:17:36 2009
 *  Last Modified : <091108.1208>
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
 * Construct a BFS archive. 
 * 
 * @param [in] options  The option mask.
 * @param [in] outputfile The name of the file to output.
 * @param [in] fileordirename The name of the archive to append. This may be 
 *                            and existing archive file or a directory.
 * 
 * @return Status of the call, 0 is success.
 */
int 
bfs_create (int options, char *outputfile, char *fileordir)
{
    int status = 0;
    struct stat stat_buf;
    
    /* If the output file is NULL then assume a default name. */
    if ((outputfile == NULL) || (outputfile[0] == '\0'))
    {
        outputfile = "a.bfs";
        fprintf (stderr, "No output file specified, defaulting to \"%s\"\n", outputfile);
    }
    
    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Creating archive \"%s\" => \"%s\"\n", 
                 strnil (fileordir), strnil (outputfile));

    /* Delete the output file if it exists so that we can open the file in
     * update mode. */
    remove (outputfile);
    
    /* Determine the type of file. */
    if ((status = stat (fileordir, &stat_buf)) == 0)
    {
        /* Determine if this is a directory */
        if ((stat_buf.st_mode & S_IFDIR) != 0)
        {
            FILE *fp;
            
            /* This is a directory build an archive. */
            if ((fp = fopen (outputfile, "wb+")) == NULL)
            {
                fprintf (stderr, "Error: Cannot open file \"%s\" for writing\n",
                         strnil (outputfile));
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
            status = bfs_filecopy (options, fileordir, outputfile);
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
        
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Created archive \"%s\" => \"%s\" [%s]\n", 
                 strnil (fileordir), strnil (outputfile), strstatus (status));
    
    /* Resturn the status to the caller. */
    return status;
}
