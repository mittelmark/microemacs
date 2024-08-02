/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System (BFS)
 *  Module        : File reader.
 *  Object Name   : $RCSfile: bfs.c,v $
 *  Revision      : $Revision: 1.1 $
 *  Date          : $Date: 2009/12/13 19:53:10 $
 *  Author        : $Author: jon $
 *  Created By    : Jon Green
 *  Created       : Fri Oct 23 22:00:08 2009
 *  Last Modified : <240802.0909>
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include "bfs.h"

#define BFS_DEBUG_ENABLE  0

#if BFS_DEBUG_ENABLE
#define BFS_DEBUG(x) {printf x; fflush (stdout);}
#define strnil(x) ((x==NULL)?"NULL":x)
#else
#define BFS_DEBUG(x) /* NULL */
#endif

/* On Windows systems then there is no fseeko() and ftello(). We could use
 * fseeki64() and ftelli64() however these are only supported on certain
 * versions of Windows and we require a generic binary. Chnange to the 64-bit
 * versions for a large build. */
#ifdef _WIN32
#ifndef fseeko 
#define fseeko fseek
#endif
#ifndef ftello
#define ftello ftell
#endif
#endif

/**
 * A magic marker used in structures to ensure that they are valid.
 */
#define BFS_MAGIC_TAG   0xb1f55f1b

struct s_bfs
{
    /** Valid flag - this value shall be 0xb1f5b1f5 */
    size_t tag;
    /** The root of the file system */
    size_t bno;
    /** The start of the file system */
    off_t fs;
    /** The info structure returned to the user. */
    bfsinfo_t info;
    /** The name of the file that we are mounting. */
    char name[1];
};

/**
 * Define the structure of a node.
 */
typedef struct s_bfs_node
{
    /* The current file pointer. */
    FILE *fp;
    /* File system */
    bfs_t bfs;
    /* The offset of the node. */
    off_t bno;
    /* The node type flag */
    unsigned char type_flag;
    /* The data length */
    off_t data_len;
    /* The data offset */
    off_t data_off;
    /* The original length */
    off_t orig_len;
    /* Auxilary data value */
    off_t aux_data;
    /* The attribute length */
    off_t attrib_len;
    /* The attribute pointer. */
    unsigned char *attrib_ptr;
    /* The data pointer */
    unsigned char *data_ptr;
} bfs_node_t;

/**
 * Define the structure of a directory entry
 */
typedef struct s_bfs_dirent
{
    /* The flags associated with the directory */
    size_t flags;
    /* The offset of the node in the file system */
    off_t bno;
    /* The length of the name */
    size_t len;
    /* The pointer to the name. */
    char *name;
    /* File system */
    bfs_t bfs;
    /* The original bfs-no */
    off_t bno_dir;
    /* The parent bfs-no */
    off_t bno_parent;
    /* The original node pointer. */
    unsigned char *node;
    /* The start of the directory list. */
    unsigned char *start_ptr;
    /* The end of the directory list */
    unsigned char *end_ptr;
    /* The current position in the directory list */
    unsigned char *cur_ptr;
    /* The integer location */
    size_t entrynum;
} bfs_dirent_t;

/**
 * The external file handle.
 */
struct s_bfsfile
{
    /* Simple memory identifier. */
    size_t tag;
    /* The node that is open. */
    struct s_bfs_node info;
    /* The integer file read position. */
    off_t freadpos;
    /* The number of file remaining bytes */
    off_t fremain;
    /* The encoding of the file */
    int encoding;
    union {
        /* Zlib structure */
        struct {
            /* The decompression buffer */
            unsigned char buf [2048];
            /* The decompression status */
            z_stream z_state;
            /* The status of the decompression. */
            int zstatus;
            /* The flush state */
            int zflush;
        } zlib;
    } u;
};

/**
 * The external directory handle.
 */
struct s_bfsdir
{
    /* Simple memory identifier. */
    size_t tag;
    /* Root of the Built-in file system */
    bfs_t bfs;
    /* The directory node that is open. */
    struct s_bfs_node info;
    /* The internal directory entry. */
    struct s_bfs_dirent dirent;
    /* The extenal directory entry that is returned via bfs_dirent() */
    bfsdirent_t bfs_dirent;
    /* The directory position. This is an integer where 0 = "."; 1 = ".." and
     * 2+ = entry. */
    size_t dirpos;
};

/**
 * Get the vuimsbf value.
 *
 * @param [in/out] pp Pointer to the start of the vuimsbf field which is advanced
 *                    to the next field on evaluation.
 *
 * @return The value of the vuimsbf field.
 */
off_t
vuimsbfGet (unsigned char **pp)
{
    unsigned char *p;
    off_t value = 0;

    assert (pp != NULL);
    assert (*pp != NULL);

    if ((p = *pp) != NULL)
    {
        int len;

        len = *p++;
        value = len & 0x1f;
        if ((len = (len >> 5)) > 0)
        {
            /* Get the value out. */
            do
            {
                value = (value << 8) | (size_t)(*p++);
            }
            while (--len > 0);
        }
        /* Save the pointer. */
        *pp = p;
    }
    return value;
}

/**
 * Get the vuimsbf value.
 *
 * @param [in] fp File pointer of the bytes to read.
 *
 * @return The value of the vuimsbf field.
 */
int
fvuimsbfGet (FILE *fp, off_t *o)
{
    int cc;
    int flen = 0;

    if ((cc = fgetc (fp)) != EOF)
    {
        off_t value = 0;
        int len;

        len = (cc >> 5) & 0x1f;
        flen = 1 + len;
        value = (off_t)(cc & 0x1f);
        if (len > 0)
        {
            /* Get the value out. */
            do
            {
                if ((cc = fgetc (fp)) != EOF)
                    value = (value << 8) | (off_t)(cc);
                else
                {
                    flen = 0;
                    break;
                }
            }
            while (--len > 0);
        }
        /* Return the value on a good status. */
        if ((flen > 0) && (o != NULL))
            *o = value;
    }
    return flen;
}

/**
 * Read the information bfs node.
 *
 * @param [in] fp  The file pointer.
 * @param [in] bfs The Built-in file system
 * @param [in] bno The offset to the binary node.
 * @param [in,out] info The decoded bnode information.
 *
 * @return A pointer following the decode, points at the data component.
 */
off_t
bnode_open (FILE *fp, bfs_t bfs, size_t bno, bfs_node_t *info)
{
    off_t bnode;                        /* Absolute offset of the bnode */

    assert (info != NULL);
    assert (fp != NULL);
    assert (bfs != NULL);
    assert (bno != 0);

    /* Save the file pointer. */
    info->fp = fp;
    /* Save the file system */
    info->bfs = bfs;
    /* Save the node bno */
    info->bno = bno;
    /* Save the node pointer. */
    bnode = bfs->fs + bno;
    /* Seek to the file position. */
    while (fseeko (fp, bnode, SEEK_SET) == 0)
    {
        int cc;

        /* Look up the directory node */
        if ((cc = fgetc (fp)) == EOF)
            break;
        bnode += 1;
        info->type_flag = (unsigned char) cc;
        /* Get the data length */
        bnode += fvuimsbfGet (fp, &info->data_len);
        /* Get the encoded size */
        if ((info->type_flag & 0x70) != 0)
            bnode += fvuimsbfGet (fp, &info->orig_len);
        else
            info->orig_len = info->data_len;
        /* Get the auxillary data */
        if ((info->type_flag & 0x08) != 0)
             bnode += fvuimsbfGet (fp, &info->aux_data);
        /* Skip the attributes */
        if ((info->type_flag & 0x80) != 0)
        {
            off_t attrib_len;

            bnode += fvuimsbfGet (fp, &attrib_len);
            if (attrib_len > 0)
            {
                bnode += attrib_len;
                /* Allocate space for the attribute data */
                info->attrib_ptr = malloc (attrib_len);
                if (info->attrib_ptr != NULL)
                {
                    /* Read in the attribute data */
                    if (fread (info->attrib_ptr, 1, attrib_len, fp) != attrib_len)
                        info->attrib_len = attrib_len;
                    else
                    {
                        free (info->attrib_ptr);
                        bnode = 0;
                        break;
                    }
                }
                else
                    break;
            }
        }
        else
        {
            info->attrib_ptr = NULL;
            info->attrib_len = 0;
        }
        /* Reset the decoded pointer. */
        info->data_ptr = NULL;
        info->data_off = bnode;
        break;
    }

    /* Return the node */
    return bnode;
}

/**
 * Close down access to the node.
 *
 * @param [in,out] info The node information.
 */
static void
bnode_close (bfs_node_t *info)
{
    assert (info != NULL);

    /* Free off the data memory */
    if (info->data_ptr != NULL)
    {
        free (info->data_ptr);
        info->data_ptr = NULL;
    }

    /* Free off the attribute memory. */
    if (info->attrib_ptr != NULL)
    {
        free (info->attrib_ptr);
        info->attrib_ptr = NULL;
    }

    /* Note we do not close the file pointer field (fp) as this is just a
     * reference. */
}

/**
 * Close the BFS file descriptor.
 *
 * @param [in] bfsfp The BFS file pointer.
 * @param [in] close Flag if the node is to be closed.
 */
void
bnode_fclose (bfsfile_t bfsfp, int close)
{
    /* Trap silly errors in debug mode */
    assert (bfsfp != NULL);
    assert (bfsfp->tag == BFS_MAGIC_TAG);
    assert (bfsfp->info.fp != NULL);

    BFS_DEBUG (("bnode_fclose Entry(%p)\n", fp));
    /* Runtime robustness checks */
    if ((bfsfp != NULL) && (bfsfp->tag == BFS_MAGIC_TAG))
    {
        /* Close the compression. */
        if (bfsfp->encoding == BFS_ENCODE_ZLIB)
            inflateEnd (&bfsfp->u.zlib.z_state);
        if (close != 0)
        {
            /* Close the file handle */
            if (bfsfp->info.fp != NULL)
                fclose (bfsfp->info.fp);
            /* Destruct the file information. */
            bnode_close (&(bfsfp->info));
        }
        /* Invalidate the tag */
        bfsfp->tag = 0;
        /* Free the memory */
        free (bfsfp);
    }
}

/**
 * Read from the node file descriptor.
 *
 * @param [in] dest The destination buffer provided by the caller.
 * @param [in] size The size of a single element in bytes.
 * @param [in] nitems The number of 'size' items to read.
 * @param [in] fp The file descriptor to read from.
 *
 * @return The number of items that have been read.
 */
int
bnode_fread (void *dest, size_t size, size_t nitems, bfsfile_t bfsfp)
{
    int nread = 0;                      /* Number of elements read */
    off_t remain;                       /* Remaining bytes */

    assert (bfsfp != NULL);
    assert (bfsfp->tag == BFS_MAGIC_TAG);
    assert (bfsfp->info.fp != NULL);

    BFS_DEBUG (("bnode_fread (%d,%d)\n", size, nitems));

    /* Determine what to do based on the encoding. */
    if (bfsfp->encoding == BFS_ENCODE_NONE)
    {
        /* Get the remaining bytes */
        remain = bfsfp->fremain;
        /* Compute the item size if necessary */
        nread = nitems;
        if (size > 1)
        {
            nread *= size;
            if (remain < (off_t) nread)
                nread = (remain/size) * size;
        }
        else if (remain < (off_t) nread)
            nread = remain;

        /* Perform the read if there is anything left */
        if (nread != 0)
        {
            /* Decrement the number of bytes read. */
            bfsfp->fremain -= nread;

            /* Perform the read. */
            nread = fread (dest, 1, nread, bfsfp->info.fp);
                /* Compute the number of items read. */
            if (size > 1)
                nread /= size;
        }
    }
    else if (bfsfp->encoding == BFS_ENCODE_ZLIB)
    {
        int dest_len;

        /* Compute the item size if necessary */
        dest_len = nitems;
        if (size > 1)
            dest_len *= size;

        /* Set up the zlib output buffer size. */
        bfsfp->u.zlib.z_state.avail_out = dest_len;
        bfsfp->u.zlib.z_state.next_out = dest;

            /* Repeat the decompression by writing to the output buffer. */
        while (bfsfp->u.zlib.zstatus == Z_OK)
        {
            bfsfp->u.zlib.zstatus =
                  inflate (&bfsfp->u.zlib.z_state, bfsfp->u.zlib.zflush);
            if (bfsfp->u.zlib.zstatus == Z_OK)
            {
                /* Check whether we need to fill the input buffer. */
                if (bfsfp->u.zlib.z_state.avail_in == 0)
                {
                    /* Try to read some more data. */
                    if (bfsfp->fremain > 0)
                    {
                        int len;

                        /* Compute the read length */
                        len = bfsfp->fremain;
                        if (len > sizeof (bfsfp->u.zlib.buf))
                            len = sizeof (bfsfp->u.zlib.buf);

                        /* Read in the data */
                        bfsfp->u.zlib.z_state.avail_in =
                              fread (bfsfp->u.zlib.buf, 1, len, bfsfp->info.fp);
                        bfsfp->fremain -= bfsfp->u.zlib.z_state.avail_in;
                    }
                    bfsfp->u.zlib.z_state.next_in = bfsfp->u.zlib.buf;
                }
                /* Check for no output */
                if (bfsfp->u.zlib.z_state.avail_out == 0)
                {
                    /* Compute the number of bytes read. */
                    nread = dest_len - bfsfp->u.zlib.z_state.avail_out;
                    break;
                }
            }
            else if (bfsfp->u.zlib.zstatus == Z_STREAM_END)
            {
                nread = dest_len - bfsfp->u.zlib.z_state.avail_out;
                break;
            }
        }
    }

    /* Return the number of items read. */
    BFS_DEBUG (("bnode_fread EXIT(%d)\n", nread));
    return nread;
}

/**
 * Prepare to read a file. We assume that the info node file pointer is
 * positioned at the data and the node has been opened.
 *
 * @param [in] info The decoded node information.
 *
 * @return The file pointer or NULL on an error.
 */
bfsfile_t
bnode_fopen (bfs_node_t *info)
{
    bfsfile_t bfsfp;

    /* Create a new file descriptor. */
    if ((bfsfp = (bfsfile_t) malloc (sizeof (struct s_bfsfile))) != NULL)
    {
        /* Populate the read structure. */
        bfsfp->tag = BFS_MAGIC_TAG;
        memmove (&bfsfp->info, info, sizeof (bfs_node_t));
        bfsfp->info.data_ptr = NULL;
        bfsfp->info.attrib_ptr = NULL;
        bfsfp->freadpos = 0;
        bfsfp->fremain = bfsfp->info.data_len;

        /* Determine if this is compressed or not. */
        bfsfp->encoding = (bfsfp->info.type_flag >> 4) & 0x07;
        if (bfsfp->encoding == BFS_ENCODE_NONE)
        {
            /* Nothing to do */;
        }
        else if (bfsfp->encoding == BFS_ENCODE_ZLIB)
        {
            /* Initialise the decompression state */
            bfsfp->u.zlib.z_state.zalloc = Z_NULL;
            bfsfp->u.zlib.z_state.zfree = Z_NULL;
            bfsfp->u.zlib.z_state.opaque = Z_NULL;
            bfsfp->u.zlib.zflush = Z_SYNC_FLUSH;

            /* Initialise the inflate algorithm */
            bfsfp->u.zlib.zstatus = inflateInit (&bfsfp->u.zlib.z_state);
            if (bfsfp->u.zlib.zstatus != Z_OK)
            {
                /* We have a problem with the decompression, clean up the
                 * mess. */
                bnode_fclose (bfsfp, 0);
                bfsfp = NULL;
                BFS_DEBUG (("bnode_fopen: Decompression failed.\n"));
            }
            else
            {
                int nread;

                /* Determine how much data to read. */
                nread = sizeof (bfsfp->u.zlib.buf);
                if (nread > bfsfp->fremain)
                {
                    nread = bfsfp->fremain;
                    bfsfp->fremain = 0;
                }

                /* Read in as much data as possible into the buffer. */
                nread = fread (bfsfp->u.zlib.buf, 1, nread, bfsfp->info.fp);

                /* Initialise zlib. */
                bfsfp->u.zlib.z_state.next_in = bfsfp->u.zlib.buf;
                bfsfp->u.zlib.z_state.avail_in = nread;
            }
        }
    }

    /* Return the file pointer to the caller. */
    BFS_DEBUG (("bfs_fopen EXIT(%p): %s\n", bfsfp));
    return bfsfp;
}

/**
 * Initialise the directory entry. Note that the info node must exist until
 * dirClose().
 *
 * @return The status of the call.
 * @retval 0 Call was successful
 * @retval -1 This is not a directory node.
 * @retval -2 Compression method not supported
 */
static int
bnode_opendir (bfs_node_t *info, bfs_dirent_t *dirent)
{
    int status = 0;

    assert (info != NULL);
    assert (dirent != NULL);

    /* If this is not a directory node then return an error. */
    if ((info->type_flag & 0x7) != BFS_TYPE_DIR)
        status = -1;
    /* Read in the directory */
    else
    {
        int data_len;
        int ii;

        /* Get the size of the data area. We use the orig_len as this is the
         * decompressed size. */
        data_len = (int)(info->orig_len);

        /* Allocate the memory if required. */
        if ((data_len > 0) && (info->data_ptr == NULL))
        {
            /* Allocate the data space. */
            info->data_ptr = malloc (data_len);
            if (info->data_ptr != NULL)
            {
                bfsfile_t fp;

                /* Read in the data. */
                if ((fp = bnode_fopen (info)) != NULL)
                {
                    /* Read in the data */
                    ii = bnode_fread (info->data_ptr, 1, data_len, fp);
                    if (ii != data_len)
                    {
                        assert (ii == data_len);
                        /* Problem reading */
                        free (info->data_ptr);
                        info->data_ptr = NULL;
                    }

                    /* Close the file */
                    bnode_fclose (fp, 0);
                }
                else
                {
                    assert (fp != NULL);
                    free (info->data_ptr);
                    info->data_ptr = NULL;
                }
            }
        }

        /* Create the new node. */
        if ((data_len == 0) || (info->data_ptr != NULL))
        {
            /* Initialise the dirent structure */
            dirent->bfs = info->bfs;
            dirent->bno_dir = info->bno;
            dirent->bno_parent = info->aux_data;
            dirent->start_ptr = info->data_ptr;
            dirent->cur_ptr = dirent->start_ptr;
            dirent->end_ptr = dirent->start_ptr + info->data_len;
            dirent->entrynum = 0;
        }
        else
            status = -4;
    }
    /* Return the data to the caller. */
    return status;
}

/**
 * Close the directory item and free any memory.
 *
 * @param [in] The directory to close.
 */
static void
bnode_closedir (bfs_dirent_t *dirent)
{
    /* Free off any memory. */
    assert (dirent != NULL);
}

/**
 * Read the next directory entry.
 *
 * @param [in] The directory to read.
 *
 * @return 0 if the read was successful.
 */
static int
bnode_readdir (bfs_dirent_t *dirent)
{
    static char *dotname = "..";
    int status = 0;

    assert (dirent != NULL);

    /* Advance to the next entry number */
    dirent->entrynum += 1;

    /* Handle '.' and '..' which do not exist in the directory list and we
     * make them on the fly */
    if (dirent->entrynum < 3)
    {
        dirent->len = dirent->entrynum;
        dirent->name = &dotname[2 - dirent->entrynum];
        /* Note for the root node then do not report the parent as the mount
         * node. */
        if ((dirent->entrynum == 1) || (dirent->bno_parent == 3))
            dirent->bno = dirent->bno_dir;
        else
            dirent->bno = dirent->bno_parent;
    }
    /* Handle an empty directory */
    else if (dirent->start_ptr == NULL)
        status = -1;
    /* Handle a populated directory */
    else
    {
        unsigned char *p;               /* The current pointer */

        p = dirent->cur_ptr;
        if ((p < dirent->start_ptr) || (p >= dirent->end_ptr))
            status = -1;
        else
        {
            /* Read the entry from the bfs node data */

            /* Get the node offset */
            dirent->bno = vuimsbfGet (&p);
            /* Get the length of the name */
            dirent->len = vuimsbfGet (&p);
            /* Point to the name */
            dirent->name = (char *)(p);
            /* Set the current pointer */
            dirent->cur_ptr = p + dirent->len;
        }
    }

    /* Return success. */
    return status;
}

/**
 * Get the type of a bnode.
 *
 * @param [in] fp  The file read pointer.
 * @param [in] bfs Root of the file system
 * @param [in] bno The bnode
 *
 * @param Returns the type of the node.
 */
static unsigned char
bnode_type (FILE *fp, bfs_t bfs, off_t bno)
{
    assert (bfs != NULL);
    assert (fp != NULL);

    /* Seek to the current position. */
    if (fseeko (fp, bfs->fs + bno, SEEK_SET) == 0)
    {
        int cc;

        cc = fgetc(fp);
        BFS_DEBUG (("bnode_type (%x) = %d\n", (int)bno, cc));

        if (cc != EOF)
            return (unsigned char)(cc & 0x07);
    }

    /* Return an error. */
    return BFS_TYPE_UNDEF;
}

/**
 * Get the root node of the file system.
 *
 * @param [in] fp The file pointer pointing to the root node.
 *
 * @return The root bno of the file system or 0.
 */
size_t
bnode_root (FILE *fp, off_t start)
{
    size_t bno = 0;

    BFS_DEBUG (("In rootGet\n"));

    /* Seek to the start of the block */
    if (fseeko (fp, start, SEEK_SET) == 0)
    {
        unsigned char buf[3];

        /* Read in the first 3 bytes */
        if (fread (&buf, 1, 3, fp) == 3)
        {
            /* Check that this really is a Built-in file system. */
            if ((buf[0] == 'b') && (buf[1] == 'f') && (buf[2] == 's'))
            {
                struct s_bfs bfsdev;        /* Dummy device node */
                bfs_node_t info;            /* bnode information */

                /* Create the bfs device */
                bfsdev.fs = start;

                /* Check that the node is the mount point. */
                (void) bnode_open (fp, &bfsdev, 3, &info);
                /* Find the root node. */
                if ((info.type_flag & 0x07) == BFS_TYPE_MOUNT)
                {
                    /* Check that the root node is correct. */
                    if (bnode_type (fp, &bfsdev, info.aux_data) == BFS_TYPE_DIR)
                        bno = info.aux_data;
                }
                /* Close the node information. */
                bnode_close (&info);
            }
        }
    }

    /* Return the root bnode. */
    return bno;
}

/**
 * Resolve a name to a bno.
 *
 * @param [in] fp The file pointer to the device.
 * @param [in] bfs The file system device.
 * @param [in] bno The node to process.
 * @param [in] name The name of the object to find.
 *
 * @return The resolved bfs-no or zero on an error.
 */
static off_t
bnode_find (FILE *fp, bfs_t bfs, size_t bno, char *name)
{
    assert (bfs != NULL);
    assert (name != NULL);
    assert (fp != NULL);

    BFS_DEBUG (("bnode_find Enter: %d %s\n", (int)(bno), strnil(name)));

    /* Skip over the leading '/' */
    if (*name == '/')
        name++;

    /* Get the root information. */
    while (*name != '\0')
    {
        off_t ii;
        bfs_node_t info;                /* Node information */
        bfs_dirent_t dirent;            /* Directory entry */
        char *sname = name;             /* Start of the name */
        char cc;                        /* Current character */
        int name_len;                   /* Length of the name */
        int status = 0;                 /* Status of the call. */

        /* Find the node. */
        if ((ii = bnode_open (fp, bfs, bno, &info)) == 0)
            return ii;

        /* Find the end of the name */
        while (((cc = *name) != '/') && (cc != '\0'))
            name++;
        name_len = name - sname;
        /* Skip over the leading slash */
        if (cc == '/')
            name++;

        /* Open the directory */
        if ((status = bnode_opendir (&info, &dirent)) == 0)
        {
            while ((status = bnode_readdir (&dirent)) == 0)
            {
                int ii;

#if BFS_DEBUG_ENABLE
                {
                    char buf[1024];

                    printf ("bnode_find: Checking \"");
                    strncpy (buf, sname, name_len);
                    buf[name_len] = '\0';
                    printf ("%s\"(%d) == \"", buf, name_len);
                    strncpy (buf, dirent.name, dirent.len);
                    buf[dirent.len] = '\0';
                    printf ("%s\"(%d)\n", buf, dirent.len);
                }
#endif
                /* Check the name length */
                if (name_len != dirent.len)
                    continue;
                /* Compare the names */
                ii = strncmp (sname, (char *)(dirent.name), name_len);
                if (ii == 0)
                {
                    BFS_DEBUG (("bnode_find@ Found match %d\n", bno));
                    bno = dirent.bno;
                    break;
                }
                else if (ii < 0)
                {
                    BFS_DEBUG (("bnode_find@ Failed match\n"));
                    status = -1;
                    break;
                }
            }

            /* Close the directory */
            bnode_closedir (&dirent);
        }

        /* Close the node. */
        bnode_close (&info);

        /* Check the status */
        if (status != 0)
        {
            bno = 0;
            break;
        }
    }

    /* Return the node to the caller. */
    BFS_DEBUG (("bnode_find EXIT: %d\n", bno));
    return bno;
}

/**
 * Get the CRC-32 value of the data block from a file.
 * 
 * @param [in] fp    The file pointer.
 * @param [in] start The address of the start of the data.
 * @param [in] len   The size of the header in bytes.
 * 
 * @return 0 on success.
 */
static bfsuint32_t
fcrc32 (FILE *fp, off_t start, off_t len)
{
    bfsuint32_t crc32 = 0xffffffff;
    
    /* Check the header for a valid CRC-32 */
    if (fseeko (fp, start, SEEK_SET) == 0)
    {
        unsigned char buf [512];
        int remain = len;
        int nread;
        
        /* Read in all of the data until we are finished */
        do
        {
            nread = sizeof (buf);
            if (remain < nread)
                nread = remain;
            
            nread = fread (buf, 1, nread, fp);
            if (nread > 0)
            {
                remain -= nread;
                crc32 = bfs_crc32 (crc32, buf, nread);
            }
        }
        while ((nread > 0) && (remain > 0));
    }
    
    /* Return the status to the caller. */
    return crc32;
}

/****************************************************************************
 * Standard public methods
 ***************************************************************************/

/**
 * Check the data partitions for corruption.
 *
 * @param [in] bfs  The BFS structure.
 * @param [in] type The CRC check to perform. Specified as a bitmask of 
 *                  areas to check BFS_CHECK_HEAD, BFS_CHECK_DATA, 
 *                  BFS_CHECK_ALL or BFS_CHECK_NONE.
 * 
 * @return Status of the check 0 if successful otherwise an error.
 */
extern int
bfs_check (bfs_t bfs, int check) 
{
    bfs_node_t info;
    int status = -1;
    off_t bno;
    FILE *fp = NULL;
    
    /* Basic name and bfs validity check */
    if ((bfs == NULL) || (bfs->tag != BFS_MAGIC_TAG))
    {
        BFS_DEBUG (("bfs_check: Basic name and bfs validity check failed.\n"));
    }
    /* Open the file. */
    if ((fp = fopen (bfs->name, "rb")) == NULL)
    {
        BFS_DEBUG (("bfs_check: Cannot open file \"%s\".\n", bfs->name));
    }
    /* Get the root information. */
    else if ((bno = bnode_open (fp, bfs, 3, &info)) == 0)
    {
        BFS_DEBUG (("bfs_check: Root information failed.\n"));
    }
    else
    {
        if ((info.type_flag & 0x07) != BFS_TYPE_MOUNT)
        {
            BFS_DEBUG (("bfs_check: Type is not a mount.\n"));
        }
        else
        {
            /* Check the Header CRC-32 */
            if (((check & BFS_CHECK_HEAD) != 0) &&
                (fcrc32 (fp, bfs->fs, (bno - bfs->fs) + info.data_len) != 0))
            {
                BFS_DEBUG (("bfs_check: Header CRC-32 failed.\n"));
            }
            
            /* Check the data CRC-32. This is a CRC-32 from the next byte
             * following the header to the end of the archive. */
            else if (((check & BFS_CHECK_DATA) != 0) &&
                     (fcrc32 (fp, bno + info.data_len, 
                              bfs->info.length - ((bno - bfs->fs) + info.data_len)) !=
                      bfs->info.data_crc32))
            {
                BFS_DEBUG (("bfs_check: Data CRC-32 failed.\n"));
            }                        
            else
                status = 0;
        }
        
        /* Close the node */
        bnode_close (&info);
    }
    
    /* Close the file descriptor */
    if (fp != NULL)
        fclose (fp);
    
    /* Return the status */
    return status;
}

/**
 * Open access to the built-in file system.
 *
 * @param [in] addr The address of the file system.
 */
bfs_t
bfs_mount (char *name, int check)
{
    FILE *fp;                           /* File pointer */
    bfs_t bfs = NULL;
    off_t bno = 0;
    off_t start = 0;

    BFS_DEBUG (("bfs_mount: %s\n", strnil (name)));

    /* Open the file. */
    if ((fp = fopen (name, "rb")) != NULL)
    {
        /* Check the start marker. */
        if ((bno = bnode_root (fp, 0)) == 0)
        {
            /* This is not a BFS file system, check the end. */
            if (fseeko (fp, -9, SEEK_END) == 0)
            {
                int cc;

                /* Get the "BFS" string */
                while ((cc = fgetc (fp)) != 'B')
                    if (cc == EOF)
                        break;
                /* Check for the end of archive. */
                if ((cc != EOF) && (fgetc (fp) == 'F') && (fgetc (fp) == 'S') && (cc != EOF))
                {
                    off_t bfs_pos;

                    /* Save the current byte offset and get the start offset. */
                    bfs_pos = ftello (fp);
                    fvuimsbfGet (fp, &start);
                    if (start <= bfs_pos)
                    {
                        start = bfs_pos - start;
                        bno = bnode_root (fp, start);
                    }
                }
            }
        }
        else
            BFS_DEBUG (("bfs_mount failed: %s. bnode_root\n", strnil(name)));

        /* Open the file system */
        if (bno != 0)
        {
            int name_len;
            int data_bno;

            /* Calculate the length of the name */
            name_len = strlen (name);

            /* Allocate a structure to hold the file system information. */
            if ((bfs = malloc (sizeof (struct s_bfs) + name_len)) != NULL)
            {
                bfs_node_t info;

                /* Initialise the basic root information. */
                memset (bfs, 0, sizeof (struct s_bfs) + name_len);
                strcpy (bfs->name, name);
                bfs->tag = BFS_MAGIC_TAG;
                bfs->fs = start;
                bfs->bno = bno;

                /* Get the node information */
                if ((data_bno = bnode_open (fp, bfs, 3, &info)) != 0)
                {
                    int cc;
                    int ii;

                    /* version_major */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.version_major = cc;
                    /* version_minor */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.version_minor = cc;
                    /* version_minor */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.version_micro = cc;
                    /* reserved */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.reserved = cc;
                    /* year + month */
                    if ((cc = fgetc (fp)) != EOF)
                    {
                        bfs->info.year = cc << 4;
                        if ((cc = fgetc (fp)) != EOF)
                        {
                            bfs->info.year |= (cc >> 4) & 0xf;
                            bfs->info.month = (cc & 0xf);
                        }
                    }
                    /* day */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.day = (cc >> 2) & 0x3f;
                    /* hour */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.hour = cc;
                    /* minute */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.minute = cc;
                    /* second */
                    if ((cc = fgetc (fp)) != EOF)
                        bfs->info.second = cc;
                    /* Convert the time from BFS time to Windows FILETIME time */
#ifdef _WIN32                    
                    {
                        SYSTEMTIME ttmp;
                        
                        memset (&ttmp, 0, sizeof (ttmp));
                        ttmp.wYear = bfs->info.year;
                        ttmp.wMonth = bfs->info.month;
                        ttmp.wDay = bfs->info.day;
                        ttmp.wHour = bfs->info.hour;
                        ttmp.wMinute = bfs->info.minute;
                        ttmp.wSecond = bfs->info.second;
                        SystemTimeToFileTime (&ttmp, &bfs->info.utc_ctime);
                    }
#else
                    /* Convert the time from BFS time to UNIX time */
                    {
                        struct tm ttmp;
                        
                        memset (&ttmp, 0, sizeof (ttmp));
                        ttmp.tm_year = bfs->info.year - 1900;
                        ttmp.tm_mon = bfs->info.month - 1;
                        ttmp.tm_mday = bfs->info.day;
                        ttmp.tm_hour = bfs->info.hour;
                        ttmp.tm_min = bfs->info.minute;
                        ttmp.tm_sec = bfs->info.second;
                        
                        /* Create the time. */
                        bfs->info.utc_ctime = mktime (&ttmp);
                    }
#endif
                    /* free_node */
                    fvuimsbfGet (fp, &bfs->info.file_offset);
                    /* length */
                    fvuimsbfGet (fp, &bfs->info.length);
                    /* data crc32 */
                    for (ii = 0; ii < 4; ii++)
                        if ((cc = fgetc (fp)) != EOF)
                            bfs->info.data_crc32 = (bfs->info.data_crc32 << 8) | (cc & 0xff);
                    /* head crc32 */
                    for (ii = 0; ii < 4; ii++)
                        if ((cc = fgetc (fp)) != EOF)
                            bfs->info.head_crc32 = (bfs->info.head_crc32 << 8) | (cc & 0xff);
                    /* name */
                    bfs->info.name = bfs->name;
                    /* root_offset */
                    bfs->info.root_offset = bfs->bno;
                    /* file_offset */
                    bfs->info.file_offset = bfs->fs;
                    
                    /* Check the Header CRC-32 */
                    if (((check & BFS_CHECK_HEAD) != 0) &&
                        (fcrc32 (fp, start, (data_bno - start) + info.data_len) != 0))
                    {
                        /* CRC for the header failed. */
                        bno = 0;
                    }
                    
                    /* Check the data CRC-32. This is a CRC-32 from the next
                     * byte following the header to the end of the archive. */
                    if (((check & BFS_CHECK_DATA) != 0) &&
                        (fcrc32 (fp, data_bno + info.data_len, 
                                 bfs->info.length - ((data_bno - start) + info.data_len)) !=
                         bfs->info.data_crc32))
                    {
                        bno = 0;
                    }                        
                    
                    /* Close the node */
                    bnode_close (&info);
                }
                
                /* Check that there were no errors. */
                if (bno == 0)
                {
                    /* There was a problem somewhere */
                    free (bfs);
                    bfs = NULL;
                }
            }
        }

        /* Close the file we have performed the mount and extracted the
         * information that we need. */
        fclose (fp);
    }
    else
        BFS_DEBUG (("bfs_mount failed: %s. Open\n", strnil(name)));

    /* Return a handle to the file system to the caller. */
    BFS_DEBUG (("bfs_mount EXIT: %s (%p)\n", strnil(name), bfs));
    return bfs;
}

/**
 * Return the mount information.
 *
 * @param [in] bfs  The BFS structure.
 *
 * @return The info structure or NULL if the mount is not valid.
 */
bfsinfo_t *
bfs_info (bfs_t bfs)
{
    bfsinfo_t *info = NULL;

    if (bfs != NULL)
    {
        assert (bfs->tag == BFS_MAGIC_TAG);
        info = &bfs->info;
    }

    /* Return the data to the caller. */
    return info;
}

/**
 * Close access to the built-in file system.
 *
 * @param [in] bfs  The BFS structure.
 */
void
bfs_umount (bfs_t bfs)
{
    BFS_DEBUG (("bfs_umount(%p)\n", bfs));

    if (bfs != NULL)
    {
        assert (bfs->tag == BFS_MAGIC_TAG);

        /* Zero the valid flag and delete the memory allocation. */
        bfs->tag = 0;
        free (bfs);
    }
}

/**
 * Open the built-in file system
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] name The name of the file to open.
 *
 * @return A file descriptor or NULL on an error.
 */
bfsfile_t
bfs_fopen (bfs_t bfs, char *name)
{
    size_t bno;                         /* The bnode */
    bfsfile_t bfsfp = NULL;             /* Return file pointer. */
    FILE *fp = NULL;                    /* File pointer. */

    BFS_DEBUG (("bfs_fopen (%s)\n", strnil (name)));

    /* Basic name and bfs validity check */
    if ((name == NULL) || (name[0] == '\0') ||
        (bfs == NULL) || (bfs->tag != BFS_MAGIC_TAG))
    {
        BFS_DEBUG (("bfs_fopen: Basic name and bfs validity check failed.\n"));
    }
    /* Open the file. */
    else if ((fp = fopen (bfs->name, "rb")) == NULL)
    {
        BFS_DEBUG (("bfs_fopen: Cannot open file \"%s\".\n", bfs->name));
    }
    /* Get the root information. */
    else if ((bno = bnode_find (fp, bfs, bfs->bno, name)) == 0)
    {
        BFS_DEBUG (("bfs_fopen: Root information failed.\n"));
    }
    else if (bnode_type (fp, bfs, bno) != BFS_TYPE_FILE)
    {
        BFS_DEBUG (("bfs_fopen: Type is not a file.\n"));
    }
    else
    {
        bfs_node_t info;

        /* Open the node. */
        if (bnode_open (fp, bfs, bno, &info) == 0)
            BFS_DEBUG (("bfs_fopen: 2nd node open failed.\n"));
        /* Open the file. */
        else if ((bfsfp = bnode_fopen (&info)) == NULL)
            bnode_close (&info);        /* Failed - clean up */
    }

    /* Close the file in the case of an error. */
    if ((bfsfp == NULL) && (fp != NULL))
        fclose (fp);

    /* Return the file pointer to the caller. */
    BFS_DEBUG (("bfs_fopen EXIT(%p): %s\n", fp, strnil(name)));
    return bfsfp;
}

/**
 * Close the file descriptor.
 *
 * @param [in] fp The file descriptor to close.
 */
void
bfs_fclose (bfsfile_t bfsfp)
{
    /* Trap silly errors in debug mode */
    assert (bfsfp != NULL);
    assert (bfsfp->tag == BFS_MAGIC_TAG);
    assert (bfsfp->info.fp != NULL);

    BFS_DEBUG (("bfs_fclose Entry(%p)\n", fp));
    /* Runtime robustness checks */
    if ((bfsfp != NULL) && (bfsfp->tag == BFS_MAGIC_TAG))
        bnode_fclose (bfsfp, 1);
}

/**
 * Read from the file descriptor.
 *
 * @param [in] fp The file descriptor to read from.
 *
 * @return The character or EOF if there are no more characters.
 */
int
bfs_fgetc (bfsfile_t fp)
{
    int cc = EOF;

#if 0
    assert (fp != NULL);
    assert (fp->tag == BFS_MAGIC_TAG);

    BFS_DEBUG (("bfs_fgetc\n"));

    if (fp->cur_ptr != NULL)
    {
        if (fp->cur_ptr < fp->end_ptr)
        {
            assert (fp->cur_ptr >= fp->start_ptr);
            cc = *(fp->cur_ptr);
            fp->cur_ptr += 1;
        }
    }
#endif
    /* Return the character to the caller. */
    return cc;
}

/**
 * Read from the file descriptor.
 *
 * @param [in] dest The destination buffer provided by the caller.
 * @param [in] size The size of a single element in bytes.
 * @param [in] nitems The number of 'size' items to read.
 * @param [in] fp The file descriptor to read from.
 *
 * @return The number of items that have been read.
 */
int
bfs_fread (void *dest, size_t size, size_t nitems, bfsfile_t bfsfp)
{
    int nread = 0;                      /* Number of elements read */
    assert (bfsfp != NULL);
    assert (bfsfp->tag == BFS_MAGIC_TAG);

    BFS_DEBUG (("bfs_fread (%d,%d)\n", size, nitems));

    /* Call the internal reader. */
    if (bfsfp->info.fp != NULL)
        nread = bnode_fread (dest, size, nitems, bfsfp);

    /* Return the number of items read. */
    BFS_DEBUG (("bfs_fread EXIT(%d)\n", nread));
    return nread;
}

/**
 * Open a directory.
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] name The name of the directory to open.
 *
 * @return A file descriptor or NULL on an error.
 */
bfsdir_t
bfs_opendir (bfs_t bfs, char *name)
{
    size_t bno;                         /* The bnode */
    bfsdir_t dp = NULL;                 /* Directory pointer. */
    FILE *fp = NULL;                    /* The archive file pointer. */

    BFS_DEBUG (("bfs_opendir (%s)\n", strnil (name)));

    /* Basic bfs validity check */
    if (bfs == NULL)
    {
        BFS_DEBUG (("bfs_opendir: Not mounted\n"));
    }
    else if (bfs->tag != BFS_MAGIC_TAG)
    {
        BFS_DEBUG (("bfs_opendir: Bad BFS pointer\n"));
    }
    else if (name == NULL)
    {
        BFS_DEBUG (("bfs_opendir: Directory name is NULL\n"));
    }
    /* Open the file. */
    else if ((fp = fopen (bfs->name, "rb")) == NULL)
    {
        BFS_DEBUG (("bfs_opendir: Cannot open file \"%s\".\n", strnil (bfs->name)));
    }
    /* Get the root information. */
    else if ((bno = bnode_find (fp, bfs, bfs->bno, name)) == 0)
    {
        BFS_DEBUG (("bfs_opendir: Cannot fine node \"%s\".\n", strnil(name)));
    }
    else if (bnode_type (fp, bfs, bno) != BFS_TYPE_DIR)
    {
        BFS_DEBUG (("bfs_opendir: Node is not a directory \"%s\".\n", strnil (bfs->name)));
    }
    /* Create a new file descriptor. */
    else if ((dp = (bfsdir_t) malloc (sizeof (struct s_bfsdir))) != NULL)
    {
        /* Populate the structure. */
        dp->tag = BFS_MAGIC_TAG;
        dp->bfs = bfs;
        dp->dirpos = 0;

        if ((bnode_open (fp, bfs, bno, &dp->info) != 0) &&
            (bnode_opendir (&dp->info, &dp->dirent) != 0))
        {
            /* Error opening the directory */
            bfs_closedir (dp);
            dp = NULL;
        }
    }

    /* Always close the file pointer. */
    if (fp != NULL)
        fclose (fp);

    /* Return the directory pointer to the caller. */
    return dp;
}

/**
 * Close the directory
 *
 * @param [in] The directory reference from a bfs_opendir().
 *
 * @return The status of the close operation/
 * @retval 0 Operation was successful.
 * @retval !0 Operation faied.
 */
int
bfs_closedir (bfsdir_t dp)
{
    int status = 0;                     /* Assume that we pass. */

    BFS_DEBUG (("bfs_closedir\n"));

    if ((dp != NULL) && (dp->tag == BFS_MAGIC_TAG))
    {
        /* Close the directory */
        bnode_closedir (&dp->dirent);
        /* Close the node */
        bnode_close (&dp->info);
        /* Mark as closed and free. */
        dp->tag = 0;
        free (dp);
    }
    else
        status = -1;
    return status;
}

/**
 * Read the next directory entry
 *
 * @param [in] The directory reference from a bfs_opendir().
 */
bfsdirent_t *
bfs_readdir (bfsdir_t dp)
{
    bfsdirent_t *dent = NULL;

    assert (dp != NULL);
    assert (dp->tag == BFS_MAGIC_TAG);

    BFS_DEBUG (("bfs_readdir\n"));

    /* Get the current directory "." */
    dent = &dp->bfs_dirent;
    /* Increment the directory position. */
    dp->dirpos += 1;

    /* Read a directory entry. */
    if (bnode_readdir (&dp->dirent) == 0)
    {
        dent->bno = dp->dirent.bno;
        dent->len = dp->dirent.len;
        dent->name = dp->dirent.name;
    }
    else
        dent = NULL;

    /* Return the directory entry. */
    return dent;
}

/**
 * Get the status of the node.
 *
 * @param [in] bfs  The file system mount point.
 * @param [in] path The pathname of the object to determine.
 * @param [in] buf  The returned status information.
 *
 * @return The status of the call.
 * @retval 0 The call suceeeded.
 * @retval !0 The call failed.
 */
int
bfs_stat (bfs_t bfs, char *path, struct s_bfsstat *buf)
{
    int status = -1;
    size_t bno;
    FILE *fp;

    BFS_DEBUG (("bfs_stat (%s)\n", strnil (path)));

    /* Basic bfs validity check */
    if (bfs == NULL)
    {
        BFS_DEBUG (("bfs_stat: Not mounted\n"));
    }
    else if (bfs->tag != BFS_MAGIC_TAG)
    {
        BFS_DEBUG (("bfs_stat: Bad BFS pointer\n"));
    }
    else if (path == NULL)
    {
        BFS_DEBUG (("bfs_stat: pathname is NULL\n"));
    }
    else if (buf == NULL)
    {
        BFS_DEBUG (("bfs_stat: statbuf is NULL\n"));
    }
    /* Open the file. */
    else if ((fp = fopen (bfs->name, "rb")) == NULL)
    {
        BFS_DEBUG (("bfs_stat: Cannot open archive \"%s\".\n", strnil (bfs->name)));
    }
    /* Perform a stat operation on the path */
    else
    {
        /* Try to find the node */
        if ((bno = bnode_find (fp, bfs, bfs->bno, path)) != 0)
        {
            struct s_bfs_node info;

            /* Open the node */
            if (bnode_open (fp, bfs, bno, &info) != 0)
            {
                buf->st_mode = info.type_flag & 0x07;
                buf->st_bfs = bfs;
                buf->st_bno = info.bno;
                buf->st_encoding = (info.type_flag >> 4) & 0x7;
                if (buf->st_encoding != 0)
                    buf->st_size = info.orig_len;
                else
                    buf->st_size = info.data_len;
                buf->st_asize = info.data_len;
                
                /* There are currenty no attributes so inherit the time from
                 * the archive. */
                buf->st_utc_mtime = bfs->info.utc_ctime;
                buf->st_utc_ctime = bfs->info.utc_ctime;                      
                
                /* Set the return status to OK */
                status = 0;
            }
        }

        /* Close the file. */
        fclose (fp);
    }

    BFS_DEBUG (("bfs_stat EXIT(%d): %s\n", status, strnil(path)));

    /* Return the status to the caller. */
    return status;
}

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
int
bfs_type (bfs_t bfs, char *path)
{
    int status = -1;                    /* Status of the call. */
    FILE *fp;                           /* Read file pointer of archive */

    BFS_DEBUG (("bfs_type (%s)\n", strnil (path)));

    /* Basic bfs validity check */
    if (bfs == NULL)
    {
        BFS_DEBUG (("bfs_type: Not mounted\n"));
    }
    else if (bfs->tag != BFS_MAGIC_TAG)
    {
        BFS_DEBUG (("bfs_type: Bad BFS pointer\n"));
    }
    else if (path == NULL)
    {
        BFS_DEBUG (("bfs_type: pathname is NULL\n"));
    }
    /* Open the file. */
    else if ((fp = fopen (bfs->name, "rb")) == NULL)
    {
        BFS_DEBUG (("bfs_type: Cannot open archive \"%s\".\n", strnil (bfs->name)));
    }
    else
    {
        size_t bno;

        /* Get the root information. */
        if ((bno = bnode_find (fp, bfs, bfs->bno, path)) != 0)
            status = bnode_type (fp, bfs, bno);
        /* Close the file. */
        fclose (fp);
    }

    /* Return the status to the caller. */
    BFS_DEBUG (("bfs_type: %s = %d\n", strnil(path), status));
    return status;
}

/**
 * Initialise the CRC-32 calculation. When constructing a CRC-32 then the
 * first call to to the crc calculation should have a "crc" value of
 * 0xfffffff.
 * 
 * @param [in] crc The starting value or continued CRC-32 value.
 * @param [in] data The data to be CRC32'ed
 * @param [in] len  The length of the data.
 */
bfsuint32_t 
bfs_crc32 (bfsuint32_t crc, unsigned char *data, int len)
{
    static bfsuint32_t crc_table [256] =
    {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
        0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
        0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
        0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
        0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
        0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
        0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
        0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
        0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
        0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
        0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
        0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
        0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
        0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
        0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
        0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
        0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
        0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
        0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
        0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
        0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
        0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
    };
    
    /* Compute the CRC32 value with the available data */
    while (--len >= 0)
        crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data++) & 0xff];
    return crc;
}
