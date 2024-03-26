/* -*- C -*- ****************************************************************
 *
 *  System        : Built-in File System
 *  Module        : Build the archive.
 *  Object Name   : $RCSfile$
 *  Revision      : $Revision$
 *  Date          : $Date$
 *  Author        : $Author$
 *  Created By    : Jon Green
 *  Created       : Wed Oct 21 16:54:05 2009
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

/* Compression */
#include <zlib.h>

/*
 * TODO: Sort the problem.
 * Windows temporary file tmpfp() does not see to work on Vista use a
 * horrible name temorarily??
 *
 */
#define TMP_FILE "bfs~~~~~.tmp"

/*
 * Debugging.
 */
#define BFSBUILD_DEBUG  0               /* Logging enable == 1; disable == 0 */

#if BFSBUILD_DEBUG
#define BFS_DEBUG(x) printf x
#else
#define BFS_DEBUG(x) /* NULL */
#endif

/**
 * The structure of the binary file system pointer.
 */
typedef struct s_bfs_fp
{
    /* The file pointer. */
    FILE *fp;
    /* The base offset */
    off_t base_offset;
    /* The current file offset. */
    off_t offset;
    /* The number of bytes written. This should be the same as offset. */
    off_t written;
    /* The first node root directory */
    off_t root_node;
    /* The first free node or zero if there is no free space. */
    off_t free_node;
    /* The total number of bytes used. */
    off_t numbytes;
    /* The estimated length of a offset in bytes. */
    off_t offset_bytes;
    /* The actual length of the archive. */
    off_t actual_len;
} bfsfp_t;

/**
 * @struct s_binfs_attribute
 * Structure of an attribute.
 */
typedef struct s_bfs_attribute
{
    /** The next attribute */
    struct s_bfs_attribute *next;
    /** Identifier associated with the attribute */
    unsigned char tag;
    /** The length of the attribute. */
    size_t len;
    /** The data associated with the attribute. */
    unsigned char data[1];
} bfsAttribute_t;

/**
 * @struct s_binfs_node
 * Define the in-memory basic structure of a node.
 */
typedef struct s_bfs_node
{
    /** The type of node */
    unsigned char type;
    /** The offset of the node. */
    off_t offset;
    /** The node name */
    char *name;
    /** The length of the name */
    size_t name_len;
    /** File system name relative path. */
    char *fsname;
    /** Parent node. */
    struct s_bfs_node *parent;
    /** Next sibling node */
    struct s_bfs_node *sibling;
    /** Next child node */
    struct s_bfs_node *child;
    /** The payload name of the node defined in the payload section */
    unsigned char *data;
    /** The encoded version of the data */
    unsigned char *encode;
    /** The length of the data. */
    off_t data_len;
    /** The length of the encoded data. */
    off_t encode_len;
    /** The name encoding */
    unsigned char encode_type;
    /** Pointer to the list of name attributes */
    struct s_bfs_attribute *attribute;
} bfsNode_t;

/**
 * Write out 'n' bytes of data.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] value A integer value to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutn (bfsfp_t *bfsfp, off_t value, int len)
{
    unsigned char buf[8];
    int ii;
    int written = 0;

    if ((ii = len) > 0)
    {
        /* Put out the integer backwards */
        do
        {
            --ii;
            buf [ii] = (unsigned char)(value & 0xff);
            value = value >> 8;
        }
        while (ii > 0);

        /* Put out the bytes MSB first. */
        for (ii = 0; ii < len; ii++)
            if (fputc (buf[ii] , bfsfp->fp) != EOF)
                written++;
        bfsfp->written += written;
        bfsfp->offset += len;
    }
    /* Return the status */
    return len;
}

#define bfsfpPut8(bfsfp,ii)   bfsfpPutn (bfsfp, ii, 1)
#define bfsfpPut16(bfsfp,ii)  bfsfpPutn (bfsfp, ii, 2)
#define bfsfpPut24(bfsfp,ii)  bfsfpPutn (bfsfp, ii, 3)
#define bfsfpPut32(bfsfp,ii)  bfsfpPutn (bfsfp, ii, 4)
#define bfsfpPut64(bfsfp,ii)  bfsfpPutn (bfsfp, ii, 8)

/**
 * Write out 'n' bytes of static data.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] c     The character to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutc (bfsfp_t *bfsfp, unsigned char c, int len)
{
    int written = 0;

    if (len > 0)
    {
        int ii;

        for (ii = 0; ii < len; ii++)
            if (fputc (c , bfsfp->fp) != EOF)
                written++;
        bfsfp->written += written;
        bfsfp->offset += len;
    }
    /* Return the status */
    return len;
}

/**
 * Write out a vuimsbf value.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] value A integer value to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutv (bfsfp_t *bfsfp, off_t value)
{
    unsigned char buf[8];               /* Temporary accumulation buffer */
    unsigned char *p = &buf[7];         /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int count = 0;                      /* Count of number of bytes. */
    int written = 0;                    /* Number of bytes written */

    /* Reverse process the string. */
    do
    {
        /* See if the value is zero. */
        if ((value & ~0x1f) == 0)
            break;
        /* Integer exceeds 0x1f so add a byte */
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
        count++;
    }
    while (count < 7);
    /* Add any residue information and set up the counter. Increment the
     * counter as there is always at least 1 byte. */
    *p = (unsigned char)(((count & 0x7) << 5) | (value & 0x1f));
    count++;

    /* Put out the bytes MSB first. */
    for (ii = 0; ii < count; ii++, p++)
        if (fputc (*p , bfsfp->fp) != EOF)
            written++;
    bfsfp->written += written;
    bfsfp->offset += count;

    /* Return the status */
    return count;
}

/**
 * Write out a fixed length vuimsbf value at a given length.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] value A integer value to write.
 * @param [in] len   The length of the data to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutvn (bfsfp_t *bfsfp, off_t value, int len)
{
    unsigned char buf[8];                     /* Temporary accumulation buffer */
    unsigned char *p = &buf[7];               /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int written = 0;                    /* Number of bytes written */

    /* Truncate the word. */
    len = (len - 1) & 7;

    /* Construct the bytes MSB first. */
    for (ii = 0; ii < len; ii++)
    {
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
    }
    *p = (unsigned char)((value & 0x1f) | ((len << 5) & 0xe0));

    /* Write out the bytes */
    for (ii = 0; ii <= len; ii++, p++)
        if (fputc (*p , bfsfp->fp) != EOF)
            written++;

    /* Update the offsets. */
    bfsfp->written += written;
    bfsfp->offset += len + 1;

    /* Return the status */
    return len + 1;
}

/**
 * Find the length of vuimsbf value
 *
 * @param [in] value A integer value to determine the storage length.
 *
 * @return The number of bytes required.
 */
static int
bfsfpLenv (off_t value)
{
    int len = 1;                        /* Must be at least 1-byte */

    /* Reverse process the string. */
    while ((value & ~0x1f) != 0)
    {
        value = value >> 8;
        len++;
    }

    /* Return the status */
    return len;
}

/**
 * Writes a file given a name to the archive.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] name  The name of the file to insert.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutfn (bfsfp_t *bfsfp, char *name)
{
    int written = 0;
    int count = 0;

    if (name != NULL)
    {
        FILE *fp;

        /* Open the file for reading. */
        if ((fp = fopen (name, "rb")) != NULL)
        {
            unsigned char buf [1024];
            int n;

            do
            {
                /* Read in a chunk. */
                n = fread (buf, 1, sizeof (buf), fp);
                if (n > 0)
                {
                    count += n;
                    written += fwrite (buf, 1, n, bfsfp->fp);
                }
            }
            while (n == sizeof (buf));

            /* Close the file. */
            fclose (fp);

            /* Increment the counters. */
            bfsfp->written += written;
            bfsfp->offset += count;

            BFS_DEBUG (("Read \"%s\": read=%d, written=%d\n", name, count, written));
        }
    }

    /* Return the number of bytes we read. */
    return count;
}

/**
 * Writes a file given a name to the archive.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] fp    The file pointer to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutfp (bfsfp_t *bfsfp, FILE *fp)
{
    int written = 0;
    int count = 0;

    if (fp != NULL)
    {
        unsigned char buf [1024];
        int n;

        do
        {
            /* Read in a chunk. */
            n = fread (buf, 1, sizeof (buf), fp);
            if (n > 0)
            {
                count += n;
                written += fwrite (buf, 1, n, bfsfp->fp);
            }
        }
        while (n == sizeof (buf));

        /* Increment the counters. */
        bfsfp->written += written;
        bfsfp->offset += count;

        BFS_DEBUG (("Read FP: read=%d, written=%d\n", count, written));
    }

    /* Return the number of bytes we read. */
    return count;
}

/**
 * Writes a block of data to the archive.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] buf   The buffer to write.
 * @param [in] len   The length of data to write.
 *
 * @return The number of bytes written.
 */
static int
bfsfpPutbuf (bfsfp_t *bfsfp, unsigned char *buf, int len)
{
    int written = 0;
    int count = 0;

    if ((buf != NULL) && (len > 0))
    {
        written += fwrite (buf, 1, len, bfsfp->fp);

        /* Increment the counters. */
        bfsfp->written += written;
        bfsfp->offset += len;
    }

    /* Return the number of bytes we read. */
    return len;
}

/**
 * Write out a vuimsbf value to a buffer.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] value A integer value to write.
 *
 * @return The number of bytes written.
 */
static int
bufPutv (unsigned char *buf, off_t value)
{
    unsigned char tmp[8];                     /* Temporary accumulation buffer */
    unsigned char *p = &tmp[7];               /* Point to last byte in the buffer. */
    int ii;                             /* Temporary loop counter. */
    int count = 0;                      /* Count of number of bytes. */
    int written = 0;                    /* Number of bytes written */

    /* Reverse process the string. */
    do
    {
        /* See if the value is zero. */
        if ((value & ~0x1f) == 0)
            break;
        /* Integer exceeds 0x1f so add a byte */
        *p-- = (unsigned char)(value & 0xff);
        value = value >> 8;
        count++;
    }
    while (count < 7);
    /* Add any residue information and set up the counter. Increment the
     * counter as there is always at least 1 byte. */
    *p = (unsigned char)(((count & 0x7) << 5) | (value & 0x1f));
    count++;

    /* Put out the bytes MSB first. */
    for (ii = 0; ii < count; ii++, p++)
        *buf++ = *p;

    /* Return the status */
    return count;
}

/**
 * Write out 'n' bytes of data.
 *
 * @param [in] buf The destination buffer.
 * @param [in] value A integer value to write.
 * @param [in] len   The number of bytes to write.
 *
 * @return The number of bytes written.
 */
static int
bufPutn (unsigned char *buf, off_t value, int len)
{
    unsigned char tmp[8];
    int ii;

    if ((ii = len) > 0)
    {
        /* Put out the integer backwards */
        do
        {
            --ii;
            tmp [ii] = (unsigned char)(value & 0xff);
            value = value >> 8;
        }
        while (ii > 0);

        /* Put out the bytes MSB first. */
        for (ii = 0; ii < len; ii++)
            *buf++ = tmp [ii];
    }
    /* Return the status */
    return len;
}

/**
 * Read in a single byte of data.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 *
 * @return The byte read
 */
static int
bfsfpGetc (bfsfp_t *bfsfp)
{
    return fgetc (bfsfp->fp);
}

/**
 * Get a vuimsbf value
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 *
 * @return The number of bytes written.
 */
static off_t
bfsfpGetv (bfsfp_t *bfsfp)
{
    int cc;
    int ii;
    off_t value = 0;

    if ((cc = fgetc (bfsfp->fp)) != EOF)
    {
        value = (off_t)(cc & 0x1f);
        ii = (cc >> 5) & 0x7;
        while (--ii >= 0)
        {
            cc = fgetc (bfsfp->fp);
            value = (value << 8) | (off_t)(cc & 0xff);
        }
    }
    /* Return the value */
    return value;
}

/**
 * Construct a new node.
 *
 * @return A pointer to the new node or NULL on an error.
 */
static bfsNode_t *
nodeConstruct (void)
{
    bfsNode_t *np;

    /* Construct the node */
    if ((np = calloc (1, sizeof (bfsNode_t))) != NULL)
    {
        /* No significant initialisation. If there is any then add it here. */
        ;
    }

    /* Return the new node. */
    return np;
}

/**
 * Perform a pre-order traversal and get the next node.
 *
 *   >       0       Level 0
 *   >      /|\
 *   >     1 2 3     Level 1
 *   >      / \ \
 *   >     4   5 6   Level 2
 *   >    / \   \
 *   >   7   8   9   level 3
 *
 * We expect the following:-
 *
 * Pre-order:  0, 1, 2, 4, 7, 8, 5, 9, 3, 6, e
 * In-order:   1, 0, 7, 4, 8, 2, 9, 5, 6, 3, e
 * Post-order: 1, 7, 8, 4, 9, 5, 2, 6, 3, 0, e
 *
 * @param [in] node The starting node.
 *
 * @return The next pre-order node or NULL if there is no next.
 */

static bfsNode_t *
nodePreOrderNext (bfsNode_t *node)
{
    int up = 0;

    while (node != NULL)
    {
        bfsNode_t *p;

        if ((up == 0) && ((p = node->child) != NULL))
            return p;
        else if ((p = node->sibling) != NULL)
            return p;
        node = node->parent;
        up = 1;
    }
    return node;
}

/**
 * The first node of the traversal given by the leftmost child of the tree.
 * DIRECTORIES ONLY.
 *
 * @param [in] node The root node.
 *
 * @return The first child node.
 */
static bfsNode_t *
nodePostOrderFirst (bfsNode_t *node)
{
    bfsNode_t *np;

    /* Descent to the leftmost child */
    while ((np = node->child) != NULL)
        node = np;
    return node;
}

/**
 * Perform a post-order traversal and get the next node.
 *
 *   >       0       Level 0
 *   >      /|\
 *   >     1 2 3     Level 1
 *   >      / \ \
 *   >     4   5 6   Level 2
 *   >    / \   \
 *   >   7   8   9   level 3
 *
 * We expect the following:-
 *
 * Pre-order:  0, 1, 2, 4, 7, 8, 5, 9, 3, 6, e
 * In-order:   1, 0, 7, 4, 8, 2, 9, 5, 6, 3, e
 * Post-order: 1, 7, 8, 4, 9, 5, 2, 6, 3, 0, e
 *
 * @param [in] node The starting node.
 *
 * @return The next post-order node or NULL if there is no next.
 */

static bfsNode_t *
nodePostOrderNext (bfsNode_t *node)
{
    int up = 0;

    do
    {
        bfsNode_t *np;

        /* Decend to the leaf node if possible */
        if ((np = node->sibling) != NULL)
        {
            while ((node = np->child) != NULL)
                np = node;
            return np;
        }
        /* Ascend to the parent */
        else if (up == 0)
            return node->parent;
        /* Mark that we have moved */
        up = 1;
    }
    while ((node = node->parent) != NULL);
    return node;
}

/**
 * Destruct a node. Note that this does not perform an unlink,
 *
 * @param [in] node The node to be destructed.
 */
static void
nodeDestruct (bfsNode_t *node)
{
    if (node != NULL)
    {
        bfsAttribute_t *ap;

        /* Free off the locally allocated memory */
        if (node->name != NULL)
            free (node->name);
        if (node->fsname != NULL)
            free (node->fsname);
        /* Destruct the data payloads. */
        if (node->data != NULL)
            free (node->data);
        if (node->encode != NULL)
            free (node->encode);
        /* Destruct the attributes. */
        while ((ap = node->attribute) != NULL)
        {
            node->attribute = ap->next;
            free (ap);
        }
    }
}

/**
 * Link a child node to the parent and perform an insertion sort.
 *
 * @param [in] parent The parent node.
 * @param [in] child The child node to be linked.
 *
 * @return The status of the call.
 * @retval 0 The insertion was sucessful.
 * @retval 1 The parent does not exist.
 * @retval 2 The node is duplicated, no insertion performed.
 */
static int
nodeLinkChild (bfsNode_t *parent, bfsNode_t *child)
{
    bfsNode_t **npp;
    bfsNode_t *np;

    /* A few quick basic checks for sanity. */
    if (parent == NULL)
        return 1;
    if (child == NULL)
        return 0;

    /* Iterate over the list of children and insert. */
    for (npp = &parent->child; (np = *npp) != NULL; npp = &np->sibling)
    {
        int status;

        status = strcmp ((char *)(child->name), (char *)(np->name));
        if (status == 0)
            return 2;
        else if (status < 0)
            break;
    }

    /* Add the node */
    child->sibling = *npp;
    *npp = child;
    child->parent = parent;
    return (0);
}

/**
 * Create a new node as a child of an existing node. The node is addded as a
 * sibling link of the children and is alphabetically sorted.
 *
 * @param [in] parent The parent node.
 * @param [in] pathname The pathname of the file system object to add.
 * @param [in] name The name of the childnode to link.
 * @param [in] type The type of node to create i.e. BFS_TYPE_FILE.
 *
 * @return A pointer to the new child node that was created,
 * @retval NULL The call failed.
 * @retval !NULL The call was successful, pointer to the new child node.
 */
static bfsNode_t *
nodeAddChild (bfsNode_t *parent, char *pathname, char *name, unsigned char type)
{
    bfsNode_t *np;

    /* Construct the node */
    if ((np = nodeConstruct ()) != NULL)
    {
        /* Initialise the node. */
        np->type = type;
        np->name = strdup (name);
        np->name_len = strlen (name);
        np->fsname = strdup (pathname);

        /* Link the node to the parent. */
        if (nodeLinkChild (parent, np) != 0)
        {
            nodeDestruct (np);
            np = NULL;
        }
    }

    /* Return the node to the caller. */
    return np;
}

/**
 * Process the directory tree build the nodes. The base directory passed in
 * the call is recursivelly traversed to build up the list of files and
 * directories.
 *
 * @param [in] bfsfp The binary file system pointer. Used to accumulate
 *                   the file size only.
 * @param [in] node The root node to process.
 * @param [in] basedir The base directory.
 */
static int
processDirectory (bfsfp_t *bfsfp, bfsNode_t *node, char *basedir)
{
    int status = 0;
    DIR *base;
    struct dirent *dp;
    off_t estimate = 0;

    /* Process the directory */
    base = opendir (basedir);
    if (base == NULL)
    {
        printf ("Error opening \"%s\"\n", basedir);
        return (1);
    }

    /* Iterate over the node and process all of the entries */
    while ((dp = readdir (base)) != NULL)
    {
        struct stat statbuf;
        char pathname [PATH_MAX];

        BFS_DEBUG (("Processing: %s/%s\n",
                    strnil (basedir), strnil (dp->d_name)));

        /* Ignore any file with a null name. */
        if (dp->d_name == NULL)
            continue;
        /* Ignore the current directory */
        if (strcmp (dp->d_name, ".") == 0)
            continue;
        /* Ignore the parent directory */
        if (strcmp (dp->d_name, "..") == 0)
            continue;

        /* Construct the full path */
        strcpy (pathname, basedir);
        strcat (pathname, "/");
        strcat (pathname, dp->d_name);

        /* Find the basic attributes of the file. */
        if (stat (pathname, &statbuf) != -1)
        {
            bfsNode_t *np;
            unsigned char type = BFS_TYPE_UNDEF;
            off_t data_len = 0;

            BFS_DEBUG (("stat: %s = %d\n", strnil (pathname), statbuf.st_size));

            /* Find the type */
            if ((statbuf.st_mode & S_IFDIR) != 0)
            {
                type = BFS_TYPE_DIR;
                /* Assume 10-byte directory node */
                estimate += 10;
            }
            else if ((statbuf.st_mode & S_IFREG) != 0)
            {
                type = BFS_TYPE_FILE;
                data_len = statbuf.st_size;
            }
            if (type != BFS_TYPE_UNDEF)
            {
                /* Assume the node has a directory entry of 3 bytes + the
                 * length of the name + the length of the data */
                estimate += 10 + 3 + strlen (dp->d_name) + data_len;

                /* Add node to the construction tree. */
                if ((np = nodeAddChild (node, pathname, dp->d_name, type)) != NULL)
                {
                    np->data_len = data_len;
                    BFS_DEBUG (("Added: [%d] %s @ %d\n",
                                type, strnil (pathname), (int)(data_len)));
                }
                else
                {
                    printf ("ERROR adding: [%d] %s\n", type, pathname);
                    status |= 2;
                }
            }
        }
        else
        {
            printf ("Stat failed on [%d] \"%s\"\n", errno, strnil (pathname));
            perror ("Stat failed");
        }
    }

    closedir (base);

    /* Add the size estimate to the file system size. */
    if (bfsfp != NULL)
        bfsfp->offset += estimate;

    /* Tail recurse over the directories that we have created. The compiler
     * might be smart enough to work out that this is a tail recursion
     * operation and not stack it, but not to worry. */
    for (node = node->child; node != NULL; node = node->sibling)
        if (node->type == BFS_TYPE_DIR)
            status |= processDirectory (bfsfp, node, node->fsname);

    return status;
}

/**
 * Build the directory node.
 *
 * @param [in] node The directory node to build.
 */
static int
buildDirectory (bfsNode_t *node)
{
    bfsNode_t *np;
    int nn;
    int flags;
    int buf_size = 0;

    /* Iterate over all of the children. */
    for (np = node->child; np != NULL; np = np->sibling)
    {
        /* Work out the current type */
        if (np->type == BFS_TYPE_FILE)
            flags = 1;
        else if (np->type == BFS_TYPE_DIR)
            flags = 2;
        else
            continue;

        /* flags */
        buf_size += bfsfpLenv (flags);
        /* node */
        buf_size += bfsfpLenv (np->offset);
        /* name_len + name */
        buf_size += bfsfpLenv (np->name_len) + np->name_len;
    }

    /* Allocate the buffer. */
    if ((node->data = malloc (buf_size)) == NULL)
        node->data_len = 0;
    else
    {
        unsigned char *pp;

        /* Point to the start of the data. */
        pp = node->data;

        /* Iterate over all of the children. */
        for (np = node->child; np != NULL; np = np->sibling)
        {
            int ii;

            /* Current directory - node */
            pp += bufPutv (pp, np->offset);
            /* Current directory - name_len */
            pp += bufPutv (pp, np->name_len);
            /* Current directory - name */
            for (ii = 0; ii < np->name_len; ii++)
                *pp++ = np->name[ii];
            BFS_DEBUG (("Added node \"%s\" to directory\n", strnil (np->fsname)));
        }

        /* Set the data length. */
        node->data_len = pp - node->data;
        assert (node->data_len != 0);
        assert (node->data_len <= buf_size);
        BFS_DEBUG (("Dir Allocate = %p@%d ?%d\n",
                    node->data, (int)(node->data_len), buf_size));
    }

    /* Completed. */
    return 0;
}

/**
 * Compress a node to a temporary file to get the size.
 *
 * @param [in] node The node to process.
 *
 * @return A non-null file pointer to encode or NULL if the original data will
 * be encoded.
 */
static FILE *
nodeCompress (bfsNode_t *node)
{
#define WORKING_BUFLEN  (128*1024)

    FILE *ufp = NULL;
    FILE *cfp;
    unsigned char uworking[WORKING_BUFLEN];
    unsigned char *ubuf = uworking;
    size_t ulen = 0;
    unsigned char cworking[WORKING_BUFLEN];
    unsigned char *cbuf = cworking;
    size_t clen = 0;
    int rlen;
    int wlen;

    /* Assume that we are not compressing. */
    node->encode_type = BFS_ENCODE_NONE;
    /* If this is a file node then try to open the file and compress it. */
    if (node->type == BFS_TYPE_FILE)
    {
        /* Open the file. */
        if ((ufp = fopen (node->fsname, "rb")) != NULL)
        {
            /* Read in a buffer full */
            rlen = fread (ubuf, 1, WORKING_BUFLEN, ufp);
            if (rlen <= 10)
            {
                /* The file is short, return the read file pointer. */
                rewind (ufp);
                fprintf (stderr, "File is short skip compression \"%s\"\n", node->fsname);
                return ufp;
            }
            ulen = rlen;
        }
    }
    else if (node->type == BFS_TYPE_DIR)
    {
        /* This is a directory node. */
        if (node->data_len < 10)
            return ufp;                 /* Not worth compressing. */
        
        /* DO Not compress !! */
        return NULL;
    }
    else
        return NULL;

    /* At this point then we have some data that might be worth compressing.
     * Open a temporary file into which we will compress the data. */
    
#ifdef _WIN32
    remove (TMP_FILE);
    if ((cfp = fopen (TMP_FILE, "wb+")) != NULL)
#else
    if ((cfp = tmpfile ()) != NULL)
#endif
    {
        z_stream c_stream;              /* compression stream */
        int zstatus;                    /* Status of the compression. */
        int zflush;                     /* The flush type. */

        /* Initialise the compression structure */
        zflush = Z_NO_FLUSH;

        /* Initialise the deflate algorithm */
        c_stream.zalloc = Z_NULL;
        c_stream.zfree = Z_NULL;
        c_stream.opaque = Z_NULL;
        zstatus = deflateInit (&c_stream, Z_BEST_COMPRESSION);

        if (zstatus != Z_OK)
        {
            /* Cannot compress return the original file. Return the file
             * pointer to the start and return the file pointer. */
            printf ("Cannot compress \"%s\"\n", strnil (node->fsname));
            fclose (cfp);
            if (ufp != NULL)
                rewind (ufp);
            return ufp;                 /* Fault */
        }

        /* initialise the stream structure */
        c_stream.next_out = cbuf;
        c_stream.avail_out = WORKING_BUFLEN;
        if (node->type == BFS_TYPE_DIR)
        {
            c_stream.next_in = node->data;
            c_stream.avail_in = (int)(node->data_len);
            ulen = node->data_len;
        }
        else
        {
            c_stream.next_in = ubuf;    /* Uncompressed buffer. */
            c_stream.avail_in = (int)(rlen);
        }

        /* Repeat the compression by writing to the output file. */
        do
        {
            /* Deflate the input. */
            zstatus = deflate (&c_stream, zflush);
            if (zstatus == Z_OK)
            {
                /* Run out out input data. */
                if (c_stream.avail_in == 0)
                {
                    if (zflush == Z_NO_FLUSH)
                    {
                        /* For a valid file pointer then try to read some
                         * more data. */
                        if (ufp != NULL)
                            c_stream.avail_in = fread (ubuf, 1, WORKING_BUFLEN, ufp);
                        if (c_stream.avail_in == 0)
                            zflush = Z_FINISH;
                        else
                            ulen += c_stream.avail_in;
                        /* Reset the buffer pointer. */
                        c_stream.next_in = ubuf;
                    }
                }
                /* Check for no output. */
                if (c_stream.avail_out == 0)
                {
                    clen += fwrite (cbuf, 1, WORKING_BUFLEN, cfp);
                    c_stream.next_out  = cbuf;
                    c_stream.avail_out = WORKING_BUFLEN;
                }
            }
            /* Check if we are flushing the output. */
            else if (zstatus == Z_STREAM_END)
            {
                int len;

                len = WORKING_BUFLEN - c_stream.avail_out;
                BFS_DEBUG (("Z_STREAM_END: avail_out = %d, buflen = %d, len=%d\n",
                            c_stream.avail_out, WORKING_BUFLEN, len));

                if (len > 0)
                    clen += fwrite (cbuf, 1, len, cfp);
            }
        }
        while (zstatus == Z_OK);

        /* Check for the end of the stream. */
        if (zstatus == Z_STREAM_END)
        {
            /* End the compression and flush the output. */
            zstatus = deflateEnd (&c_stream);
            fflush (cfp);
        }
        /* Handle the final compression status */
        if (zstatus == Z_OK)
        {
            /* Make a decision as to what we are going to return. */
            BFS_DEBUG (("Compressed file \"%s\" %d=>%d (%d%%)\n",
                        strnil (node->fsname), ulen, clen, (ulen * 100) / clen));
            fprintf (stderr,
                     "Compressed file \"%s\" %d=>%d (%d%%)\n",
                     strnil (node->fsname), ulen, clen, (ulen * 100) / clen);
            
            if (((ulen * 100) / clen) > 110)
            {
                /* We are going to take the compressed version set up the
                 * node to convey zlib compression. */
                assert (c_stream.total_out == clen);
                node->encode_len = c_stream.total_out;
                node->encode_type = BFS_ENCODE_ZLIB;
                /* Close the uncompressed file */
                if (ufp != NULL)
                    fclose (ufp);
                /* Rewind the compressed file pointer back to the start and
                 * return the compressed file pointer. */
                fflush (cfp);
                rewind (cfp);
                return cfp;
            }
            else
            {
                BFS_DEBUG (("Use uncompressed \"%s\"\n", strnil (node->fsname)));
            }
        }
    }
    else
        fprintf (stderr, "Cannot open temporary file for compression on \"%s\"\n", node->fsname);
        

    /* Close the temporary compression stream. */
    if (cfp != NULL)
    {
        fclose (cfp);
#ifdef _WIN32
        remove (TMP_FILE);
#endif
    }
    
    /* Rewind the uncompressed file pointer. */
    if (ufp != NULL)
        rewind (ufp);
    return ufp;
}

/**
 * Write the node data to the file.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] node The node to write.
 */
static void
nodeWrite (bfsfp_t *bfsfp, bfsNode_t *node)
{
    off_t datum;
    int encoding;
    int aux_flag;
    FILE *fp;
    unsigned *dp;

    BFS_DEBUG (("nodeWrite: %s @ %d\n", strnil (node->fsname),
                (int)(node->data_len & 0xffffffff)));

    /* Save the current byte offset */
    node->offset = bfsfp->offset;

    /* Work out the current type; if it is a directory then build it. */
    if (node->type == BFS_TYPE_DIR)
    {
        buildDirectory (node);
        BFS_DEBUG (("nodeWrite[DIR]: %s @ %d\n",
                    strnil (node->fsname), (int)(node->data_len & 0xffffffff)));
    }

    /* Write the node block */
    fp = nodeCompress (node);

    /* leader */
    /* bfsfpPut8 (bfsfp, '@');*/

    /* attrib_flag */
    datum = (node->attribute == NULL) ? 0 : 0x80;
    /* encoding 0=none */
    encoding = (node->encode_type & 0x07) << 4;
    /* auxillary_flag */
    if ((node->type == BFS_TYPE_DIR) ||
        (node->type == BFS_TYPE_MOUNT))
        aux_flag = 0x08;
    else
        aux_flag = 0x00;
    /* type */
    datum |= encoding | aux_flag | (node->type & 0x07);
    bfsfpPut8 (bfsfp, datum);

    /* data_length */
    if (encoding == 0)
        bfsfpPutv (bfsfp, node->data_len);
    else
    {
        bfsfpPutv (bfsfp, node->encode_len);
        /* original_length */
        bfsfpPutv (bfsfp, node->data_len);
    }

    /* Add the auxillary data */
    if ((aux_flag != 0) &&
        ((node->type == BFS_TYPE_DIR) || (node->type == BFS_TYPE_MOUNT)))
    {
        /* Placeholder for the parent node */
        bfsfpPutvn (bfsfp, 0, bfsfp->offset_bytes);
    }

    /* attributes */
    if (node->attribute != NULL)
    {
        /* TODO: Need to handle the attributes. */
    }

    /* Put the data_bytes */
    if (fp != NULL)
    {
        datum = bfsfpPutfp (bfsfp, fp);
        fclose (fp);
    }
    else if ((node->data != NULL) && (node->data_len != 0))
        datum = bfsfpPutbuf (bfsfp, node->data, node->data_len);
    else if ((node->data == NULL) && (node->data_len != 0))
        datum = bfsfpPutc (bfsfp, 0, node->data_len);

    if (node->encode_type == BFS_ENCODE_NONE)
    {
        if (datum != node->data_len)
            printf ("ERROR: \"%s\" size mismatch %d!=%d\n",
                    strnil (node->fsname), (int)(datum), (int)(node->data_len));
    }
    else
    {
        if (datum != node->encode_len)
            printf ("ERROR: \"%s\" compressed size mismatch %d!=%d\n",
                    strnil (node->fsname), (int)(datum), (int)(node->encode_len));
    }

    /* Put the padding. */
    bfsfpPut8 (bfsfp, 0x0f);
    
    /* Clean up the compression file if it exists */
#ifdef _WIN32
    remove (TMP_FILE);
#endif    
}

/**
 * Write the node data to the file.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] node The node to write.
 */
static int
nodeWriteTree (bfsfp_t *bfsfp, bfsNode_t *node)
{
    off_t oo;                           /* Original offset */
    bfsNode_t *np;                      /* Temporary node pointer */

    BFS_DEBUG (("In node write tree\n"));

    /* Perform a post-order traversal over the node tree and establish the
     * names as package data. */
    for (np = nodePostOrderFirst (node);
         (np != NULL) && (np->type != BFS_TYPE_MOUNT);
         np = nodePostOrderNext (np))
    {
        nodeWrite (bfsfp, np);
    }

    /* Set up the root node entry point */
    np = node->child;
    if (np != NULL)
        bfsfp->root_node = np->offset;

    /* Add the trailer. */
    bfsfpPut8 (bfsfp, 'B');
    bfsfpPut8 (bfsfp, 'F');
    bfsfpPut8 (bfsfp, 'S');
    bfsfpPutv (bfsfp, bfsfp->written);

    BFS_DEBUG (("Root_node is at offset %d(0x%08x)\n",
                (int)(bfsfp->root_node), (int)(bfsfp->root_node)));
    return 0;
}

/**
 * Build the mount point block.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] node The root node to write.
 */
static void
nodeWriteHeader (bfsfp_t *bfsfp, bfsNode_t *root)
{
    /* output the header. */
    bfsfpPut8 (bfsfp, 'b');
    bfsfpPut8 (bfsfp, 'f');
    bfsfpPut8 (bfsfp, 's');

    /* Allocate the root data */
    if ((root->data = malloc (root->data_len)) != NULL)
    {
        time_t curr_time;               /* The current time. */
        struct tm *curr_gmttime;        /* Time structure */
        unsigned char *p;
        int ii;

        p = root->data;
        /* version_major */
        *p++ = (unsigned char) bfs_version_major;
        /* version_minor */
        *p++ = (unsigned char) bfs_version_minor;
        /* version_micro */
        *p++ = (unsigned char) bfs_version_micro;
        /* reserved */
        *p++ = 0;

        /* Fill in the time. */
        curr_time = time (NULL);
        if ((curr_gmttime = gmtime (&curr_time)) != NULL)
        {
            /* Year MSB 8 bits */
            ii = curr_gmttime->tm_year + 1900;
            *p++ = (ii >> 4) & 0xff;
            /* Year LSB 4 bits and month */
            *p++ = ((ii << 4) & 0xf0) | (curr_gmttime->tm_mon + 1);
            /* day */
            *p++ = (curr_gmttime->tm_mday << 2) & 0xfc;
            /* hour */
            *p++ = curr_gmttime->tm_hour;
            /* minute */
            *p++ = curr_gmttime->tm_min;
            /* second */
            *p++ = curr_gmttime->tm_sec;
        }
        else
        {
            /* Zero fill. */
            for (ii = 0; ii < 6; ii++)
                *p++ = 0;
        }
        /* free_node - none here so reset. */
        *p++ = 0;
        /* length - fill in later */
        for (ii = 0; ii < bfsfp->offset_bytes; ii++)
        {
            if (ii == 0)
                *p++ = ((bfsfp->offset_bytes - 1) & 7) << 5;
            else
                *p++ = 0;
        }
        /* crc32_data */
        for (ii = 0; ii < 4; ii++)
            *p++ = 0xdd;
        /* crc32_header */
        for (ii = 0; ii < 4; ii++)
            *p++ = 0xcc;

        /* Re-evaluate the data length */
        root->data_len = p - root->data;
    }

    /* Add the mount point node */
    nodeWrite (bfsfp, root);
}

/**
 * Fix the directory nodes ".." offset. Perform an in-order traversal of the
 * tree and fix the nodes.
 *
 * @param [in] bfsfp The bfsfp file descriptor.
 * @param [in] node The root node of the tree.
 */
static void
nodeFix (bfsfp_t *bfsfp, bfsNode_t *root)
{
    bfsNode_t *np;
    off_t crc32data_off = 0;
    off_t crc32head_off = 0;

    /* Flush all of the write data out. */
    fflush (bfsfp->fp);
    rewind (bfsfp->fp);

    /* Iterate over the nodes. */
    for (np = root; np != NULL; np = nodePreOrderNext (np))
    {
        /* Ignore files. */
        if (np->type == BFS_TYPE_FILE)
            continue;
        if (np->type == BFS_TYPE_MOUNT)
        {
            /* Seek to the node position. */
            if (fseeko (bfsfp->fp, bfsfp->base_offset + 3, SEEK_SET) == 0)
            {
                int status;
                off_t datalen;
                off_t aux_flag_off;
                off_t arch_len_off;

                /* Get the type */
                status = fgetc (bfsfp->fp);
                /* Data length */
                datalen = bfsfpGetv (bfsfp);
                /* original_length */
                if ((status & 0x70) != 0)
                    datalen = bfsfpGetv (bfsfp);
                /* Auxillary flag */
                if ((status & 0x08) != 0)
                {
                    aux_flag_off = ftello (bfsfp->fp);
                    datalen = bfsfpGetv (bfsfp);
                }
                else
                    aux_flag_off = 0;
                /* attribute flag */
                if ((status & 0x80) != 0)
                {
                    /* Skip the attribute data */
                    datalen = bfsfpGetv (bfsfp);
                    fseeko (bfsfp->fp, datalen, SEEK_CUR);
                }
                /* Within the data - skip version and time */
                fseeko (bfsfp->fp, 10, SEEK_CUR);
                /* Skip the free_node */
                datalen = bfsfpGetv (bfsfp);
                /* Save the archive length offset and then skip */
                arch_len_off = ftello (bfsfp->fp);
                datalen = bfsfpGetv (bfsfp);
                /* Save the crc32 data and header offsets */
                crc32data_off = ftello (bfsfp->fp);
                if (crc32data_off > 0)
                    crc32head_off = crc32data_off + 4;

                /* Re-write the aux flag */
                if (aux_flag_off > 0)
                {
                    /* Get the position of the file. */
                    status = fseeko (bfsfp->fp, aux_flag_off, SEEK_SET);
                    assert (status == 0);
                    bfsfpPutvn (bfsfp, bfsfp->root_node, bfsfp->offset_bytes);
                    fflush (bfsfp->fp);
                }
                /* Re-write the archive length */
                if (arch_len_off > 0)
                {
                    /* Get the position of the file. */
                    status = fseeko (bfsfp->fp, arch_len_off, SEEK_SET);
                    assert (status == 0);
                    bfsfpPutvn (bfsfp, bfsfp->actual_len, bfsfp->offset_bytes);
                    fflush (bfsfp->fp);
                }
            }
        }
        else if (np->type == BFS_TYPE_DIR)
        {
            /* Seek to the node position. */
            if (fseeko (bfsfp->fp, bfsfp->base_offset + np->offset, SEEK_SET) == 0)
            {
                int status;
                off_t datalen;

                /* Get the type */
                status = fgetc (bfsfp->fp);
                /* Data length */
                datalen = bfsfpGetv (bfsfp);
                /* original_length */
                if ((status & 0x70) != 0)
                    datalen = bfsfpGetv (bfsfp);
                /* Re-write the aux flag */
                if ((status & 0x08) != 0)
                {
                    size_t pos;

                    if (np->parent != NULL)
                        datalen = np->parent->offset;
                    else
                        datalen = 0;

                    pos = ftell (bfsfp->fp);
                    assert (pos > 0);
                    status = fseeko (bfsfp->fp, pos, SEEK_SET);
                    assert (status == 0);
                    bfsfpPutvn (bfsfp, datalen, bfsfp->offset_bytes);
                    fflush (bfsfp->fp);
                }
            }
        }
    }

    /* Compute the CRC-32 for the data. */
    if (crc32head_off != 0)
    {
        /* Advance to the byte after the CRC-32 checksum */
        if (fseeko (bfsfp->fp, crc32head_off + 4, SEEK_SET) == 0)
        {
            unsigned char buf [2048];
            bfsuint32_t crc32 = 0xffffffff;
            int nread;
            
            /* Read in all of the data until we are finished */
            do
            {
                nread = fread (buf, 1, sizeof (buf), bfsfp->fp);
                if (nread > 0)
                    crc32 = bfs_crc32 (crc32, buf, nread);
            }
            while (nread > 0);

            /* Fill in the data CRC-32 */
            if (fseeko (bfsfp->fp, crc32data_off, SEEK_SET) == 0)
            {
                bfsfpPutn (bfsfp, crc32, 4);
                fflush (bfsfp->fp);
            }
        }

        /* Compute the CRC-32 for the header. */
        if (fseeko (bfsfp->fp, bfsfp->base_offset, SEEK_SET) == 0)
        {
            unsigned char buf [2048];
            bfsuint32_t crc32 = 0xffffffff;
            int remain;
            int nread;

            /* Compute the number of remaining bytes in the header. */
            remain = crc32head_off - bfsfp->base_offset;

            /* Read in all of the data until we are finished */
            do
            {
                nread = sizeof (buf);
                if (remain < nread)
                    nread = remain;

                nread = fread (buf, 1, nread, bfsfp->fp);
                if (nread > 0)
                {
                    remain -= nread;
                    crc32 = bfs_crc32 (crc32, buf, nread);
                }
            }
            while ((nread > 0) && (remain > 0));

            /* Fill in the data CRC-32 */
            if (fseeko (bfsfp->fp, crc32head_off, SEEK_SET) == 0)
                bfsfpPutn (bfsfp, crc32, 4);
        }
    }
}

/**
 * Construct a BFS archive.
 *
 * @param [in] options  The option mask.
 * @param [in] fp The file pointer to write.
 * @param [in] direname The directory to process.
 *
 * @return Status of the call, 0 is success.
 */
int
bfs_build (int options, FILE *fp, char *dirname)
{
    int status;
    bfsNode_t *root;                    /* The root node. */
    off_t offset;
    bfsfp_t bfsfp;
    int written;

    /* Keep user up to date */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Building from directory \"%s\"\n", strnil (dirname));

    /* Create the mount node */
    root = nodeConstruct ();
    if (root != NULL)
    {
        bfsNode_t *np;

        /* "mount" data block size */
        root->data_len = 30;            /* Maximum size */
        root->type = BFS_TYPE_MOUNT;

        /* Create the root directory. */
        if ((np = nodeConstruct ()) != NULL)
        {
            root->child = np;
            np->parent = root;
            np->type = BFS_TYPE_DIR;
        }
    }

    /* Open up the temporary bfsfp file to write. */
    memset ((char *)(&bfsfp), 0, sizeof (bfsfp));

    /* Process the directory. */
    if ((root != NULL) && (root->child != NULL))
    {
        status = processDirectory (&bfsfp, root->child, dirname);
        /* Calculate the offset size estimate */
        bfsfp.offset_bytes = bfsfpLenv (bfsfp.offset << 1);
        BFS_DEBUG (("Size estimate = %d @ %d bytes\n",
                    (int)(bfsfp.offset), bfsfp.offset_bytes));
        /* Reset the offset field. */
        bfsfp.offset = 0;
    }

    /* Seek to the end of the file pointer and get the base offset. */
    if (fseeko (fp, 0, SEEK_END) != 0)
    {
        fprintf (stderr, "Error: Cannot seek to end of archive file.\n");
        return (-1);
    }

    /* Get the offset of the start of the archive */
    bfsfp.base_offset = ftello (fp);
    if (bfsfp.base_offset < 0)
    {
        fprintf (stderr, "Error: Cannot get writing offset\n");
        return (-2);
    }
    if (bfsfp.base_offset != 0)
        fprintf (stderr, "Archive commencing from byte offset %d\n",
                 (int)(bfsfp.base_offset));
    bfsfp.fp = fp;

    /* Build the header. */
    nodeWriteHeader (&bfsfp, root);

    /* Build the node name */
    nodeWriteTree (&bfsfp, root);
    bfsfp.actual_len = bfsfp.written;

    /* Go back and fix the tree */
    nodeFix (&bfsfp, root);

    /* Print out the information. */
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Archive constructed %d bytes\n", (int)(bfsfp.actual_len));
    if ((options & BFSUTIL_OPT_QUIET) == 0)
        fprintf (stderr, "Completed archive build from \"%s\" [%s]\n",
                 strnil (dirname), strstatus (0));
    return 0;
}
