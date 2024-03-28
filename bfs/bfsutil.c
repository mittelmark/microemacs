/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Archive builder.
 *  Date          : $Date$
 *  Author        : John Green, Detlef Groth
 *  Created By    : Jon Green
 *  Created       : Sat Nov 7 19:24:43 2009
 *  Last Modified : <240328.2002>
 *
 *  Description
 *
 *  Notes
 *
 *  History
 *
 ****************************************************************************
 *
 *  Copyright (c) 2009 Jon Green, 2024 Detlef Groth.
 *
 *  License: GPL - see file license.txt in the same folder as this file
 *
 ****************************************************************************/

#include "bfsutil.h"

/* Add the command line options. */
#include <getopt.h>

/** The name of the program */
#define PROG_NAME      "bfs"
/** Copyright statement for program */
#define PROG_COPYRIGHT "Copyright (c) 2009 Jon Green <jon@jasspa.com>"
/* Platform identification. */
#ifdef _SUNOS
#define PROG_PLATFORM  "SunOS"
#elif (defined _LINUX)
#define PROG_PLATFORM  "Linux"
#elif (defined _WIN32)
#define PROG_PLATFORM  "Win32"
#else
#define PROG_PLATFORM  "UNIX"
#endif

/* Version numbers */
int bfs_version_major = BFS_VERSION_MAJOR;
int bfs_version_minor = BFS_VERSION_MINOR;
int bfs_version_micro = BFS_VERSION_MICRO;

/**
 * version; print the version information on the given stream.
 *
 * @param fp  (in)  The file pointer to the stream on which the message
 *                  should be delivered.
 */
static void
version (FILE *fp)
{
    fprintf (fp, "%s: V%d.%d.%d : (%s) %s\n",
             PROG_NAME, 
             bfs_version_major, bfs_version_minor, bfs_version_micro,
             PROG_PLATFORM, __DATE__);
    fprintf (fp, "%s: %s\n", PROG_NAME, PROG_COPYRIGHT);
}

/**
 * usage; Print the usage of the utility on the given stream. Provides some
 * help for the user. There is NO return from this call.
 *
 * @param fp  (in) The reporting stream file descriptor.
 */
static void
usage (FILE *fp)
{
    int ii;

    fprintf (fp, "Synopsis: Build and manipulate a BFS archive.\n");
    fprintf (fp, "Usage:    %s [options] <dir> or <file>\n\n", PROG_NAME);
    fprintf (fp, "Syntax:\n");
    fprintf (fp, "  file.bin  - me binary\n");
    fprintf (fp, "  file.bbin - me bfs binary\n");    
    fprintf (fp, "  file.bfs  - bfs archiv file\n");    
    fprintf (fp, "  dir       - jasspa directory with macros and (optional) spelling etc subfolders\n\n");    
    fprintf (fp, "Options:\n");
    fprintf (fp, "  -a <file.bin> <dir>            : Append archive to end of existing file\n");
    fprintf (fp, "  -c <file.bfs> <dir>            : Create a standalone archive\n");
    fprintf (fp, "  -f <file.bfs> <file.bbin>      : Extract archive to a file.\n");
    fprintf (fp, "  -i <file.bfs>                  : Print archive information.\n");
    fprintf (fp, "  -l <file.bfs|file.bbin>        : List the contents of the archive.\n");
    fprintf (fp, "  -n/-N                          : Exclude(n)/Allow(N) zero length files, exclude by default\n");
    fprintf (fp, "  -o <file.bbin>                 : The name of the output file (optionally used with -a).\n");
    fprintf (fp, "  -q                             : Quiet, minimise messages.\n");
    fprintf (fp, "  -s <file.bbin>                 : Strip an archive from the end of a file.\n");
    fprintf (fp, "                                   This modifies file.bbin directly.\n");
    fprintf (fp, "  -t <file.bfs|file.bbin>        : Test the archive\n");
    fprintf (fp, "  -v                             : Enable verbose information.\n");    
    fprintf (fp, "  -x <dir> <file.bfs|file.bbin>  : Extract archive from file to a directory.\n");
    fprintf (fp, "  -z <type>                      : Compression options (default is -z9f)\n");
    fprintf (fp, "           d - Compress directories\n");
    fprintf (fp, "           f - Compress file\n");
    fprintf (fp, "           0-9 Compression level, 9 is maximum, 0 is none\n"); 
    fprintf (fp, "  -h/-?                          : Help information\n");
    fprintf (fp, "\n");

    /* No return */
    exit (1);
}

/**
 * The main program.
 *
 * @param [in] argc The number of arguments.
 * @param [in] argv
 */
int
main (int argc, char *argv[])
{
    int ii;                             /* Local integer variable */
    char *bfsopt_append = NULL;         /* File to append. */
    char *bfsopt_output = NULL;         /* Name of the output file. */
    char *bfsopt_create = NULL;         /* Create a file. */
    char *bfsopt_xdir = NULL;           /* Extract to directory */
    char *bfsopt_xfile = NULL;          /* Extract to file */
    char bfsopt_do = 0;                 /* Test the archive */
    int bfsopt_flags;                   /* Option flags */
    int status = 0;                     /* Status of the call. */
    
    /* Assign default options */
    bfsopt_flags = (BFSUTIL_OPT_COMPRESS_FILE |
                    BFSUTIL_OPT_SKIP_ZERO_LEN);
    
    /* Process the options */
    while ((ii = getopt (argc, argv, "a:c:f:hinNo:qtlsvx:z?")) != EOF)
    {
        switch (ii)
        {
            /* -a <file>  : Append archive to end of existing file. */
        case 'a':
            if (optarg != NULL)
            {
                if (bfsopt_append != NULL)
                    free (bfsopt_append);
                bfsopt_append = strdup (optarg);
            }
            else
            {
                fprintf (stderr, "ERROR: File expected with -a option i.e. -a file\n");
                usage (stderr);
            }
            if (bfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c\n", bfsopt_do);
            bfsopt_do = ii;
            break;

            /* -c <file>  : Create a standalone archive */
        case 'c':
            if (optarg != NULL)
            {
                if (bfsopt_create != NULL)
                    free (bfsopt_create);
                bfsopt_create = strdup (optarg);
            }
            else
            {
                fprintf (stderr, "ERROR: File expected with -c option i.e. -c file\n");
                usage (stderr);
            }
            if (bfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c\n", bfsopt_do);
            bfsopt_do = ii;
            break;

            /* -f <file>  : Extract archive to a file. */
        case 'f':
            if (optarg != NULL)
            {
                if (bfsopt_xfile != NULL)
                    free (bfsopt_xfile);
                bfsopt_xfile = strdup (optarg);
            }
            else
            {
                fprintf (stderr, "ERROR: File expected with -f option i.e. -f dir\n");
                usage (stderr);
            }
            if (bfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c\n", bfsopt_do);
            bfsopt_do = ii;
            break;
            
            /* -i : Print the achive information. */
            /* -l : List the contents of the archive. */
            /* -s : Strip the archive from end of file. */
            /* -t : Test the archive */
        case 'l':
        case 's':
        case 't':
        case 'i':
            if (bfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c\n", bfsopt_do);
            bfsopt_do = ii;
            break;
            
            /* -n : Exclude zero length files */
        case 'n':
            bfsopt_flags |= BFSUTIL_OPT_SKIP_ZERO_LEN;
            break;
            
            /* -N : Allow zero length files */
        case 'N':
            bfsopt_flags &= ~BFSUTIL_OPT_SKIP_ZERO_LEN;
            break;
            
            /* -o <file>  : Name of the output file. */
        case 'o':
            if (optarg != NULL)
            {
                if (bfsopt_output != NULL)
                    free (bfsopt_output);
                bfsopt_output = strdup (optarg);
            }
            else
            {
                fprintf (stderr, "ERROR: File expected with -o option i.e. -o file\n");
                usage (stderr);
            }
            break;

            /* -q : Quiet running, minimise messages. */
        case 'q':
            bfsopt_flags &= ~BFSUTIL_OPT_VERBOSE;
            bfsopt_flags |= BFSUTIL_OPT_QUIET;            
            break;      
            
            /* -v : Enable verbose messages */
        case 'v':
            bfsopt_flags &= ~BFSUTIL_OPT_QUIET;
            bfsopt_flags |= BFSUTIL_OPT_VERBOSE;
            break;
            
            /* -x <dir>   : Extract archive to a directory. */
        case 'x':
            if (optarg != NULL)
            {
                if (bfsopt_xdir != NULL)
                    free (bfsopt_xdir);
                bfsopt_xdir = strdup (optarg);
            }
            else
            {
                fprintf (stderr, "ERROR: File expected with -x option i.e. -x dir\n");
                usage (stderr);
            }
            if (bfsopt_do != 0)
                fprintf (stderr, "WARN: Ignoring option %c\n", bfsopt_do);
            bfsopt_do = ii;
            break;

            /* -z <type>  : Compression options (default is -zf) */
        case 'z':
            if (optarg != NULL)
            {
                char *p;

                for (p = optarg; *p != '\0'; p++)
                {
                    /* f - Compress file */
                    if (*p == 'f')
                        bfsopt_flags |= BFSUTIL_OPT_COMPRESS_FILE;
                    /* d - Compress directories */
                    else if (*p == 'd')
                        bfsopt_flags |= BFSUTIL_OPT_COMPRESS_DIR;
                    /* ! - No compression */
                    else if (*p == '!')
                        bfsopt_flags &= ~(BFSUTIL_OPT_COMPRESS_FILE |
                                          BFSUTIL_OPT_COMPRESS_DIR);
                    else
                        fprintf (stderr, "WARN: Ignoring compression option option %c\n", *p);
                }
            }
            break;

        default:
            fprintf (stderr, "Illegal option %c (%#02x)\n\n",
                     (isprint (ii) ? ii : '?'), (int)(ii));
            /* Allow drop through */

            /* -?/-h : Help */
        case '?':
        case 'h':
            version (stdout);
            usage (stdout);
            /* No return */
        }
    }

    /* Print the version information */
    if ((bfsopt_flags & BFSUTIL_OPT_QUIET) != 0)
        version (stderr);

    /* Make sure some  arguments have been specified. */
    if ((argc - optind) == 0)
    {
        fprintf (stderr, "ERROR: No arguments specified.\n");
        exit (1);
    }

    /* Process the arguments that exist */
    for (status = 0; (optind < argc) && (status == 0); optind++)
    {
        switch (bfsopt_do)
        {
            /* Append archive */
        case 'a': 
            status = bfs_append (bfsopt_flags, bfsopt_append, bfsopt_output, argv[optind]);
            break;
            
            /* Create an archive */
        case 'c': 
            status = bfs_create (bfsopt_flags, bfsopt_create, argv[optind]);
            break;
            
            /* Extract the archive as a file */
        case 'f':
            status = bfs_xfile (bfsopt_flags, bfsopt_xfile, argv[optind]);
            break;
            
            /* Print the archive information */
        case 'i':
            status = bfs_printinfo (bfsopt_flags, argv[optind]);
            break;
            
            /* List the archive */
        case 'l':
            status = bfs_list (bfsopt_flags, argv[optind]);
            break;
            
            /* Remove the archive */
        case 's':
            status = bfs_strip (bfsopt_flags, argv[optind]);
            break;
            
            /* Test the archive. */
        case 't':
            status = bfs_test (bfsopt_flags, argv[optind]);
            break;
            
            /* Extract the archive as a directory */
        case 'x':
            status = bfs_xdir (bfsopt_flags, bfsopt_xdir, argv[optind]);
            break;
            
            /* Handle an error */
        default:
            fprintf (stderr, "Unhandled option '%c'(%d)\n",
                     (isprint (bfsopt_do) ? bfsopt_do : '?'), bfsopt_do);
            status = -1;
            break;
        }
    }
    
    /* Exit with the status of the call. */
    return (status);
}
