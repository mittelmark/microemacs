/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * dirlist.c - System directory tree display routines.
 *
 * Copyright (c) 1996-2001 Jon Green & Steven Phillips
 * Copyright (C) 2002-2009 JASSPA (www.jasspa.com)
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
 * Created:     1996
 * Synopsis:    System directory tree display routines.
 * Authors:     Jon Green & Steven Phillips
 * Description:
 *     This file contains routines to construct and display the system
 *     file structure.
 */

#define	__DIRC	1			/* Define the filename */

#include "emain.h"
#include "efunc.h"

#if MEOPT_DIRLIST

#include	<time.h>
#if (defined _UNIX) || (defined _DOS) || (defined _WIN32)
#include	<errno.h>
#include        <limits.h>                     /* Constant limit definitions */
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef _WIN32
#include        <direct.h>                     /* Directory entries */
#else
#ifdef _DIRENT
#include	<dirent.h>
#else
#include	<sys/dir.h>
#endif
#endif
#include	<time.h>
#ifdef _DOS
#include <dos.h>
#endif
#endif

#define DEBUG 0
#if DEBUG
FILE *fp = NULL;
#endif

#define DIR_UNREAL            0x01      /* Node is not real */
#define DIR_UNKNOWN           0x02      /* Node is not known */
#define DIR_HIDDEN            0x04      /* Node is hidden */
#define DIR_GONE              0x08      /* Node has gone */

#define LDO_GETPATH           0x01      /* Get path from the user */
#define LDO_SELECT            0x02      /* Select node */
#define LDO_EVAL              0x04      /* Do evaluation */
#define LDO_OPEN              0x08      /* Open node */
#define LDO_CLOSE             0x10      /* Close node */
#define LDO_TOGGLE            0x20      /* Toggle Open/Close state */
#define LDO_FORCE             0x40      /* Force re-evaluation */
#define LDO_RECURSE           0x80      /* Recursive evaluation */

typedef struct DIRNODE {
    struct DIRNODE *parent;             /* Pointer to the parent (previous level) */
    struct DIRNODE *next;               /* Pointer to sibling (same level) */
    struct DIRNODE *lnext;              /* Pointer to link next */
    struct DIRNODE *lhead;              /* Pointer to link head */
    struct DIRNODE *child;              /* Pointer to the child (next level) */
    meShort  mask;                        /* Status mask */
    meUByte *lname;                       /* Symbolic link name - not nice here
                                         * will fold into the dname soonest */
    meUByte  dname[1];                    /* Name of the directory */
} DIRNODE;

static DIRNODE  *dirlist;                /* Directory list root */
static meLine     *curDirLine;             /* Current dir node */


/*
 * dirConstructNode
 * Construct a new node.
 */
static DIRNODE *
dirConstructNode (meUByte *name, int mask)
{
    DIRNODE *dnode;

    if((dnode = (DIRNODE *) meMalloc(sizeof(DIRNODE) + meStrlen(name))) != NULL)
    {
        dnode->mask = mask;
        dnode->next = NULL;
        dnode->lhead = NULL;
        dnode->lnext = NULL;
        dnode->child = NULL;
        dnode->lname = NULL;
        dnode->parent = NULL;
        meStrcpy (dnode->dname, name);
    }
    return (dnode);
}

/*
 * dirDestructNode
 * Destruct an existing node
 */
static void
dirDestructNode(DIRNODE *dnode)
{
    DIRNODE *dn, *ndn ;
    if(dnode->lname)
    {
        if((dn = dnode->child) != NULL)
        {
            if(dn->lhead == dnode)
                dn->lhead = dnode->lnext ;
            else
            {
                dn = dn->lhead ;
                while(dn->lnext != dnode)
                    dn = dn->lnext ;
                dn->lnext = dnode->lnext ;
            }
        }
        meFree(dnode->lname);
    }
    if(dnode->lhead != NULL)
    {
        dn = dnode->lhead ;
        while(dn != NULL)
        {
            ndn = dn->lnext ;
            dn->child = NULL ;
            dn->lnext = NULL ;
            dn = ndn ;
        }
    }
    meFree(dnode);
}

/*
 * deleteTree
 * Delete a all of a nodes children
 */
static void
dirDeleteTree(DIRNODE *root)
{
    DIRNODE *dnode;                     /* Working node pointer */
    DIRNODE *dt;                        /* Temporary node */

    /* Recursively iterate over the children and delete */
    for (dnode = root->child; dnode != NULL; /* NULL */)
    {
        if((dnode->child != NULL) && (dnode->lname == NULL))
            dirDeleteTree (dnode);
        dt = dnode;                     /* Remember node to delete */
        dnode = dnode->next ;           /* Point to next node */
        dirDestructNode(dt) ;           /* Delete the node */
    }
    root->child = NULL;
}

/*
 * deleteNode
 * Delete a node and all of it's children
 */
static void
dirDeleteNode(DIRNODE *dnode)
{
    DIRNODE *dparent;                   /* Directory parent */
    DIRNODE *dp;                        /* General directory pointer */

    if ((dnode == NULL) || (dnode == dirlist))
        return;
    
    if(dnode->lname == NULL)
        /* not a link so kill it */
        dirDeleteTree(dnode);                 /* Delete any children */
    /* Delete the node itself */
    dparent = dnode->parent;
    dp = dparent->child;
    if (dp == dnode)
        dparent->child = dp->next;   /* Link parent to next child */
    else
    {
        while(dp->next != dnode)
            dp = dp->next ;
        dp->next = dnode->next;
    }
    dirDestructNode(dnode) ;           /* Kill off the node */
}

/*
 * dirFindNode
 * Find a node with name 'dname' in the node list.
 */
static DIRNODE *
dirFindNode (DIRNODE *root, meUByte *dname)
{
    DIRNODE *dnode;                     /* Working node pointer */
    int status;                         /* Comparison status */
#if DEBUG    
    fprintf (fp, "dirFindNode: root=%s, dname=%s\n",
             (root==NULL) ? "NULL" : root->dname,
             (dname==NULL) ? "NULL" : dname); 
#endif
    /* Iterate down the next list. The list is alphabetically sorted */
    for(dnode=root->child; dnode != NULL; dnode = dnode->next)
    {
        if ((status = meStrcmp(dname, dnode->dname)) < 0)
            break;
        else if (status == 0)
        {
#if DEBUG
            fprintf (fp, "Found\n");
            fflush (fp);
#endif
            return dnode ;             /* Return pointer to node found */
        }
    }
#if DEBUG
    fprintf(fp, "Not found\n");
    fflush(fp);
#endif
    return NULL;                        /* Not found */
}

/*
 * dirLinkNode
 * Add a new node to a node. We assume that the caller has already performed
 * a dirFindNode and the node is not in the list, we do not check for
 * equality here.
 */
static DIRNODE *
dirLinkNode (DIRNODE *root, DIRNODE *dnode)
{
    DIRNODE *dprev;
    DIRNODE *dnext;

#if DEBUG
    fprintf (fp, "dirLinkNode: root=%s, dname=%s\n",
             (root==NULL) ? "NULL" : root->dname,dnode->dname); 
    fflush (fp);
#endif
    /* Link to the child if it is NULL */
    if(((dprev = root->child) == NULL) ||
        (meStrcmp (dnode->dname, dprev->dname) < 0))
    {
        dnode->next = dprev;         /* Link nexts */
        root->child = dnode;            /* Link to root as child */
    }
    else
    {
        /* Add the new node */
        for(dnext = dprev->next; dnext != NULL; dprev = dnext, dnext = dnext->next)
            if (meStrcmp (dnode->dname, dnext->dname) < 0)
                break;
        
        /* Link to the previous node */
        dnode->next = dprev->next;
        dprev->next = dnode;
    }
    dnode->parent = root;           /* Form parent linkage */
    
    return dnode ;
}

/* Give the path of:-
 * 
 * dos        c:/xxxx/yyyyy
 * unix       /xxxx/yyyyy
 * 
 * it adds the path to the tree, as soon as a dir is encountered which
 * doesn't exist it returns NULL, else returns a pointer to the last node.
 * 
 * NOTE: on unix symbolic links are coped with and the path also added,
 *       so function can be recursive.
 */
static DIRNODE *
addLinkPath(DIRNODE *dnode, meUByte *pathname) ;

static DIRNODE *
addPath(meUByte *pathname, int mask)
{
    DIRNODE *dp, *ndp ;
    meUByte   *p, *q, cc ;
    
    if(isUrlLink(pathname))
        return NULL ;

    /* Construct the root directory node if it does not already exist */
    if (dirlist == NULL)
    {
#if DEBUG
        fp = fopen ("me.out", "wb");
#endif
        dirlist = dirConstructNode ((meUByte *)"Desktop ", DIR_UNREAL|DIR_UNKNOWN);
    }

    dp = dirlist ;
    p = pathname ;
    for(; (q=meStrchr(p,DIR_CHAR)) != NULL ; dp = ndp, p = q)
    {
        cc = *++q ;
        *q = '\0' ;
        while(dp->lname != NULL)
            dp = dp->child ;
        /* see if the path already exists */
        if((ndp = dirFindNode(dp,p)) == NULL)
        {
            if(getFileStats(pathname,0,NULL,NULL) != meFILETYPE_DIRECTORY)
            {
                *q = cc ;
                return NULL ;
            }            
            /* Create the new node - this is a directory */
            ndp = dirLinkNode(dp, dirConstructNode(p,DIR_UNKNOWN|mask));
            
#ifdef _UNIX
            /* Check if its a symbolic link */
            {
                meUByte lbuf [1024];
                int ii;
                
                /* readlink does not like an end '/' so remove it */
                q[-1] = '\0' ;
                if ((ii=readlink((char *)pathname,(char *)lbuf, 1024)) > 0)
                {
#if DEBUG
                    fprintf (fp, "[%s is a LINK]\n", pathname);
#endif
                    lbuf [ii] = DIR_CHAR ;
                    lbuf [ii+1] = '\0' ;
                    ndp->lname = meStrdup (lbuf);
                }
                q[-1] = DIR_CHAR ;
            }
#endif
        }
#ifdef _UNIX
        if(ndp->lname != NULL)
        {
            if((dp = ndp->child) == NULL)
            {
                if(ndp->mask & DIR_UNKNOWN)
                    dp = addLinkPath(ndp,pathname) ;
                if(dp == NULL)
                    return NULL ;
            }
        }
#endif
        if((mask & DIR_HIDDEN) == 0)
            ndp->mask &= ~DIR_HIDDEN ;
        *q = cc ;
    }
    return dp ;
}

static DIRNODE *
addLinkPath(DIRNODE *dnode, meUByte *pathname)
{
    DIRNODE *dp ;
    meUByte lpp[meBUF_SIZE_MAX], buff[meBUF_SIZE_MAX], *ff ;
    
    if(dnode->mask & DIR_UNKNOWN)
        dnode->mask = (dnode->mask & ~DIR_UNKNOWN)|DIR_HIDDEN ;
    if(dnode->lname[0] != DIR_CHAR)
    {
        meStrcpy(buff,pathname) ;
        ff = getFileBaseName(buff) ;
        meStrcpy(ff,dnode->lname) ;
        ff = buff ;
    }
    else
        ff = dnode->lname ;
    fileNameCorrect(ff,lpp,NULL) ;
    dp = addPath(lpp,DIR_HIDDEN) ;
    if(dp == NULL)
        return NULL ;
    dnode->child = dp ;
    dnode->lnext = dp->lhead ;
    dp->lhead = dnode ;
    return dp ;
}


static int
evalNode(register DIRNODE *dnode, meUByte *pathname, int flags)
{
    int     ii;                             /* Local loop counter */
    int     len;                            /* Length of the pathname */

    while(dnode->lname != NULL)
    {
        if((dnode->child == NULL) &&
           ((!(dnode->mask & DIR_UNKNOWN) && !(flags & LDO_FORCE)) ||
             (addLinkPath(dnode,pathname) == NULL)))
            return meTRUE ;
        dnode = dnode->child ;
    }
    if((dnode->mask & DIR_UNKNOWN) || (flags & LDO_FORCE))
    {
        DIRNODE *dn, *ndn ;
        
        dn = dnode->child ;
        while(dn != NULL)
        {
            dn->mask |= DIR_GONE ;
            dn = dn->next ;
        }
#if DEBUG
        fprintf(fp, "evalNode %s\n",pathname) ;
#endif
        mlwrite(0,(meUByte *)"[Evaluating %s]",pathname);
        len = meStrlen(pathname) ;
        if(len == 0)
        {
#ifdef _UNIX
            /* Under unix we just have to add the root '/' to the tree if its not
             * already there */
            if((dn=dirFindNode(dnode,(meUByte *)"/")) == NULL)
                dirLinkNode(dnode,dirConstructNode((meUByte *)"/",DIR_UNKNOWN));
            else
                dn->mask &= ~DIR_GONE ;
#endif
#ifdef _WIN32
            /* Under Windows get the list of drives and add to the directory list. */
            meUByte drvname [] = "X:/";
            int   curDrive;
            
            /* Get the drives */
            curDrive = _getdrive();
            for (ii = 1; ii <= 26; ii++)    /* Drives are a-z (1-26) */
            {
                if((ii == 1) || (_chdrive (ii) == 0)) /* Drive exists ?? */
                {
                    /* Yes - add to the drive list */
                    drvname [0] = ii + 'a' - 1;
                    if((dn=dirFindNode(dnode,drvname)) == NULL)
                        dirLinkNode (dnode,dirConstructNode(drvname,DIR_UNKNOWN));
                    else
                        dn->mask &= ~DIR_GONE ;
                }
            }
            /* Restore the previous drive */
            _chdrive (curDrive);
#endif
#ifdef _DOS
            /* Same under dos */
            meUByte drvname [] = "X:/";
            union REGS reg ;		/* cpu register for use of DOS calls */
    
            for (ii = 1; ii <= 26; ii++)    /* Drives are a-z (1-26) */
            {
                reg.x.ax = 0x440e ;
                reg.h.bl = ii ;
                intdos(&reg,&reg) ;
                if((reg.x.cflag == 0) || (reg.x.ax != 0x0f))
                {
                    drvname [0] = ii + 'a' - 1;
                    if((dn=dirFindNode(dnode,drvname)) == NULL)
                        dirLinkNode (dnode,dirConstructNode(drvname,DIR_UNKNOWN));
                    else
                        dn->mask &= ~DIR_GONE ;
                }
            }
#endif
        }
        else
        {
            int     noFiles ;                   /* Number of files */
            meUByte **files, *ss ;                /* File list */
            
            ss = pathname+len ;
            getDirectoryList(pathname,&curDirList) ;
            noFiles = curDirList.size ;
            files = curDirList.list ;
            
            /* Build a profile of the current directory. */
            for(ii=0 ; ii<noFiles ; ii++)
            {
                meUByte *fn;
                int flen ;
                
                if(TTbreakTest(0))
                    return ctrlg(meFALSE,1) ;
                /* Get the file and strip off any directory tail */
                fn = files [ii];
                flen = meStrlen(fn) - 1 ;
                if (fn[flen] == DIR_CHAR)
                {
                    if((flen > 2) || (fn [0] != '.') || ((flen != 1) && (fn[1] != '.')))
                    {
                        /* Add to the directory tree */
                        if((dn = dirFindNode (dnode, fn)) == NULL)
                        {
#ifdef _UNIX
                            meUByte lbuf [1024];
                            int ii ;
#endif
                            dn = dirConstructNode(fn, DIR_UNKNOWN) ;
                            dirLinkNode(dnode,dn) ;
#ifdef _UNIX
                            /* Check if its a symbolic link - readlink does not like the
                             * trailing '/' so don't copy it
                             */
                            meStrncpy(ss,fn,flen) ;
                            ss[flen] = '\0' ;
                            if ((ii=readlink((char *)pathname,(char *)lbuf,1024)) > 0)
                            {
#if DEBUG
                                fprintf (fp, "[%s is a LINK]\n",pathname);
#endif
                                lbuf [ii] = DIR_CHAR ;
                                lbuf [ii+1] = '\0';
                                dn->lname = meStrdup(lbuf);
                            }
#endif
                        }
                        else
                            dn->mask &= ~DIR_GONE ;
                    }
                }
#if DEBUG
                fprintf (fp, "evalNode: Past Dir =%s\n", fn);
#endif
            }
            *ss = '\0' ;
        }
        dn = dnode->child ;
        while(dn != NULL)
        {
            ndn = dn->next ;
            if(dn->mask & DIR_GONE)
                dirDeleteNode(dn) ;
            dn = ndn ;
        }
        /*
         * This node is now known, but flag it as hidden, as the call to
         * setHiddenFlag needs to know that this was effectively hidden.
         */
        if(dnode->mask & DIR_UNKNOWN)
            dnode->mask = (dnode->mask & ~DIR_UNKNOWN) | DIR_HIDDEN ;
    }

    if(flags & LDO_RECURSE)
    {   
        DIRNODE *dtemp, *dhead;
        meUByte *ss ;
        len = meStrlen(pathname) ;
        ss = pathname+len ;
        
        dhead = dtemp = dnode->child ;
        dnode->child = NULL ;
        while(dtemp != NULL)
        {
            meStrcpy(ss,dtemp->dname) ;
            if(evalNode(dtemp,pathname,flags) <= 0)
            {
                dnode->child = dhead ;
                *ss = '\0' ;
                return meABORT ;
            }
            dtemp = dtemp->next ;
        }
        dnode->child = dhead ;
        *ss = '\0' ;
    }
    return meTRUE ;
}

static void
setHiddenFlag(DIRNODE *dnode, int flags)
{
    if(flags & LDO_OPEN)
        dnode->mask &= ~DIR_HIDDEN ;
    else
        dnode->mask |= DIR_HIDDEN ;
    if(flags & LDO_RECURSE)
    {
        DIRNODE  *dtemp, *dhead;
        
        /* skip the links */
        while(dnode->lname != NULL)
        {
            if((dnode = dnode->child) == NULL)
                return ;
        }
        dhead = dtemp = dnode->child ;
        dnode->child = NULL ;
        while(dtemp != NULL)
        {
            setHiddenFlag(dtemp,flags) ;
            dtemp = dtemp->next ;
        }
        dnode->child = dhead ;
    }
}

static DIRNODE *
meFindDirLine(DIRNODE *dnode, meUByte *fname, long *itemNo)
{
    DIRNODE *dn, *ndn ;
    do
    {
        /* See if we have a hit */
        if(*itemNo <= 0)
        {
            if(dnode == dirlist)
                fname[0] = '\0' ;
            else
                meStrcpy(fname,dnode->dname) ;
            return dnode ;
        }
        *itemNo = (*itemNo) - 1 ;
        /* Move to the next node */
        dn = dnode ;
        while((dn->lname != NULL) && (dn->child != NULL))
            dn = dn->child ;
        if((dn->child != NULL) && ((dnode->mask & DIR_HIDDEN) == 0))
        {
            DIRNODE *cdn ;
            int      len ;
            
            if(dnode == dirlist)
                len = 0 ;
            else
                len = meStrlen(dnode->dname) ;
            if(dn != dnode)
            {
                ndn = dnode->child ;
                dnode->child = NULL ;
                cdn = meFindDirLine(dn->child,fname+len,itemNo) ;
                dnode->child = ndn ;
            }
            else
                cdn = meFindDirLine(dnode->child,fname+len,itemNo) ;
            if(cdn != NULL)
            {
                if(len > 0)
                    meStrncpy(fname,dnode->dname,len) ;
                return cdn ;
            }
        }
        dnode = dnode->next;     /* Advance to next next */
    } while (dnode != NULL);

    /* Not found */
    return NULL;
}

/*
 * findDirLine
 * Find the directory item for the line number
 */
static DIRNODE *
findDirLine(long itemNo, meUByte *fname)
{
    return meFindDirLine(dirlist,fname,&itemNo) ;
}

/*
 * dirDrawDir
 * Draw the directory list into a pop-up buffer.
 */
static void
dirDrawDirectory(meBuffer *bp, DIRNODE *dnode, int iLen, meUByte *iStr, meUByte *fname)
{
    DIRNODE *dn, *ndn ;
    meUByte buf[meBUF_SIZE_MAX], *fn ;              /* Working line buffer */
    int len, flen ;                      /* Length buffer under construction */
    
    while(dnode != NULL)
    {
        dn = dnode ;
        while((dn->lname != NULL) && (dn->child != NULL))
            dn = dn->child ;
        
        if((len = iLen) != 0)
            /* Add continuation bars if we are at a child level */
            meStrncpy(buf,iStr,iLen);
        
        if (dnode->next == NULL)
            buf[len++] = boxChars[BCNE] ;
        else
            buf[len++] = boxChars[BCNES] ;
        
        /* Add the open/close state of the current node */
        if(dn->mask & DIR_UNKNOWN)
            buf[len++] = '?';
        else if(dn->child == NULL)
            buf[len++] = boxChars[BCEW];
        else
            buf[len++] = (dnode->mask & DIR_HIDDEN)  ? '+' : '-';
        buf [len++] = ' ';
        
        /* Add the filename */
        flen = meStrlen(dnode->dname) ;
        //meStrncpy(buf+len,dnode->dname,flen) ;
        memcpy(buf+len,dnode->dname,flen);
        len += flen ;
        if(flen > 1)
            len-- ;
        if(dnode->lname != NULL)
            len += sprintf((char *)buf+len, " -> %s", dnode->lname);
        fn = NULL ;
        if(fname != NULL)
        {
            if(iLen == 0)
            {
                if(fname[0] != '\0')
                    fn = fname ;
            }
            else if(!meStrncmp(fname,dnode->dname,flen))
            {
                if(fname[flen] == '\0')
                {
                    buf[0] = '*';
                    curDirLine = meLineGetPrev(bp->dotLine) ;
                }
                else
                    fn = fname+flen ;
            }
        }
        buf [len] = '\0';
        /*        printf("%s\n",buf) ;*/
        addLineToEob(bp,buf);
        
        if ((dn->child != NULL) && ((dnode->mask & DIR_HIDDEN) == 0))
        {
            /* Add a bar for the level depending on whether there is
             * a next present */
            iStr[iLen] = (dnode->next != NULL) ? boxChars[BCNS] : ' ' ;
            if(dn != dnode)
            {
                ndn = dnode->child ;
                dnode->child = NULL ;
                dirDrawDirectory(bp,dn->child,iLen+1,iStr,fn) ;
                dnode->child = ndn ;
            }
            else
                dirDrawDirectory(bp,dnode->child,iLen+1,iStr,fn) ;
        }
        dnode = dnode->next;     /* Advance to next next */
    }
}

static meUByte dirBufName[]="*directory*" ;
/*
 * dirDrawDir
 * Draw the directory list into a pop-up buffer.
 */
static void
dirDrawDir(meUByte *fname, int n)
{
    meBuffer *bp;                         /* Buffer pointer */
    meWindow *wp;                         /* Window associated with buffer */
    meUByte   iStr[meBUF_SIZE_MAX];               /* Vertical bars buffer */
    
    /* Find the buffer and vapour the old one */
    if((wp = meWindowPopup(dirBufName,(BFND_CREAT|BFND_CLEAR|WPOP_USESTR),NULL)) == NULL)
        return ;
    bp = wp->buffer ;                   /* Point to the buffer */
    
    if(n & LDO_SELECT)
    {
        if(fnamecmp(bp->fileName,fname))
            meStrrep(&bp->fileName,fname) ;
    }
    curDirLine = NULL ;
    dirDrawDirectory(bp,dirlist,0,iStr,bp->fileName) ;
    bp->dotLine = meLineGetNext(bp->baseLine);
    bp->dotOffset = 0 ;
    bp->dotLineNo = 0 ;
    if(curDirLine != NULL)
    {
        curDirLine = meLineGetNext(curDirLine) ;
        while(bp->dotLine != curDirLine)
        {
            bp->dotLine = meLineGetNext(bp->dotLine) ;
            (bp->dotLineNo)++ ;
        }
    }
    meModeSet(bp->mode,MDVIEW) ;
    meModeClear(bp->mode,MDAUTOSV) ;
    meModeClear(bp->mode,MDUNDO) ;
    resetBufferWindows(bp) ;
}

int
directoryTree(int f, int n)
{
    DIRNODE *dnode, *dn ;
    meUByte buf[meBUF_SIZE_MAX] ;
    
    if(n & LDO_GETPATH)
    {
        if(inputFileName((meUByte *)"List Directory ", buf, 1) <= 0)
            return meABORT ;
        if((dnode = addPath(buf,0)) == NULL)
            return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[%s not a directory]",buf);
    }
    else
    {
        if(meStrcmp(frameCur->bufferCur->name,dirBufName))
            return mlwrite(MWABORT,(meUByte *)"[buffer is not *directory*]") ;
        if((dnode = findDirLine(frameCur->windowCur->dotLineNo,buf)) == NULL)
            return meABORT ;
    }
    dn = dnode ;
    while((dn->lname != NULL) && (dn->child != NULL))
        dn = dn->child ;
    
    /* If the users has asked for a toggle then work out
     * whether this is an open or close */
    if(n & LDO_TOGGLE)
    {
        if((dnode->mask & DIR_HIDDEN) || (dn->mask & DIR_UNKNOWN))
            n |= LDO_OPEN ;
        else
            n |= LDO_CLOSE ;
    }
    /* do we need to evaluate the node ? Note that unkown nodes are 
     * effectively hidden
     */
    if((n & LDO_EVAL) && (n & LDO_OPEN) &&
       ((dn->mask & DIR_UNKNOWN) || (n & LDO_FORCE)))
        evalNode(dnode,buf,n) ;
    
    if(n & (LDO_OPEN|LDO_CLOSE|LDO_RECURSE)) 
        setHiddenFlag(dnode,n) ;
    
    dirDrawDir(buf,n) ;
    return meTRUE ;
}

#ifdef _ME_FREE_ALL_MEMORY
void dirFreeMemory(void)
{
    if(dirlist != NULL)
    {
        dirDeleteTree(dirlist) ;
        dirDestructNode(dirlist) ;
    }
}
#endif
#endif
