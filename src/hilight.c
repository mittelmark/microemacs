/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * hilight.c - Token based hilighting routines.
 *
 * Copyright (C) 1994-2001 Steven Phillips
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
 * Created:     21/12/94
 * Synopsis:    Token based hilighting routines.
 * Authors:     Steven Phillips
 * Notes:
 *     The format of hilight tokens are as similar to regex as possible.
 *     There is a mis-use of the \{\} intervals to indicate the start
 *     and end of the hilighted part of a token.
 */ 

#define	__HILIGHTC				/* Name file */

#include "emain.h"

#if MEOPT_HILIGHT

/*
 * HilFunc
 * This function is invoked whenever the source text reaches a defined postion,
 * presently defined at the start and end of the highlight selection blocks.
 * The call back function deals with the change in colour that results. 
 * */
struct HILDATA;                         /* Forward reference structure */
typedef void (*HilFunc)(int, struct HILDATA *);

/*
 * HILDATA
 * This structure contains data that is exported from hilightLine to the
 * worker functions. It contains enough information to allow the selection
 * colour modifications to be performed.
 */
typedef struct HILDATA {
    meHilight *root;                    /* Root of the hilighting */
    meHilight *node;                    /* Current node used in hilighting */
    meHilight *cnode;                   /* Current working node */
    meHilight *hnode;                   /* Current hilighting node */
    meSchemeSet *blkp;                  /* Block pointer */
    int noColChng;                      /* Number of colour changes */
    int srcPos;                         /* Source position */
    meUShort srcOff;                    /* Offset in source to find */
    HilFunc hfunc;                      /* Callback function */
    meUShort len;                       /* Transient length information */
    meUShort flag;                      /* Video structure flag */
    meUByte colno;                      /* Color number */
    meUByte tabWidth;                   /* The current tab width */
} HILDATA;    

#define meHIL_TEST_BACKREF 0x00
#define meHIL_TEST_SINGLE  0x01
#define meHIL_TEST_CLASS   0x02
#define meHIL_TEST_EOL     0x03
#define meHIL_TEST_SPACE   0x04
#define meHIL_TEST_DIGIT   0x05
#define meHIL_TEST_XDIGIT  0x06
#define meHIL_TEST_LOWER   0x07
#define meHIL_TEST_UPPER   0x08
#define meHIL_TEST_ALPHA   0x09
#define meHIL_TEST_ALNUM   0x0a
#define meHIL_TEST_SWORD   0x0b
#define meHIL_TEST_WORD    0x0c
#define meHIL_TEST_ANY     0x0d
#define meHIL_TEST_MASK    0x0f
#define meHIL_TEST_NOCLASS (1+meHIL_TEST_ANY-meHIL_TEST_SPACE)

#define meHIL_TEST_INVERT        0x10
#define meHIL_TEST_MATCH_NONE    0x20
#define meHIL_TEST_MATCH_MULTI   0x40
#define meHIL_TEST_VALID         0x80
#define meHIL_TEST_INVALID       0x00
#define meHIL_TEST_TOKEN         (meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_WORD)
#define meHIL_TEST_SOL           0xfe

#define meHIL_TEST_DEF_GROUP     10

static meUByte *meHilTestNames[meHIL_TEST_NOCLASS]={
    (meUByte *)"space",(meUByte *)"digit",(meUByte *)"xdigit",(meUByte *)"lower",(meUByte *)"upper",
    (meUByte *)"alpha",(meUByte *)"alnum",(meUByte *)"sword",(meUByte *)"word",(meUByte *)"any", 
} ;

#define meHIL_MODECASE   0x001
#define meHIL_MODELOOKB  0x002
#define meHIL_MODETOKEWS 0x004
#define meHIL_MODESTART  0x008
#define meHIL_MODESTTLN  0x010
#define meHIL_MODETOEOL  0x020
#define meHIL_MODEMOVE   0x040
#define meHIL_MODETOKEND 0x080

static meUByte varTable[11] ;
static meUShort meHilightBranchType ;

void
freeToken(meHilight *root)
{
    meUByte ii, top ;
    
    top = hilListSize(root) ;
    for(ii=0 ; ii<top ; ii++)
        freeToken(root->list[ii]) ;
    meNullFree(root->table) ;
    meNullFree(root->list) ;
    meNullFree(root->close) ;
    meNullFree(root->rtoken) ;
    meNullFree(root->rclose) ;
    free(root) ;
}

static meHilight *
createHilight(meUByte hilno, meInt size, meUByte *noHils , meHilight ***hTbl)
{
    meHilight *root ;
    
    if(hilno >= *noHils)
    {
        if((*hTbl = (meHilight **) 
            meRealloc(*hTbl,(hilno+1)*sizeof(meHilight *))) == NULL)
            return NULL ;
        memset((*hTbl)+(*noHils),0,(hilno+1-(*noHils))*sizeof(meHilight *)) ;
        *noHils = hilno+1 ;
    }
    else
    {
        if((root=(*hTbl)[hilno]) != NULL)
        {
            meHilight *hn ;
            while((hn = meHilightGetColumnHilight(root)) != NULL)
            {
                meHilightSetColumnHilight(root,meHilightGetColumnHilight(hn)) ;
                meFree(hn) ;
            }
            root->close = NULL ;
            freeToken(root) ;
        }
        (*hTbl)[hilno] = NULL ;
    }
    if((root = meMalloc(sizeof(meHilight) + size)) == NULL)
        return NULL ;
    memset(root,0,sizeof(meHilight)) ;
    return root ;
}

static void
ListToTable(meHilight *root)
{
    meUByte pos, ii, size ;
    meUByte cc ;
    
    if(root->listSize < 8)
        return ;
    
    root->table = (meUByte *) meMalloc(TableSize) ;
    
    root->table[0] = root->ordSize ;
    pos  = root->ordSize ;
    size = hilListSize(root) ;
    for(; (pos<size) && ((meUByte)(root->list[pos]->token[0]) < TableLower) ; pos++)
        ;
    cc = TableLower ;
    for(ii=1 ; ii<TableSize ; ii++,cc++)
    {
        root->table[ii] = pos ;
        for(; (pos<size) && (root->list[pos]->token[0] == cc) ; pos++)
            ;
    }
    root->table[TableSize-1] = size;
}

#define ADDTOKEN_REMOVE   0x10000
#define ADDTOKEN_DUMMY    0x20000
#define ADDTOKEN_OPTIM    0x40000

#define tokenSetType(node,tt) \
((node)->type = (((node)->type & (HLASOL|HLASTTLINE)) | (tt)))

static void
hilNodeOptimize(meHilight *root, int flags) ;

static meHilight *
addTokenNode(meHilight *root, meUByte *token, int flags)
{
    meHilight *node, *nn ;
    meUByte c1=1, ii, jj, top, spec ;
    int len ;
    
    /* Have we run out of room, is so try to optimize */
    if(!(flags & ADDTOKEN_OPTIM) && (hilListSize(root) == 0xfe))
        hilNodeOptimize(root,flags) ;

    if((*token == meCHAR_LEADER) && (*++token != meCHAR_TRAIL_LEADER))
    {
        /* special test */
        ii  = 0 ;
        top = root->ordSize ;
        spec = 1 ;
    }
    else
    {
        ii = root->ordSize ;
        top = hilListSize(root) ;
        spec = 0 ;
    }
    for( ; ii<top ; ii++)
    {
        meUByte c2, *s1, *s2 ;
        node = root->list[ii] ;
        s1 = token ;
        s2 = node->token ;
        while((c1 = *s1) == (c2 = *s2))
        {
            if(c1 == '\0')
            {
                if(flags & ADDTOKEN_DUMMY)
                {
                    /* if adding a dummy, the node already exists as a legit, 
                     * the only thinkg to do is correct the ASTTLINE & ASOL flags,
                     * otherwise leave it alone */
                    node->type &= (~(HLASOL|HLASTTLINE)) | flags ; 
                    return node ;
                }
                /* if removing token or readding, this node must
                 * be reset
                 */
                if(node->rtoken != NULL)
                {
                    meFree(node->rtoken) ;
                    node->rtoken = NULL ;
                }
                if(node->close != NULL)
                {
                    meFree(node->close) ;
                    node->close = NULL ;
                }
                if(node->rclose != NULL)
                {
                    meFree(node->rclose) ;
                    node->rclose = NULL ;
                }
                tokenSetType(node,(flags & ADDTOKEN_REMOVE) ? HLVALID:0) ;
                return node ;
            }
            s1++ ;
            s2++ ;
        }
        if(c1 == '\0')
        {
            if(flags & ADDTOKEN_REMOVE)
                return NULL ;
            break ;
        }
        if(c2 == '\0')
        {
            node->type &= (~(HLASOL|HLASTTLINE)) | flags ; 
            return addTokenNode(node,s1,flags) ;
        }
        if(c2 > c1)
            break ;
    }
    if(flags & ADDTOKEN_REMOVE)
        return NULL ;
    
    /* ii points to position to insert top must now be the table size */
    /* going to be added to this list, check that we have enough room */
    if(((top=hilListSize(root)) == 0xfe) && !(flags & ADDTOKEN_OPTIM))
    {
        mlwrite(MWABORT|MWWAIT,(meUByte *)"Table full, cant add [%s]. Optimise!",token) ;
        return NULL ;
    }
    top++ ;
    len = meStrlen(token) ;
    if(((root->list = (meHilight **) meRealloc(root->list,top*sizeof(meHilight *))) == NULL) ||
       ((nn = meMalloc(sizeof(meHilight)+len)) == NULL))
        return NULL ;
    memset(nn,0,sizeof(meHilight)) ;
    nn->type = ((flags & ADDTOKEN_DUMMY) ?
                (HLVALID|(flags & (HLASOL|HLASTTLINE))):HLASOL|HLASTTLINE) ;
    meStrcpy(nn->token,token) ;
    
    if(c1 == 0)
    {
        int kk, ll ;
        for(jj=0,kk=ii ;  kk<top-1 ; kk++,jj++)
            if(meStrncmp(token,root->list[kk]->token,len))
                break ;
        if((nn->list = (meHilight **) meMalloc(jj*sizeof(meHilight *))) == NULL)
            return NULL ;
        for(jj=0,ll=ii ;  ll<kk ; ll++,jj++)
        {
            nn->list[jj] = root->list[ll] ;
            meStrcpy(nn->list[jj]->token,nn->list[jj]->token+len) ;
            if((nn->list[jj]->type & (HLASOL|HLSOL))  != (HLASOL|HLSOL))
                nn->type &= ~HLASOL ;
            if((nn->list[jj]->type & (HLASTTLINE|HLSTTLINE))  != (HLASTTLINE|HLSTTLINE))
                nn->type &= ~HLASTTLINE ;
        }
        nn->listSize = jj ;
        root->list[ii++] = nn ;
        for( ; ll<top ; ii++,ll++)
            root->list[ii] = root->list[ll] ;
        jj = 1 - jj ;
    }
    else
    {
        for(jj=top-1 ; jj>ii ; jj--)
            root->list[jj] = root->list[jj-1] ;
        root->list[ii] = nn ;
        jj = 1 ;
    }
    if(spec)
        (root->ordSize) += jj ;
    else
        (root->listSize) += jj ;
    
    if(root->table == NULL)
        ListToTable(root) ;
    else
    {
        if(spec)
            ii = 0 ;
        else
            ii = GetTableSlot((meUByte)(*token)) + 1 ;
        for(; ii<TableSize ; ii++)
            (root->table[ii]) += jj ;
    }
    return nn ;
}

static void
hilNodeOptimize(meHilight *root, int flags)
{
    int ii, sz, no=0, cc=0, jj=0 ;
    meUByte tok[2] ;
    
    ii = root->ordSize ;
    sz = ii+root->listSize ;
    while(ii < sz)
    {
        if(root->list[ii]->token[0] != cc)
        {
            if(jj > no)
            {
                tok[0] = cc ;
                no = jj ;
            }
            cc = root->list[ii]->token[0] ;
            jj = 0 ;
        }
        else
            jj++ ;
        ii++ ;
    }
    if(no)
    {
        tok[1] = '\0' ;
        addTokenNode(root,tok,flags|ADDTOKEN_DUMMY|ADDTOKEN_OPTIM) ;
    }
}


static meUByte *
meHiltItemCompile(meUByte *dest, meUByte **token,
                  meUByte noGroups, meUByte changeCase)
{
    meUByte cc, *ss=*token ;
    if((cc=*ss++) == '[')
    {
        meUByte *s1, *dd ;
        meUByte tt, ii=0 ;
        
        *dest++ = meCHAR_LEADER ;
        tt = meHIL_TEST_VALID ;
        if(*ss == '^')
        {
            tt |= meHIL_TEST_INVERT ;
            ss++ ;
        }
        if(((cc = *ss++) == '[') && (*ss == ':'))
        {
            s1 = ss ;
            while(*++s1 != '\0')
            {
                if(*s1 == ']')
                {
                    if((s1[1] == ']') && (*--s1 == ':'))
                    {
                        *s1 = '\0' ;
                        for(ii=0 ; ii<meHIL_TEST_NOCLASS ; ii++)
                            if(!meStrcmp(ss+1,meHilTestNames[ii]))
                                break ;
                        if(ii == meHIL_TEST_NOCLASS)
                        {
                            /* this is an unknown char class return an error */
                            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unknown hilight class %s]",ss+1);
                            *s1 = ':' ;
                            return NULL ;
                        }
                        *s1 = ':' ;
                        ss = s1+3 ;
                        ii += meHIL_TEST_SPACE ;
                    }
                    break ;
                }
            }
        }
        *dest++ = ((ii == 0) ? meHIL_TEST_CLASS:ii)|tt ;
        *dest++ = meHIL_TEST_DEF_GROUP ;
        if(ii == 0)
        {
            dd = dest++ ;
            do {
                if(cc == '\0')
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Open []");
                    return NULL ;
                }
                *dest++ = cc ;
            } while((cc=*ss++) != ']') ;
            *dd = (meUByte) (dest-dd-1) ;
        }
    }
    else if(cc == '\\')
    {
        cc = *ss++ ;
        switch(cc)
        {
        case '!':
            cc = *ss++ ;
            if((cc <= '0') || ((cc-='0') > noGroups))
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Back reference out of range]");
                return NULL ;
            }
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_BACKREF ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            *dest++ = cc ;
            break ;
        case 'A':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_ALPHA ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'D':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_DIGIT ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'H':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_XDIGIT ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'L':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_LOWER ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'M':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_ALNUM ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'S':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_SPACE ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'U':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_UPPER ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'W':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_INVERT|meHIL_TEST_WORD ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'a':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_ALPHA ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'd':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_DIGIT ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'e':   *dest++ = 0x1b; break;
        case 'f':   *dest++ = 0x0c; break;
        case 'g':   *dest++ = 0x07; break;
        case 'h':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_XDIGIT ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'l':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_LOWER ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'm':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_ALNUM ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'n':   *dest++ = 0x0a; break;
        case 'r':   *dest++ = 0x0d; break;
        case 's':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_SPACE ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 't':   *dest++ = 0x09; break;
        case 'u':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_UPPER ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'v':   *dest++ = 0x0b; break;
        case 'w':
            *dest++ = meCHAR_LEADER ;
            *dest++ = meHIL_TEST_VALID|meHIL_TEST_WORD ;
            *dest++ = meHIL_TEST_DEF_GROUP ;
            break ;
        case 'x':
            cc = *ss ;
            if(isXDigit(cc))
            {
                register meUByte c1 ;
                c1 = *++ss ;
                if(isXDigit(c1))
                {
                    cc = (hexToNum(cc) << 4) | hexToNum(c1) ;
                    ss++ ;
                }
                else
                    cc = hexToNum(cc) ;
                if(cc == 0)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Invalid \\x character]");
                    return NULL ;
                }
                if(changeCase)
                    cc = toLower(cc) ;
                *dest++ = cc ;
                if(cc == meCHAR_LEADER)
                    *dest++ = meCHAR_TRAIL_LEADER ;
            }
            break;
        case '\'':
        case '<':
        case '>':
        case 'B':
        case '`':
        case 'b':
        case '|':
            mlwrite(MWABORT|MWWAIT,(meUByte *)"[Hilight does not support \\%c]",cc);
            return NULL ;
        default:
            if(isDigit(cc))
            {
                if(((cc-='0') == 0) || (cc > noGroups))
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Back reference out of range]");
                    return NULL ;
                }
                *dest++ = meCHAR_LEADER ;
                *dest++ = meHIL_TEST_VALID|meHIL_TEST_BACKREF ;
                *dest++ = meHIL_TEST_DEF_GROUP ;
                *dest++ = cc ;
            }
            else
            {
                if(changeCase)
                    cc = toLower(cc) ;
                *dest++ = cc;
            }
        }
    }
    else if(cc == '.')
    {
        *dest++ = meCHAR_LEADER ;
        *dest++ = meHIL_TEST_VALID|meHIL_TEST_ANY ;
        *dest++ = meHIL_TEST_DEF_GROUP ;
    }
    else
    {
        if(cc == meCHAR_LEADER)
        {
            *dest++ = meCHAR_LEADER ;
            if((cc = *ss++) != meCHAR_TRAIL_LEADER)
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unsupported special \\ quoted char]");
                return NULL ;
            }
        }
        else if(changeCase)
            cc = toLower(cc) ;
        *dest++ = cc;
    }
    *token = ss ;
    return dest ;
}

/* compiles the given token into a string form compatible with the main
 * hilight search engine.
 * The retValues are:
 * 
 *     retValues[0] : Token length in items
 *     retValues[1] : Token hilight start offset.
 *     retValues[2] : Token hilight end offset.
 *     retValues[3] : Number of groups.
 * 
 * Returns a pointer to the start of the last item or NULL on failure.
 */
static meUByte *
meHiltStringCompile(meUByte *dest, meUByte *token, meUByte changeCase,
                    int *retValues)
{
    meUByte cc, *dd, *lastDest ;
    meUByte nextGroupNo=0, group=0 ;
    int startOffset=0, endOffset=-1, len=0 ;
    
    while((cc=*token) != '\0')
    {
        if(cc == '\\')
        {
            switch(token[1])
            {
            case '(':
                if(nextGroupNo >= 9)
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Too many groups]");
                    return NULL ;
                }
                group = 1 ;
                token += 2 ;
                if((*token == '\0') || ((*token == '\\') && (token[1] == ')')))
                {
                    mlwrite(MWABORT|MWWAIT,(meUByte *)"[Bad group \\(.\\)]");
                    return NULL ;
                }
                break ;
            case '|':
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Hilight does not support \\|]");
                return NULL ;
            case ')':
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unmatched group \\)]");
                return NULL ;
            case '{':
                startOffset = len ;
                token += 2 ;
                break ;
            case '}':
                if(endOffset < 0)
                    endOffset = len ;
                token += 2 ;
                break ;
            }
            if((cc = *token) == '\0')
                break ;
        }
        if(cc == '$')
        {
            if(*++token != '\0')
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[End-of-line $ must be the last char]");
                return NULL ;
            }
            if(endOffset < 0)
                endOffset = len ;
            dd = dest ;
            *dd++ = meCHAR_LEADER ;
            *dd++ = meHIL_TEST_VALID|meHIL_TEST_EOL ;
            *dd++ = meHIL_TEST_DEF_GROUP ;
        }
        else if((dd = meHiltItemCompile(dest,&token,nextGroupNo,changeCase)) == NULL)
            return NULL ;
        if(group)
        {
            if((*token++ != '\\') || (*token++ != ')'))
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[A group \\(.\\) must enclose a single char]");
                return NULL ;
            }
            if(dd <= dest+2)
            {
                /* groups must encase a test (a [a-z] or . etc) if not the
                 * turn it into a test */
                dest[3] = dest[0] ;
                dest[1] = (meHIL_TEST_VALID|meHIL_TEST_SINGLE) ;
                dest[0] = meCHAR_LEADER ;
                dd = dest+4 ;
            }
            dest[2] = ++nextGroupNo ;
            group = 0 ;
        }
        if(((cc=token[0]) == '?') || (cc == '*') || (cc == '+'))
        {
            token++ ;
            if(dd <= dest+2)
            {
                /* closures can only be done on tests (a [a-z]* or .+ etc). If not then
                 * turn it into a test */
                dest[3] = dest[0] ;
                dest[2] = meHIL_TEST_DEF_GROUP ;
                dest[1] = (meHIL_TEST_VALID|meHIL_TEST_SINGLE) ;
                dest[0] = meCHAR_LEADER ;
                dd = dest+4 ;
            }
            if(cc != '+')
                dest[1] |= meHIL_TEST_MATCH_NONE ;
            if(cc != '?')
                dest[1] |= meHIL_TEST_MATCH_MULTI ;
        }
        lastDest = dest ;
        dest = dd ;
        len++ ;
    }
    /* can't have a zero length search string */
    if(len == 0)
    {
        mlwrite(MWABORT|MWWAIT,(meUByte *)"[Invalid zero length token]");
        return NULL ;
    }
    *dest = '\0' ;
    if(endOffset < 0)
        endOffset = len ;
    retValues[0] = len ;
    retValues[1] = startOffset ;
    retValues[2] = endOffset ;
    retValues[3] = nextGroupNo ;
    
    return lastDest ;
}

static meHilight *
meHiltTokenAddSearchString(meHilight *root, meHilight *node, meUByte *token, int flags)
{
    meUByte buff[meBUF_SIZE_MAX], *dd, *ss, *lastItem ;
    int retValues[4], l1, l2, len ;
    
    dd = buff ;
    ss = token ;
    /* must remove the start of line '^' as its a zero length item 
     * implemented in flags */
    if(*ss == '^')
    {
        if(node == NULL)
            flags |= HLSOL ;
        else
        {
            /* dirty special case */
            *dd++ = meCHAR_LEADER ;
            *dd++ = meHIL_TEST_SOL ;
        }
        ss++ ;
    }
    if((lastItem=meHiltStringCompile(dd,ss,(meUByte) ((meHilightGetFlags(root) & HFCASE) != 0),
                                     retValues)) == NULL)
        return NULL ;
    len = retValues[0] ;
    l1 = retValues[1] ;
    l2 = retValues[2] ;
    
    l2 = len - l2 ;
    len -= l1 + l2 ;
    
    if(node == NULL)
    {
        meUByte  sttTst, endTst ;
        ss = buff ;
        /* if the token starts with "^\\s*\\{" then we can
         * optimize this to HLSTTLINE */
        if(l1 && (len || l2) && (flags & HLSOL) && (ss[0] == meCHAR_LEADER) &&
           (ss[1] == (meHIL_TEST_VALID|meHIL_TEST_MATCH_NONE|meHIL_TEST_MATCH_MULTI|meHIL_TEST_SPACE)))
        {
            flags &= ~HLSOL ;
            flags |= HLSTTLINE ;
            l1-- ;
            ss += 3 ;
        }
        /* if the token ends with ".*\\}" then we can
         * optimize this to HLENDLINE - note as the token includes 
         * the .*, a replace should also replace the .* */
        if(!l2 && (len || l1) && (lastItem[0] == meCHAR_LEADER) &&
           (lastItem[1] == (meHIL_TEST_VALID|meHIL_TEST_MATCH_NONE|meHIL_TEST_MATCH_MULTI|meHIL_TEST_ANY)))
        {
            flags |= HLENDLINE|HLREPTOEOL ;
            len-- ;
            lastItem[0] = '\0' ;
        }
        
        /* setup the start and end Test flag */
        if(flags & HLTOKEN)
        {
            sttTst = meHIL_TEST_TOKEN ;
            endTst = meHIL_TEST_TOKEN ;
        }
        else
        {
            if(l1 && (len || l2) && (ss[0] == meCHAR_LEADER) &&
               ((ss[1] & ~(meHIL_TEST_MASK|meHIL_TEST_INVERT)) == meHIL_TEST_VALID) &&
               ((ss[1] & meHIL_TEST_MASK) > meHIL_TEST_CLASS))
            {
                sttTst = ss[1] ;
                l1-- ;
                ss += 3 ;
            }
            else
                sttTst = meHIL_TEST_INVALID ;
            if(l2 && (len || l1) && (lastItem[0] == meCHAR_LEADER) &&
               ((lastItem[1] & ~(meHIL_TEST_MASK|meHIL_TEST_INVERT)) == meHIL_TEST_VALID) &&
               ((lastItem[1] & meHIL_TEST_MASK) > meHIL_TEST_CLASS))
            {
                endTst = lastItem[1] ;
                l2-- ;
                lastItem[0] = '\0' ;
            }
            else
                endTst = meHIL_TEST_INVALID ;
        }
        /* if the token starts at the begining of the line add the
         * optimizing AllSOL flag */
        if(flags & HLSOL)
            flags |= HLASOL|HLASTTLINE ;
        else if(flags & HLSTTLINE)
            flags |= HLASTTLINE ;
        /* must add all sections of the token as dummy nodes */
        dd = ss ;
        while(dd <= lastItem)
        {
            if((*dd++ == meCHAR_LEADER) && (*dd++ != meCHAR_TRAIL_LEADER) && (dd != ss+2)) 
            {
                dd[-2] = '\0' ;
                if(addTokenNode(root,ss,flags|ADDTOKEN_DUMMY) == NULL)
                    return NULL ;
                dd[-2] = meCHAR_LEADER ;
                dd++ ;
            }
        }
        if(((node = addTokenNode(root,ss,flags)) == NULL) || (flags & ADDTOKEN_REMOVE))
            return NULL ;
        
        if(flags & (HLBRANCH|HLBRACKET))
            flags &= ~HLCONTIN ;
        tokenSetType(node,flags) ;
        node->tknSttTst = sttTst ;
        node->tknEndTst = endTst ;
        node->tknSttOff = l1 ;
        node->tknEndOff = l2 ;
    }
    else
    {
        if((dd = meStrdup(buff)) == NULL)
            return NULL ;
        
        node->close = dd ;
        node->clsSttOff = l1 ;
        node->clsEndOff = l2 ;
    }
    /* store the len temporarily into scheme for len tests on the replace strings */
    node->scheme = len ;
    return node ;
}

static meHilight *
meHiltTokenAddReplaceString(meHilight *root, meHilight *node, meUByte *token, int mainRepl)
{
    meUByte buff[meBUF_SIZE_MAX], *dd, *ss, cc ;
    int len ;
    
    ss = token ;
    dd = buff ;
    len = 0 ;
    while((cc=*ss++) != '\0')
    {
        if(cc == '\\')
        {
            cc = *ss++ ;
            if((cc > '0') && (cc <= '9'))
            {
                *dd++ = meCHAR_LEADER ;
                cc -= '0' ;
            }
        }
        else if(cc == meCHAR_LEADER)
        {
            *dd++ = meCHAR_LEADER ;
            if((cc = *ss++) != meCHAR_TRAIL_LEADER)
            {
                mlwrite(MWABORT|MWWAIT,(meUByte *)"[Unsupported special \\ quoted char]");
                return NULL ;
            }
        }
        *dd++ = cc ;
        len++ ;
    }
    *dd = 0 ;
    
    if((dd = meStrdup(buff)) == NULL)
        return NULL ;
    
    if(mainRepl)
        node->rtoken = dd ;
    else
        node->rclose = dd ;
    
    if(len != node->scheme)
    {
        node->type |= HLRPLCDIFF ;
        meHilightGetFlags(root) |= HFRPLCDIFF ;
    }
    return node ;
}


int
hilight(int f, int n)
{
    meHilight *root, *node ;
    meUByte  buf[meBUF_SIZE_MAX] ;
    meUShort type ;
    meUByte  hilno ;
    int ii ;
    
    if(n == 0)
    {
        if((meGetString((meUByte *)"Hi no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
           ((hilno = (meUByte) meAtoi(buf)) == 0) || 
           (meGetString((meUByte *)"Flags",0,0,buf,meBUF_SIZE_MAX) <= 0))
            return meABORT ;
        type = (meUShort) meAtoi(buf) ;
        
        if((root = createHilight(hilno,0,&noHilights,&hilights)) == NULL)
            return meFALSE ;
        
        /* force a complete update next time */ 
        sgarbf = meTRUE ;
        meHilightGetFlags(root) = (meUByte) type ;
        if(type & HFLOOKBLN)
        {
            if(meGetString((meUByte *)"Lines",0,0,buf,meBUF_SIZE_MAX) <= 0)
                return meABORT ;
            meHilightSetLookBackLines(root,meAtoi(buf)) ;
        }
        if(type & HFLOOKBSCH)
        {
            if((meGetString((meUByte *)"Ind no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
               ((ii = meAtoi(buf)) <= 0) || (ii >= noIndents) || (indents[ii] == NULL))
                return meABORT ;
            meHilightGetLookBackScheme(root) = (meUByte) ii ;
        }
        if((ii=meGetString((meUByte *)"Scheme",MLEXECNOUSER,0,buf,meBUF_SIZE_MAX)) == meABORT)
            return meABORT ;
        if(ii == meFALSE)
            ii = globScheme ;
        else if((ii=convertUserScheme(meAtoi(buf), -1)) < 0)
            return meFALSE ;
        root->scheme = (meScheme) ii ;
        if((ii=meGetString((meUByte *)"Trunc scheme",MLEXECNOUSER,0,buf,meBUF_SIZE_MAX)) == meABORT)
            return meABORT ;
        if(ii == meFALSE)
            ii = trncScheme ;
        else if((ii=convertUserScheme(meAtoi(buf), -1)) < 0)
            return meFALSE ;
        meHilightGetTruncScheme(root) = ii ;
        hilights[hilno] = root ;
        return meTRUE ;
    }
    n = (n < 0) ? ADDTOKEN_REMOVE:0 ;
    
    if((meGetString((meUByte *)"Hi no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
       ((hilno = (meUByte) meAtoi(buf)) == 0) ||
       (hilno >= noHilights) ||
       ((root  = hilights[hilno]) == NULL) ||
       (meGetString((meUByte *)"Type",0,0,buf,meBUF_SIZE_MAX) <= 0))
        return meFALSE ;
    type = (meUShort) meAtoi(buf) ;
    if(type & HLCOLUMN)
    {
        meInt fmCol, toCol ;
        
        if((meGetString((meUByte *)"From",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
           ((fmCol = (meUShort) meAtoi(buf)) < 0) ||
           (!(n & ADDTOKEN_REMOVE) &&
            ((meGetString((meUByte *)"To",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             ((toCol = (meUShort) meAtoi(buf)) < fmCol))))
            return meFALSE ;
        while(((node = meHilightGetColumnHilight(root)) != NULL) && (meHilightGetFromColumn(node) < fmCol))
            root = node ;
        if((node != NULL) && (meHilightGetFromColumn(node) == fmCol))
        {
            if(n & ADDTOKEN_REMOVE)
            {
                /* free off the column hilight */ 
                meHilightSetColumnHilight(root,meHilightGetColumnHilight(node)) ;
                meFree(node) ;
                return meTRUE ;
            }
        }
        else if(n & ADDTOKEN_REMOVE)
            return meTRUE ;
        else if((node = meMalloc(sizeof(meHilight))) == NULL)
            return meABORT ;
        else
        {
            memset(node,0,sizeof(meHilight)) ;
            node->type = HLCOLUMN ;
            meHilightSetFromColumn(node,fmCol) ;
            meHilightSetColumnHilight(node,meHilightGetColumnHilight(root)) ;
            meHilightSetColumnHilight(root,node) ;
        }
        meHilightSetToColumn(node,toCol) ;
        goto get_scheme ;
    }
    
    if(meGetString((meUByte *)"Token",0,0,buf,meBUF_SIZE_MAX) <= 0)
        return meFALSE ;
    
    /* add the token */
    node = meHiltTokenAddSearchString(root,NULL,(meUByte *) buf,(type|n)) ;
    if(n & ADDTOKEN_REMOVE)
        return meTRUE ;
    if(node == NULL)
        return meFALSE ;
    type = node->type ;
    
    if((type & (HLVALID|HLONELNBT)) != HLVALID)
    {
        if(((type & HLREPLACE) && 
            ((meGetString((meUByte *)"Replace",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             (meHiltTokenAddReplaceString(root,node,(meUByte *)buf,1) == NULL))) ||
           ((type & HLBRACKET) &&
            ((meGetString((meUByte *)"Close",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             (meHiltTokenAddSearchString(root,node,(meUByte *) buf,0) == NULL) ||
             ((type & HLREPLACE) && 
              ((meGetString((meUByte *)"Replace",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
               (meHiltTokenAddReplaceString(root,node,(meUByte *)buf,0) == NULL))) ||
             (meGetString((meUByte *)"Ignore",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             ((node->ignore = buf[0]) > 255))) ||
           ((type & HLCONTIN) &&
            ((meGetString((meUByte *)"Continue",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             ((node->close = (meUByte *) meStrdup(buf)) == NULL) ||
             ((type & HLREPLACE) && 
              ((meGetString((meUByte *)"Replace",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
               ((node->rclose = (meUByte *) meStrdup(buf)) == NULL))))) ||
           ((type & HLBRANCH) &&
            ((meGetString((meUByte *)"hilno",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
             ((node->ignore = (meUByte) meAtoi(buf)) > 255))))
            return meFALSE;
get_scheme:        
        /* Get the colour index. This uses the root hilight 
         * colour if not specified. */
        ii = meGetString((meUByte *)"Scheme",0,0,buf,meBUF_SIZE_MAX);
        if (ii < 0)
            return meFALSE ;
        
        if (ii > 0) 
        {
            if ((ii=convertUserScheme(meAtoi(buf),-1)) < 0)
                return meFALSE ;
            node->scheme = ii ;
        }
        else
            node->scheme = root->scheme ;
    }
    return meTRUE ;
}


#define findTokenCharTest(ret,lastChar,srcText,tokTest,testStr)              \
do {                                                                         \
    meUByte *__ts, ftctcc ;                                                  \
    lastChar='\0';                                                           \
    if((ftctcc=*srcText++) == '\0') ftctcc=meCHAR_NL ;                       \
    switch(tokTest & meHIL_TEST_MASK)                                        \
    {                                                                        \
    case meHIL_TEST_BACKREF:                                                 \
	__ts = testStr ;                                                     \
	ret = (varTable[*testStr++] == ftctcc) ;                             \
	break ;                                                              \
    case meHIL_TEST_SINGLE:                                                  \
	__ts = testStr ;                                                     \
	ret = (*testStr++ == ftctcc) ;                                       \
	break ;                                                              \
    case meHIL_TEST_CLASS:                                                   \
        {                                                                    \
	    meUByte rc, nrc ;                                                \
                                                                             \
	    __ts = testStr ;                                                 \
	    for(ret=*testStr++ ; ret>0 ; ret--)                              \
            {                                                                \
		rc=*testStr++ ;                                              \
		nrc = *testStr ;                                             \
		if((nrc == '-') && (ret > 1))                                \
                {                                                            \
		    /* range */                                              \
		    testStr++ ;                                              \
		    nrc = *testStr++ ;                                       \
		    ret -= 2 ;                                               \
		    if((ftctcc >= rc) && (ftctcc <= nrc))                    \
                    {                                                        \
			testStr += ret - 1 ;                                 \
			break ;                                              \
		    }                                                        \
		}                                                            \
		/* single char compare */                                    \
		else if(ftctcc == rc)                                        \
                {                                                            \
		    testStr += ret - 1 ;                                     \
		    break ;                                                  \
		}                                                            \
	    }                                                                \
	    break ;                                                          \
	}                                                                    \
    case meHIL_TEST_EOL:                                                     \
	ret = (ftctcc == meCHAR_NL) ;                                        \
	break ;                                                              \
    case meHIL_TEST_SPACE:                                                   \
	ret = isSpace(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_DIGIT:                                                   \
	ret = isDigit(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_XDIGIT:                                                  \
	ret = isXDigit(ftctcc) ;                                             \
	break ;                                                              \
    case meHIL_TEST_LOWER:                                                   \
	ret = isLower(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_UPPER:                                                   \
	ret = isUpper(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_ALPHA:                                                   \
	ret = isAlpha(ftctcc) ;                                              \
	break ;                                                              \
    case meHIL_TEST_ALNUM:                                                   \
	ret = isAlphaNum(ftctcc) ;                                           \
	break ;                                                              \
    case meHIL_TEST_SWORD:                                                   \
	ret = isSpllWord(ftctcc) ;                                           \
	break ;                                                              \
    case meHIL_TEST_WORD:                                                    \
	ret = isWord(ftctcc) ;                                               \
	break ;                                                              \
    case meHIL_TEST_ANY:                                                     \
	ret = 1 ;                                                            \
	break ;                                                              \
    }                                                                        \
    if(tokTest & meHIL_TEST_INVERT)                                          \
	ret = !ret ;                                                         \
    if(!ret)                                                                 \
    {                                                                        \
	if(tokTest & meHIL_TEST_MATCH_NONE)                                  \
        {                                                                    \
	    ret = 1 ;                                                        \
	    srcText-- ;                                                      \
	}                                                                    \
	tokTest = 0 ;                                                        \
    }                                                                        \
    else if((lastChar=ftctcc) == meCHAR_NL)                                  \
	tokTest = 0 ;                                                        \
    else if(tokTest & meHIL_TEST_MATCH_MULTI)                                \
    {                                                                        \
	if((tokTest & meHIL_TEST_MASK) <= meHIL_TEST_CLASS)                  \
	    testStr = __ts ;                                                 \
	tokTest |= meHIL_TEST_MATCH_NONE ;                                   \
    }                                                                        \
} while(tokTest & meHIL_TEST_MATCH_MULTI)

static int
__findTokenSingleCharTest(meUByte cc, meUByte tokTest)
{
    int ret ;
    switch(tokTest & meHIL_TEST_MASK)
    {
    case meHIL_TEST_EOL:
        ret = (cc == '\0') ;
        break ;
    case meHIL_TEST_SPACE:
        ret = isSpace(cc) ;
        break ;
    case meHIL_TEST_DIGIT:
        ret = isDigit(cc) ;
        break ;
    case meHIL_TEST_XDIGIT:
        ret = isXDigit(cc) ;
        break ;
    case meHIL_TEST_LOWER:
        ret = isLower(cc) ;
        break ;
    case meHIL_TEST_UPPER:
        ret = isUpper(cc) ;
        break ;
    case meHIL_TEST_ALPHA:
        ret = isAlpha(cc) ;
        break ;
    case meHIL_TEST_ALNUM:
        ret = isAlphaNum(cc) ;
        break ;
    case meHIL_TEST_SWORD:
        ret = isSpllWord(cc) ;
        break ;
    case meHIL_TEST_WORD:
        ret = isWord(cc) ;
        break ;
    case meHIL_TEST_ANY:
        ret = 1 ;
        break ;
    }
    if(tokTest & meHIL_TEST_INVERT)
        ret = !ret ;
    return ret ;
}    

#define findTokenSingleCharTest(cc,tokTest)                                  \
((tokTest == meHIL_TEST_INVALID) || __findTokenSingleCharTest(cc,tokTest))

static meHilight *
findToken(meHilight *root, meUByte *text, meUByte mode, 
          meUByte lastChar, meUShort *len)
{
    meHilight *nn, *node ;
    meUByte  cc, *s1, *s2;
    int status;
    int hi, low, mid ;
    meUShort type ;
    
    if(root->table == NULL)
    {
        low = root->ordSize ;
        hi  = low+root->listSize ;
    }
    else
    {
        cc = *text ;
        if(mode & meHIL_MODECASE)
            cc = toLower(cc) ;
        low = GetTableSlot(cc) ;
        hi  = root->table[low+1] ;
        low = root->table[low] ;
    }
    if(low != hi)
    {
        hi-- ;
        if(mode & meHIL_MODECASE)
        {
            do
            {
                mid = (low + hi) >> 1;		/* Get mid value. */
                s1 = text ;
                nn = root->list[mid] ;
                s2 = (meUByte *) nn->token ;
                do {
                    cc = *s1++ ;
                    if((status=((int) toLower(cc)) - ((int) *s2++)) != 0)
                        break ;
                } while(*s2 != '\0') ;
                if(status == 0)
                    break ;
                else if (status < 0)
                {
                    if((status == '\0' - meCHAR_NL) && (cc == '\0') && (*s2 == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        break ;
                    }
                    hi  = mid-1 ;		/* Discard bottom half */
                }
                else
                    low = mid+1 ;		/* Discard top half */
            } while(low <= hi) ;		/* Until converges */
        }
        else
        {
            do
            {
                mid = (low + hi) >> 1;		/* Get mid value. */
                s1 = text ;
                nn = root->list[mid] ;
                s2 = (meUByte *) nn->token ;
                do {
                    if((status=((int) (cc=*s1++)) - ((int) *s2++)) != 0)
                        break ;
                } while(*s2 != '\0') ;
                if(status == 0)
                    break ;
                else if (status < 0)
                {
                    if((status == '\0' - meCHAR_NL) && (cc == '\0') && (*s2 == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        break ;
                    }
                    hi  = mid-1 ;		/* Discard bottom half */
                }
                else
                    low = mid+1 ;		/* Discard top half */
            } while(low <= hi) ;		/* Until converges */
        }
        if(low <= hi)
        {
            type = nn->type ;
            if((cc != '\0') && (hilListSize(nn) > 0) &&
               (!(type & HLASTTLINE) || (mode & meHIL_MODESTTLN)) &&
               (!(type & HLASOL) || (mode & meHIL_MODESTART)) &&
               ((node = findToken(nn,s1,mode,lastChar,len)) != NULL))
            {
                *len += s1-text ;
                return node ;
            }
            if((((type & HLVALID) == 0) || ((type & HLONELNBT) && (mode & meHIL_MODELOOKB))) && 
               (((mode & meHIL_MODETOEOL) == 0) || (type & HLBRINEOL)) &&
               (((type & HLSTTLINE) == 0) || (mode & meHIL_MODESTTLN)) &&
               (((type & HLSOL) == 0) || (mode & meHIL_MODESTART)) &&
               (findTokenSingleCharTest(lastChar,nn->tknSttTst) ||
                ((mode & meHIL_MODETOKEWS) && (mode & meHIL_MODETOKEND) &&
                 findTokenSingleCharTest(' ',nn->tknSttTst))) &&
               findTokenSingleCharTest(*s1,nn->tknEndTst))
            {
                *len = s1-text ;
                return nn ;
            }
        }
    }
    if((hi = root->ordSize) > 0)
    {
        /* note that findTokenCharTest may not set lstc even if it does 
         * match as the ? & * may match 0 chars - so theres no last!
         * Therefore initialize to a sensible value */
        meUByte dd, lstc ;
        mid = 0 ;
        
        for( ; mid < hi ; mid++)
        {
            nn = root->list[mid] ;
            s2 = nn->token ;
            dd = *s2++ ;
            s2++ ;
            s1 = text ;
            findTokenCharTest(cc,lstc,s1,dd,s2) ;
            if(cc)
            {
                if(lstc != meCHAR_NL)
                {
                    if(mode & meHIL_MODECASE)
                    {
                        while((dd=*s2++) != '\0')
                        {
                            cc = *s1++ ;
                            if((toLower(cc)) != dd)
                                break ;
                        }
                    }
                    else
                    {
                        while((dd=*s2++) != '\0')
                            if((cc=*s1++) != dd)
                                break ;
                    }
                    if((dd == meCHAR_NL) && (cc == '\0') &&
                       (nn->tknEndTst == meHIL_TEST_INVALID))
                    {
                        if(!nn->tknEndOff)
                            s1-- ;
                        dd = *s2 ;
                    }
                }
                else
                {
                    if(!nn->tknEndOff)
                        s1-- ;
                    dd = (nn->tknEndTst == meHIL_TEST_INVALID) ? *s2:0 ;
                    cc = '\0' ;
                }
                if(dd == '\0')
                {
                    varTable[nn->token[1]] = lstc ;
                    type = nn->type ;
                    if((cc != '\0') && (hilListSize(nn) > 0) &&
                       (!(type & HLASTTLINE) || (mode & meHIL_MODESTTLN)) &&
                       (!(type & HLASOL) || (mode & meHIL_MODESTART)) &&
                       ((node = findToken(nn,s1,mode,lastChar,len)) != NULL))
                    {
                        *len += s1-text ;
                        return node ;
                    }
                    if((((type & HLVALID) == 0) || ((type & HLONELNBT) && (mode & meHIL_MODELOOKB))) &&
                       (((mode & meHIL_MODETOEOL) == 0) || (type & HLBRINEOL)) &&
                       (((type & HLSTTLINE) == 0) || (mode & meHIL_MODESTTLN)) &&
                       (((type & HLSOL) == 0) || (mode & meHIL_MODESTART)) &&
                       (findTokenSingleCharTest(lastChar,nn->tknSttTst) ||
                        ((mode & meHIL_MODETOKEWS) && (mode & meHIL_MODETOKEND) &&
                         findTokenSingleCharTest(' ',nn->tknSttTst))) &&
                        findTokenSingleCharTest(*s1,nn->tknEndTst))
                    {
                        *len = s1-text ;
                        return nn ;
                    }
                }
            }
        }
    }
    return NULL ;
}

#define __hilCopyChar(dstPos,cc,tw)                                          \
do                                                                           \
{                                                                            \
    /* the largest character size is a tab which is user definable */        \
    if(dstPos >= disLineSize)                                                \
    {                                                                        \
        disLineSize += 512 ;                                                 \
        disLineBuff = meRealloc(disLineBuff,disLineSize+32) ;                \
    }                                                                        \
    if(isDisplayable(cc))                                                    \
    {                                                                        \
        if(cc == ' ')                                                        \
            disLineBuff[dstPos++] = displaySpace ;                           \
        else if(cc == meCHAR_TAB)                                            \
            disLineBuff[dstPos++] = displayTab ;                             \
        else                                                                 \
            disLineBuff[dstPos++] = cc ;                                     \
    }                                                                        \
    else if(cc == meCHAR_TAB)                                                \
    {                                                                        \
        int ii=get_tab_pos(dstPos,tw) ;                                      \
        disLineBuff[dstPos++] = displayTab ;                                 \
        while(--ii >= 0)                                                     \
            disLineBuff[dstPos++] = ' ' ;                                    \
    }                                                                        \
    else if(cc < 0x20)                                                       \
    {                                                                        \
        disLineBuff[dstPos++] = '^' ;                                        \
        disLineBuff[dstPos++] = cc ^ 0x40 ;                                  \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        /* Its a nasty character */                                          \
        disLineBuff[dstPos++] = '\\' ;                                       \
        disLineBuff[dstPos++] = 'x' ;                                        \
        disLineBuff[dstPos++] = hexdigits[cc/0x10] ;                         \
        disLineBuff[dstPos++] = hexdigits[cc%0x10] ;                         \
    }                                                                        \
}                                                                            \
while(0)

/*
 * hilSchemeChange.
 * This function manages the scheme changes. Each change in scheme results
 * in a call. The changes of scheme are added to the "hd->blkp" structure.
 * In addition the last block node is retained in "hd->hnode" which is the
 * current hilighting scheme. The selection hilighting uses this node to 
 * switch the selection schemes on an off. The correct state is mainated by
 * this function.
 */
static void
hilSchemeChange (meHilight *node, HILDATA *hd)
{
    register meSchemeSet *blkp;
    
    if (hd->noColChng > 0)
    {
        /* Re-allocate a new hilight block - run out of room */
        if(hd->noColChng == hilBlockS)
        {
            hilBlockS += 20 ;
            /* add 2 to hilBlockS to allow for a double trunc-scheme change */
            hilBlock = meRealloc(hilBlock, (hilBlockS+2)*sizeof(meSchemeSet)) ;
        }
        blkp = hilBlock + hd->noColChng + 1 ;
    }
    else
    {
        blkp = hilBlock + 1 ;
        blkp->column = 0;
    }
    /* Apply the new change of colour to the new enry created. */
    hd->noColChng++ ;
    blkp->scheme = node->scheme + (meScheme)(hd->colno);
    hd->hnode = node;
    hd->blkp = blkp;
}

static int
hilCopyChar(register int dstPos, register meUByte cc, HILDATA *hd)
{
    /* Change the selection hilight if required. */
    if ((hd->srcOff != 0xffff) && (hd->srcPos >= hd->srcOff))
        hd->hfunc (dstPos, hd);
    /* Copy the character. */
    __hilCopyChar(dstPos,cc,hd->tabWidth);
    return dstPos ;
}

static int
hilCopyReplaceString(int sDstPos, register meUByte *srcText,
                     int len, HILDATA *hd)
{
    int dstPos = sDstPos ;
    int hoff, dstLen ;
    meUByte cc ;
    
    if((hd->srcOff != 0xffff) && (hd->srcPos+len >= hd->srcOff))
    {
        /* as the source len and destination len could be different, the best
         * we can do is set the hilight change position to be the ratio
         * between the two */
        meUByte *ss=srcText ;
        dstLen = 0 ;
        while((cc = *ss++) != 0)
        {
            if(cc == meCHAR_LEADER)
                ss++ ;
            dstLen++ ;
        }
        if(len)
            hoff = sDstPos+((hd->srcOff-hd->srcPos)*dstLen/len) ;
        else
            hoff = sDstPos+(hd->srcOff-hd->srcPos)*dstLen ;
    }
    else
        hoff = 0x7fffffff ;
    while((cc = *srcText++) != '\0')
    {
        if((cc == meCHAR_LEADER) && ((cc=*srcText++) != meCHAR_TRAIL_LEADER))
            cc = varTable[cc] ;
        if(dstPos >= hoff)
        {
            hd->hfunc (dstPos, hd);
            /* we may have another offset to cope with */
            if((hd->srcOff != 0xffff) && (hd->srcPos+len >= hd->srcOff))
            {
                if(len)
                    hoff = sDstPos+((hd->srcOff-hd->srcPos)*dstLen/len) ;
                else
                    hoff = sDstPos+(hd->srcOff-hd->srcPos)*dstLen ;
            }
            else
                hoff = 0x7fffffff ;
        }
        __hilCopyChar(dstPos,cc,hd->tabWidth);
    }
    /* This is not quite right - but will have to do for present. Change
     * the hilighting according to the ammount of text used. */
    return dstPos ;
}

static int
hilCopyString(register int dstPos, register meUByte *srcText,HILDATA *hd)
{
    meUByte cc ;
    /* Handle the selection hilighting if enabled. */
    if (hd->srcOff != 0xffff)
    {
        meUShort srcPos;
        
        srcPos = hd->srcPos;
        while((cc = *srcText++) != '\0')
        {
            if (hd->srcOff <= srcPos)
                (hd->hfunc)(dstPos, hd);
            __hilCopyChar(dstPos,cc,hd->tabWidth);
            srcPos++;
        }
    }
    else
    {
        while((cc = *srcText++) != '\0')
            __hilCopyChar(dstPos,cc,hd->tabWidth);
    }
    return dstPos ;
}

static int
hilCopyLenString(register int dstPos, register meUByte *srcText,
                 register int len, HILDATA *hd)
{
    meUByte cc ;
    
    /* Handle the selection hilighting if enabled. */
    if ((hd->srcOff != 0xffff) && ((hd->srcOff - hd->srcPos) < len))
    {
        meUShort srcPos;
        
        srcPos = hd->srcPos;
        while (--len >= 0)
        {
            if (hd->srcOff <= srcPos)
                (hd->hfunc)(dstPos, hd);
            
            cc = *srcText++ ;
            __hilCopyChar(dstPos,cc,hd->tabWidth);
            srcPos++;
        }
    }
    else
    {
        while(len--)
        {
            cc = *srcText++ ;
            __hilCopyChar(dstPos,cc,hd->tabWidth);
        }
    }
    return dstPos ;
}


/*
 * hilHandleCallback
 * This function handles the callbacks when the text has reached a designated
 * position in the source line.
 */
static void
hilHandleCallback (int dstPos, struct HILDATA *hd)
{
    /* Handle the start of the selection hilight */
    if (hd->flag & VFSHBEG)
    {
        /* Change to selection hilighting */
        hd->colno += meSCHEME_SELECT;    /* To new index */
        hd->blkp->column = dstPos;       /* Terminate the current style */
        hilSchemeChange (hd->hnode, hd); /* New colour node */
        hd->blkp->column = dstPos;       /* New end of this style */
        hd->flag &= ~VFSHBEG;            /* Knock off the background mode */
        
        /* Reprime the callback with the location of the end of hilight */
        if (hd->flag & VFSHEND)
            hd->srcOff = selhilight.eoff;
        else
            hd->srcOff = 0xffff ; /* Set to unreachable value */
    }
    else if (hd->flag & VFSHEND)
    {
        /* Restore the existing hilighting */
        hd->colno -= meSCHEME_SELECT;    /* To new index */
        hd->blkp->column = dstPos;       /* Terminate current colour */
        hilSchemeChange (hd->hnode, hd); /* New colour node */
        hd->blkp->column = dstPos;       /* New end of this colour */
        hd->flag &= ~VFSHEND;            /* Remove the end flag */
        hd->srcOff = 0xffff ;            /* Set to unreachable value */
    }
}

meUShort 
hilightLine(meVideoLine *vp1, meUByte mode)
{
    meUByte *srcText=vp1->line->text ;
    meUByte  hilno = vp1->hilno, cc=meCHAR_NL ;
    meHilight *node;
    meUShort len ;
    register int srcWid=vp1->line->length, dstPos=0 ;
    HILDATA hd;
    
    /* Initialise the hilight data structure */
    hd.root = hilights[hilno];          /* Root of the hilighting */
    hd.flag = vp1->flag;                /* Cache the video flag */
    hd.noColChng = 0;                   /* Reset number of colour changes */
    hd.srcPos = 0;                      /* Reset the source position */
    hd.srcOff = 0xffff;                 /* No callback required */
    hd.blkp = hilBlock + 1;             /* block pointer */
    hd.tabWidth = vp1->wind->buffer->tabWidth;
    
    /* Determine if we are processing a forground or hilighted line.
     * Determined by the current line flag */
    hd.colno = meSCHEME_NORMAL ;
    if (hd.flag & VFCURRL) 
        hd.colno += meSCHEME_CURRENT ;
    
    /* Determine if we are handling a selection hilighted line.
     * If so then set up the starting background colour in accordance
     * with the start/end/all selection hilighting flags in the video
     * structure. */
    if (hd.flag & VFSHMSK)              /* Selection hilight to be done ? */
    {
        hd.colno += meSCHEME_SELECT ;   /* Assume selection hilight enabled */
        
        if (hd.flag & VFSHALL)          /* Whole line hilighted ?? */
            hd.flag &= ~VFSHMSK;        /* Yes - kill off the flag */
        else
        {
            /* Check for the beginning of a slection hilight. If it is at the 
             * beginning of the line then use this as the starting colour. */
            if (hd.flag & VFSHBEG)
            {
                if (selhilight.soff!=0) /* Starts in column 0 ?? */
                {   /* Does not start in column 0 */
                    hd.colno -= meSCHEME_SELECT ;   /* remove selection hilight */
                    hd.srcOff = selhilight.soff;
                    hd.hfunc = hilHandleCallback;
                }
                else
                    hd.flag &=~VFSHBEG; /* Knock off the begin - found */
            }
            
            /* Check for the end of a selection hilight. If it is at the 
             * beginning of the line then it effectivelly disables the 
             * hilight. */
            if ((hd.flag & VFSHEND) && ((hd.flag & VFSHBEG) == 0))
            {
                if (selhilight.eoff==0) /* End's in column 0 ?? */
                {
                    hd.colno -= meSCHEME_SELECT; /* Yes - use normal colour */
                    hd.flag &=~VFSHEND;     /* Knock off end - found */
                }
                else
                {
                    hd.srcOff = selhilight.eoff;
                    hd.hfunc = hilHandleCallback;
                }
            }
        }
    }
    
    mode |= (meHIL_MODESTTLN|meHIL_MODESTART|(meHilightGetFlags(hd.root) & (HFCASE|HFTOKEWS))) ;
    if(vp1->bracket != NULL)
    {
        node = vp1->bracket ;
        hilSchemeChange ((hd.cnode = node), &hd);
        goto BracketJump ;
    }
    hilSchemeChange ((hd.cnode = hd.root), &hd);
    
    for(;;)
    {
        node = meHilightGetColumnHilight(hd.root) ;
        while(node != NULL)
        {
            if(meHilightGetFromColumn(node) == hd.srcPos)
            {
                if((len = (meUShort) meHilightGetToColumn(node)) > srcWid)
                    len = srcWid ;
                len -= hd.srcPos ;
                goto column_token ;
            }
            else if(meHilightGetFromColumn(node) > hd.srcPos)
                break ;
            node = meHilightGetColumnHilight(node) ;
        }
        if((node = findToken(hd.root,srcText+hd.srcPos,mode,cc,&len)) != NULL)
        {
column_token:
            if(node->tknEndOff)
                len -= node->tknEndOff ;
            if(len == 0)
            {
                /* the match string was 0 in length we will not move forward.
                 * This could lead to a spin, but not always as this could be
                 * a branch token.
                 * First time around set a flag to say we're here, next time
                 * around do something about it */
                if(mode & meHIL_MODEMOVE)
                    goto advance_char ;
                mode |= meHIL_MODEMOVE ;
            }
            else
                mode &= ~meHIL_MODEMOVE ;
            if(node->tknSttOff)
            {
                dstPos = hilCopyLenString(dstPos,srcText+hd.srcPos,node->tknSttOff,&hd) ;
                hd.srcPos += node->tknSttOff ;
                len -= node->tknSttOff ;
            }
            
            if(hd.blkp->column == dstPos)
            {
                hd.blkp->scheme = node->scheme + hd.colno ;
                hd.hnode = node;
            }
            else
            {
                hd.blkp->column = dstPos ;
                hilSchemeChange (node, &hd);
                hd.blkp->column = dstPos ;
            }
            if(node->type & HLREPLACE)
                dstPos = hilCopyReplaceString(dstPos,node->rtoken,len,&hd) ;
            else if(len)
                dstPos = hilCopyLenString(dstPos,srcText+hd.srcPos,len,&hd) ;
            
            hd.srcPos += len ;
            
BracketJump:
            if(node->type & HLBRACKET)
            {
                meUByte ignore, tt=1, *c1, *c2, ss, *s1, *s2, solt ;
                
                c1 = (meUByte *) node->close ;
                /* the SOL is a special close bracket code for '^' */
                if((solt = ((c1[0] == meCHAR_LEADER) && (c1[1] == meHIL_TEST_SOL))))
                    c1 += 2 ;
                ignore = node->ignore ;
                s1 = srcText+hd.srcPos ;
                for( ; ; hd.srcPos++)
                {
                    if(!solt || (hd.srcPos == 0))
                    {                       
                        c2 = c1 ;
                        s2 = s1 ;
                        while((tt = *c2++) != '\0')
                        {
                            if((tt == meCHAR_LEADER) && ((tt = *c2++) != meCHAR_TRAIL_LEADER))
                            {
                                meUByte ret, vv ;
                                
                                vv = *c2++ ;
                                findTokenCharTest(ret,ss,s2,tt,c2) ;
                                if(!ret)
                                {
                                    tt = 1 ;
                                    break ;
                                }
                                varTable[vv] = ss ;
                            }
                            else
                            {
                                if((ss = *s2++) == '\0')
                                    ss = meCHAR_NL ;
                                else if(mode & meHIL_MODECASE)
                                    ss = toLower(ss) ;
                                if(tt != ss)
                                    break ;
                            }
                            if(ss == meCHAR_NL)
                            {
                                if((tt=*c2) != '\0')
                                {
                                    if((tt != meCHAR_LEADER) || (c2[1] != (meHIL_TEST_VALID|meHIL_TEST_EOL)) ||
                                       (c2[3] != '\0'))
                                        break ;
                                    c2 += 3 ;
                                    tt = '\0' ;
                                }
                                if(!node->clsEndOff)
                                    s2-- ;
                            }
                        }
                    }
                    else
                        tt = 1 ;
                    if((tt == '\0') || (hd.srcPos == srcWid))
                        break ;
                    ss = *s1++ ;
                    dstPos = hilCopyChar(dstPos,ss, &hd) ;
                    if(ss == ignore)
                    {
                        if(++hd.srcPos == srcWid)
                            break ;
                        dstPos = hilCopyChar(dstPos,*s1++,&hd) ;
                    }
                }
                if(tt != '\0')
                {
                    hd.blkp->column = dstPos ;
                    if(node->type & HLSNGLLNBT)
                        node = NULL ;
                    goto hiline_exit ;
                }
                len = s2-s1 ;
                if(node->clsSttOff)
                {
                    dstPos = hilCopyLenString(dstPos,s1,node->clsSttOff,&hd) ;
                    hd.srcPos += node->clsSttOff ;
                    s1 += node->clsSttOff ;
                    len -= node->clsSttOff ;
                }
                if(node->clsEndOff)
                    len -= node->clsEndOff ;
                if(node->type & HLREPLACE)
                    dstPos = hilCopyReplaceString(dstPos,node->rclose,len,&hd) ;
                else
                    dstPos = hilCopyLenString(dstPos,s1,len,&hd) ;
                hd.srcPos += len ;
                if(hd.cnode == node)
                    hd.cnode = hd.root ;
            }
            else if(node->type & HLCONTIN)
            {
                char *ss ;
                int   pos ;
                
                ss  = (char *) node->close ;
                len = strlen(ss) ;
                pos = srcWid ;
                while((srcText[--pos] == ' ') && (pos > 0))
                    ;
                pos -= len-1 ;
                if((pos >= hd.srcPos) &&
                   !strncmp((char *)srcText+pos,ss,len))
                {
                    dstPos = hilCopyString(dstPos,srcText+hd.srcPos, &hd) ;
                    hd.blkp->column = dstPos ;
                    goto hiline_exit ;
                }
            }
            if(node->type & HLENDLINE)
            {
                if((node->type & HLREPLACE) && (node->type & HLREPTOEOL))
                {
                    hd.blkp->column = dstPos ;
                    node = NULL ;
                    goto hiline_exit ;
                }
                if(!(node->type & HLBRINEOL) || (hd.srcPos == srcWid))
                {
                    dstPos = hilCopyString(dstPos,srcText+hd.srcPos, &hd) ;
                    hd.blkp->column = dstPos ;
                    node = NULL ;
                    goto hiline_exit ;
                }
                hd.blkp->column = dstPos ;
                mode |= meHIL_MODETOEOL ;
                hd.cnode = node;
            }
            else
            {
                if(node->type & HLBRANCH)
                {
                    if((hilno = node->ignore) == 0)
                        hilno = vp1->wind->buffer->hilight ;
                    meHilightBranchType = node->type ;
                    mode &= ~meHIL_MODETOEOL ;
                    hd.root = hilights[hilno] ;
                    hd.cnode = hd.root;
                }
                if(hd.blkp->column == dstPos)
                {
                    hd.blkp->scheme = hd.cnode->scheme + hd.colno ;
                    hd.hnode = node;
                }
                else
                {
                    hd.blkp->column = dstPos ;
                    hilSchemeChange(hd.cnode, &hd);
                    hd.blkp->column = dstPos ;
                }
            }
            if(dstPos)
            {
                cc = disLineBuff[dstPos-1] ;
                mode &= ~(meHIL_MODESTTLN|meHIL_MODESTART) ;
            }
            mode |= meHIL_MODETOKEND ;
        }
        else
        {
advance_char:
            if((cc=srcText[hd.srcPos]) == 0)
                break ;
            mode &= ~(meHIL_MODEMOVE|meHIL_MODESTART|meHIL_MODETOKEND) ;
            if((mode & meHIL_MODESTTLN) && !isSpace(cc))
                mode &= ~meHIL_MODESTTLN ;
            dstPos = hilCopyChar(dstPos,cc, &hd) ;
            hd.srcPos++ ;
        }
    }
    
    if(hd.blkp->column != dstPos)
        hd.blkp->column = dstPos ;
    node = NULL ;
hiline_exit:
    
    if((hd.flag & (VFSHBEG|VFSHEND)) == VFSHEND)
    {
        /* must end the selection hilighting */
        hd.colno -= meSCHEME_SELECT ;
        hilSchemeChange (hd.hnode, &hd);
        hd.blkp->column = dstPos;
    }
    else if((hd.flag & (VFSHBEG|VFSHEND)) == VFSHBEG)
    {
        /* must end the selection hilighting */
        hd.colno += meSCHEME_SELECT ;
        hilSchemeChange (hd.hnode, &hd);
        hd.blkp->column = dstPos;
    }
    
    if((vp1[0].hilno != hilno) && (meHilightBranchType & HLSNGLLNBT))
    {
        hilno = vp1[0].hilno ;
        node = NULL ;
    }
    if((vp1[1].hilno != hilno) || (vp1[1].bracket != node))
    {
        vp1[1].flag |= VFCHNGD ;
        vp1[1].hilno = hilno ;
        vp1[1].bracket = node ;
    }
    return hd.noColChng ;
}

#define hilOffsetChar(off,dstPos,dstJmp,cc,tw)                               \
{                                                                            \
    int ii ;                                                                 \
    if(isDisplayable(cc))                                                    \
        ii = 1 ;                                                             \
    else if(cc == meCHAR_TAB)                                                \
        ii = get_tab_pos(dstPos,tw) + 1 ;                                    \
    else if(cc < 0x20)                                                       \
        ii = 2 ;                                                             \
    else                                                                     \
        ii = 4 ;                                                             \
    if(dstJmp)                                                               \
    {                                                                        \
        *off++ = ii+dstJmp ;                                                 \
        dstJmp = 0 ;                                                         \
    }                                                                        \
    else                                                                     \
        *off++ = ii ;                                                        \
    dstPos += ii ;                                                           \
}

#define hilOffsetString(lastcc,off,dstPos,dstJmp,srcText,tw)                 \
{                                                                            \
    meUByte __cc, *__ss=(srcText) ;                                          \
    while((__cc = *__ss++) != '\0')                                          \
    {                                                                        \
        hilOffsetChar(off,dstPos,dstJmp,__cc,tw)                             \
        lastcc = __cc ;                                                      \
    }                                                                        \
}

#define hilOffsetLenString(lastcc,off,dstPos,dstJmp,srcText,len,tw)          \
{                                                                            \
    meUShort __ll=len ;                                                      \
    meUByte *__ss=(srcText) ;                                                \
    while(__ll--)                                                            \
    {                                                                        \
        lastcc = *__ss++ ;                                                   \
        hilOffsetChar(off,dstPos,dstJmp,lastcc,tw)                           \
    }                                                                        \
}

static int
hilOffsetReplaceString(register meUByte **offPtr, register int dstPos,
                       int *dstJmp, register meUByte *srcText, int len, int tw)
{
    meUByte *off=*offPtr, cc, lastcc='\0' ;
    int            rlen=dstPos, ii, dd=*dstJmp ;
    
    while((cc = *srcText++) != '\0')
    {
        if((cc == meCHAR_LEADER) && ((cc=*srcText++) != meCHAR_TRAIL_LEADER))
            cc = varTable[cc] ;
        lastcc = cc ;
        hilOffsetChar(off,dstPos,dd,cc,tw)
    }
    
    rlen = dstPos-rlen ;
    if(len)
    {
        dd = *dstJmp ;
        off = *offPtr ;
        while(len)
        {
            ii = rlen / len-- ;
            if(dd)
            {
                *off++ = ii+dd ;
                dd = 0 ;
                *dstJmp = 0 ;
            }
            else
                *off++ = ii ;
            rlen -= ii ;
        }
        *offPtr = off ;
    }
    else
        *dstJmp += rlen ;
    *off = lastcc ;
    return dstPos ;
}

void
hilightCurLineOffsetEval(meWindow *wp)
{
    meUByte *srcText=wp->dotLine->text ;
    meUByte *off=wp->dotCharOffset->text ;
    meUByte  hilno=wp->buffer->hilight, cc=meCHAR_NL, mode ;
    meUByte  tw = wp->buffer->tabWidth;
    meHilight *node, *root ;
    meUShort len ;
    register int srcWid=wp->dotLine->length, srcPos=0, dstPos=0 ;
    int dstJmp=0 ;
    
    root = hilights[hilno];          /* Root of the hilighting */
    
    mode = (meHIL_MODESTTLN|meHIL_MODESTART|(meHilightGetFlags(root) & HFCASE)) ;
    /*    if(vp1->bracket != NULL)*/
    /*    {*/
    /*        node = vp1->bracket ;*/
    /*        hilSchemeChange ((hd.cnode = node), &hd);*/
    /*        goto BracketJump ;*/
    /*    }*/
    for(;;)
    {
        node = meHilightGetColumnHilight(root) ;
        while(node != NULL)
        {
            if(meHilightGetFromColumn(node) == srcPos)
            {
                if((len = (meUShort) meHilightGetToColumn(node)) > srcWid)
                    len = srcWid ;
                len -= srcPos ;
                goto column_token ;
            }
            else if(meHilightGetFromColumn(node) > srcPos)
                break ;
            node = meHilightGetColumnHilight(node) ;
        }
        if((node = findToken(root,srcText+srcPos,mode,cc,&len)) != NULL)
        {
column_token:
            if(node->tknEndOff)
                len -= node->tknEndOff ;
            if(len == 0)
            {
                /* the match string was 0 in length we will not move forward.
                 * This could lead to a spin, but not always as this could be
                 * a branch token.
                 * First time around set a flag to say we're here, next time
                 * around do something about it */
                if(mode & meHIL_MODEMOVE)
                    goto advance_char ;
                mode |= meHIL_MODEMOVE ;
            }
            else
                mode &= ~meHIL_MODEMOVE ;
            if(node->type & HLREPLACE)
            {
                if(node->tknSttOff)
                {
                    hilOffsetLenString(cc,off,dstPos,dstJmp,srcText+srcPos,node->tknSttOff,tw) ;
                    srcPos += node->tknSttOff ;
                    len -= node->tknSttOff ;
                }
                dstPos = hilOffsetReplaceString(&off,dstPos,&dstJmp,node->rtoken,len,tw) ;
                if(*off)
                    cc = *off ;
            }
            else
            {
                hilOffsetLenString(cc,off,dstPos,dstJmp,srcText+srcPos,len,tw) ;
            }
            srcPos += len ;
            
            /*BracketJump:*/
            if(node->type & HLBRACKET)
            {
                meUByte ignore, tt=1, *c1, ss, *c2, *s1, *s2, solt ;
                
                c1 = (meUByte *) node->close ;
                /* the SOL is a special close bracket code for '^' */
                if((solt = ((c1[0] == meCHAR_LEADER) && (c1[1] == meHIL_TEST_SOL))))
                    c1 += 2 ;
                ignore = node->ignore ;
                s1 = srcText+srcPos ;
                for( ; ; srcPos++)
                {
                    if(!solt || (srcPos == 0))
                    {                       
                        c2 = c1 ;
                        s2 = s1 ;
                        while((tt = *c2++) != '\0')
                        {
                            if((tt == meCHAR_LEADER) && ((tt = *c2++) != meCHAR_TRAIL_LEADER))
                            {
                                meUByte ret, vv ;
                                
                                vv = *c2++ ;
                                findTokenCharTest(ret,ss,s2,tt,c2) ;
                                if(!ret)
                                {
                                    tt = 1 ;
                                    break ;
                                }
                                varTable[vv] = ss ;
                            }
                            else if((ss = *s2++) != tt)
                            {
                                if((tt != meCHAR_NL) || (ss != '\0'))
                                    break ;
                            }
                            if(ss == '\0')
                            {
                                if((tt=*c2) != '\0')
                                {
                                    if((tt != meCHAR_LEADER) || (c2[1] != (meHIL_TEST_VALID|meHIL_TEST_EOL)) ||
                                       (c2[3] != '\0'))
                                        break ;
                                    c2 += 3 ;
                                    tt = '\0' ;
                                }
                                if(!node->clsEndOff)
                                    s2-- ;
                            }
                        }
                    }
                    else
                        tt = 1 ;
                    if((tt == '\0') || (srcPos == srcWid))
                        break ;
                    ss = *s1++ ;
                    hilOffsetChar(off,dstPos,dstJmp,ss,tw) ;
                    if(ss == ignore)
                    {
                        if(++srcPos == srcWid)
                            break ;
                        ss = *s1++ ;
                        hilOffsetChar(off,dstPos,dstJmp,ss,tw) ;
                    }
                }
                if(tt != '\0')
                    goto hiline_exit ;
                
                len = s2-s1 ;
                
                if(node->clsEndOff)
                    len -= node->clsEndOff ;
                
                if(node->type & HLREPLACE)
                {
                    if(node->clsSttOff)
                    {
                        hilOffsetLenString(cc,off,dstPos,dstJmp,s1,node->clsSttOff,tw) ;
                        srcPos += node->clsSttOff ;
                        len -= node->clsSttOff ;
                    }
                    dstPos = hilOffsetReplaceString(&off,dstPos,&dstJmp,node->rclose,len,tw) ;
					if(*off)
						cc = *off ;
                }
                else
                {
                    hilOffsetLenString(cc,off,dstPos,dstJmp,s1,len,tw) ;
                }
                srcPos += len ;
            }
            else if(node->type & HLCONTIN)
            {
                char *ss ;
                int   pos ;
                
                ss  = (char *) node->close ;
                len = strlen(ss) ;
                pos = srcWid ;
                while((srcText[--pos] == ' ') && (pos > 0))
                    ;
                pos -= len-1 ;
                if((pos >= srcPos) &&
                   !strncmp((char *)srcText+pos,ss,len))
                {
                    hilOffsetString(cc,off,dstPos,dstJmp,srcText+srcPos,tw) ;
                    goto hiline_exit ;
                }
            }
            if(node->type & HLENDLINE)
            {
                if((node->type & HLREPLACE) && (node->type & HLREPTOEOL))
                {
                    dstPos = hilOffsetReplaceString(&off,dstPos,&dstJmp,(meUByte *) "",
                                                    (srcWid-srcPos),tw) ;
                    goto hiline_exit ;
                }
                if(!(node->type & HLBRINEOL) || (srcPos == srcWid))
                {
                    hilOffsetString(cc,off,dstPos,dstJmp,srcText+srcPos,tw) ;
                    goto hiline_exit ;
                }
                mode |= meHIL_MODETOEOL ;
            }
            else
            {
                if(node->type & HLBRANCH)
                {
                    if((hilno = node->ignore) == 0)
                        hilno = wp->buffer->hilight ;
                    root = hilights[hilno] ;
                }
            }
            if(dstPos)
                mode &= ~(meHIL_MODESTTLN|meHIL_MODESTART) ;
        }
        else
        {
advance_char:
            if((cc=srcText[srcPos]) == 0)
                break ;
            mode &= ~(meHIL_MODEMOVE|meHIL_MODESTART) ;
            if((mode & meHIL_MODESTTLN) && !isSpace(cc))
                mode &= ~meHIL_MODESTTLN ;
            hilOffsetChar(off,dstPos,dstJmp,cc,tw) ;
            srcPos++ ;
        }
    }
hiline_exit:
    *off = '\0' ;
}



#define INDINDCURLINE 0x0100
#define INDINDONWARD  0x0200
#define INDGOTIND     0x0400

#define INDBRACKETOPEN   (0x1000)
#define INDBRACKETCLOSE  (0x1800)  
#define INDCONTINUE      (0x0800|INDGOTIND)
#define INDEXCLUSION     (0x2000)
#define INDFIXED         (INDGOTIND)
#define INDIGNORE        (0x2800)
#define INDNEXTONWARD    (INDINDONWARD|INDGOTIND)
#define INDCURONWARD     (INDINDCURLINE|INDINDONWARD|INDGOTIND)
#define INDSINGLE        (INDINDCURLINE|INDGOTIND)
#define INDFILETYPE      (0x4000)
#define INDFTNEXTONWARD  (INDINDONWARD|INDGOTIND)
#define INDFTCURONWARD   (INDINDCURLINE|INDINDONWARD|INDGOTIND)
#define INDBOTH          (0x8000)

/* Define the integer indent field. The format is:-
 * 
 *     7     6  5 4 3 2.1 0  
 * +------+----+-+-+-+-+-+-+
 * |indent|sign| | | | | | |
 * +------+----+-+-+-+-+-+-+
 * 
 * With an implied decimal point if it is an 'indent' 
 * size based definition.
 */
#define INDNUMINDSIZE     0x80          /* indentWidth based indent == 1 */
#define INDNUMNEG         0x40          /* -ve offset */
#define INDNUMOFFSETMASK  0x3f          /* Integer offset mask */
#define INDNUMOFFSETSHIFT 2             /* The implied decimal point position */
#define INDNUMMASK        0xff          /* Number mask */

/* INDBOTH - defines both the current line and next indent lindent in a single
 * word the format of the indent packs two indent offsets into a single word
 * and takes a special format defined as follows. The indent field is smaller
 * that the standard indent field using just 7 bits to squeeze the offset into
 * the field.
 * 
 * >  15     14   13 12 11 10.9 8     7      6    5 4 3 2.1 0
 * > +--+------+----+--+--+--+-+-+-----+------+----+-+-+-+-+-+
 * > | 1|indent|sign|  |  |  | | |SPARE|indent|sign| | | | | |
 * > +--+------+----+--+--+--+-+-+-----+------+----+-+-+-+-+-+
 * >     Current Line Indent            Next Line Indent
 * 
 * With an implied decimal point if it is an 'indent' size based definition.
 */

#define INDNUM7INDSIZE     0x40          /* indentWidth based indent == 1 */
#define INDNUM7NEG         0x20          /* -ve offset */
#define INDNUM7OFFSETMASK  0x1f          /* Integer offset mask */
#define INDNUM7OFFSETSHIFT 2             /* The implied decimal point position */
#define INDNUM7MASK        0x7f          /* Number mask */

/* Conversion utilities to interchange the 8-bit and 7-bit indent offest
 * formats between each other. */
#define ind8ToInd7(xx) ((((xx) & ~INDNUMOFFSETMASK) >> 1) | ((xx) & INDNUM7OFFSETMASK))
#define ind7ToInd8(xx) ((((xx) & (INDNUM7INDSIZE|INDNUM7NEG)) << 1) | ((xx) & INDNUM7OFFSETMASK))

/* Method to determine if a 8-bit indent offset will overflow when converted
 * to a 7-bit offset. Returns non-zero if an overflow will occur. */
#define ind8ToInd7Overflow(xx) ((xx) & (INDNUMOFFSETMASK & (~INDNUM7OFFSETMASK)))

static meInt
indentLookBack(meLine *lp, meUByte lindent, meUShort offset)
{
    meVideoLine vps[2] ;
    meUShort noColChng ;
    
    /*    meAssert(hilights == indents) ;
    vps[0].hilno = lindent ;
    vps[0].wind = frameCur->windowCur ;
    vps[0].line = lp ;
    vps[0].flag = 0 ;
    vps[0].bracket = NULL ;*/
    meAssert(hilights == indents);
    vps[0].wind = frameCur->windowCur;
    vps[0].line = lp;
    vps[0].flag = 0;
    vps[1].hilno = vps[0].hilno = lindent;
    vps[1].bracket = vps[0].bracket = NULL;    
    noColChng = hilightLine(vps,0) ;
    while(noColChng > 0)
    {
        if((hilBlock[noColChng].scheme & INDFILETYPE) &&
           (hilBlock[noColChng].column < offset))
            return (meInt) (hilBlock[noColChng].scheme & 0xff) ;
        noColChng-- ;
    }
    return -1 ;
}

void
hilightLookBack(meWindow *wp)
{
    meVideoLine  *vptr ;
    meBuffer *bp ;
    register meLine *lp, *blp ;
    meUByte  hilno ;
    meHilight *root, *bracket ;
    int ii, jj ;
    
    bp = wp->buffer ;
    hilno = bp->hilight ;
    root = hilights[hilno] ;
    bracket = NULL ;
    blp = wp->dotLine ;
    ii = wp->dotLineNo - wp->vertScroll ;
    while(--ii >= 0)
        blp=meLineGetPrev(blp) ;
    
    if(meHilightGetFlags(root) & HFLOOKBSCH)
    {
        meHilight **bhis ;
        bhis = hilights ;
        hilights = indents ;
        lp = blp ;
        ii = meHilightGetLookBackLines(indents[meHilightGetLookBackScheme(root)]) ;
        while((--ii >= 0) && ((lp = meLineGetPrev(lp)) != frameCur->bufferCur->baseLine))
        {
            if((jj = indentLookBack(lp,meHilightGetLookBackScheme(root),0xffff)) >= 0)
            {
                if((jj != 0) && (bhis[jj] != NULL))
                {
                    hilno = jj ;
                    root = bhis[jj] ;
                }
                break ;
            }
        }
        hilights = bhis ;
    }
    
    if(meHilightGetFlags(root) & HFLOOKBLN)
    {
        ii = 0 ;
        jj = meHilightGetLookBackLines(root) ;
        lp = blp ;
        while(ii < jj)
        {
            if((blp=meLineGetPrev(lp)) == bp->baseLine)
                break ;
            lp = blp ;
            ii++ ;
        }
        if(ii > 0)
        {
            meVideoLine vps[2] ;
            
            /* the flag is used only to set the current line, do the select hilighting
             * and flag the next line as changed if in a bracket, therefore init to 
             * 0 and forget about */
            vps[0].wind = wp ;
            vps[0].hilno = hilno ;
            vps[0].bracket = NULL ;
            vps[0].flag = 0 ;
            vps[1].hilno = hilno ;
            vps[1].bracket = NULL ;
            vps[1].flag = 0 ;
            for(;;)
            {
                vps[0].line = lp ;
                if(hilightLine(vps,meHIL_MODELOOKB) > 1)
                {
                    if((vps[0].bracket != NULL) && (vps[0].bracket == vps[1].bracket) &&
                       (vps[0].bracket->type & (HLONELNBT|HLSNGLLNBT)))
                        vps[1].bracket = NULL ;
                    if(vps[1].hilno != hilno)
                    {
                        if((vps[0].hilno == vps[1].hilno) &&
                           (meHilightBranchType & (HLONELNBT|HLSNGLLNBT)))
                        {
                            vps[1].hilno = hilno ;
                            vps[1].bracket = NULL ;
                        }
                        meHilightBranchType = 0 ;
                    }
                }
                if(--ii == 0)
                    break ;
                vps[0].hilno = vps[1].hilno ;
                vps[0].bracket = vps[1].bracket ;
                lp = meLineGetNext(lp) ;
            }
            hilno = vps[1].hilno ;
            bracket = vps[1].bracket ;
        }
    }
    vptr  = wp->video->lineArray + wp->frameRow ;  /* Video block */
    if((vptr->hilno != hilno) || (vptr->bracket != bracket))
    {
        vptr->hilno = hilno ;
        vptr->bracket = bracket ;
        vptr->flag |= VFCHNGD ;
    }
}

/* Retrieve the indent value given the indent mask and the indentWidth to be used
 * in any expansion. */
int
meIndentGetIndent(meUByte indent, meUShort bIndentWidth)
{
    int ind;
    
    /* Get the indent out */
    ind = (int)(indent & INDNUMOFFSETMASK);
    if ((indent & INDNUMINDSIZE) != 0)
        ind = (bIndentWidth * ind) / (1 << INDNUMOFFSETSHIFT);
    if ((indent & INDNUMNEG) != 0)
        ind = -ind;
    return ind;
}

static int
meGetIndent(meUByte *prompt)
{
    meUByte *p, *q, buf[32] ;
    int ipart, fpart ;                   /* Integer & flag part. */
    
    /* The indent type may be specified in a number of different ways:-
     * 
     * -4, 4, 8, ....        This is a character based indent.
         * t,+t,-t,2t,-2t, ....  indentWidth based indent. 
     * -2/3t, 3/2t, ...      tagsize vulgar fraction based indent.
     */
    if(meGetString(prompt,0,0,buf,32) <= 0)
        return meABORT ;
        
    /* Get rid of any leading white space. */
    for (q = buf; (*q == ' ') || (*q == '\t'); q++)
        ;
    /* Check for a indent offset */
    if ((p = meStrchr(q, 't')) != NULL)
    {
        fpart = INDNUMINDSIZE ;
        p[1] = '\0';                /* Get rid of any training information. */
        
        /* Test for special string cases */
        if ((meStrcmp (q, "t") == 0) || (meStrcmp (q, "+t") == 0))
            ipart = 1 << INDNUMOFFSETSHIFT;
        else if (meStrcmp (q, "-t") == 0)
            ipart = -1 << INDNUMOFFSETSHIFT;
        else
        {
            int fpart;              /* Fractional part */
            
            /* Check for a vulgar fraction */
            *p = '\0';              /* Get rid of 'i' */
            if ((p = meStrchr(q, '/')) != NULL)
                *p++ = '\0';        /* Kill off the fraction */
            ipart = meAtoi(q);      /* Get integer part */
            
            /* Get fractional part */
            if (p == NULL)
                fpart = 1;
            else if ((fpart = meAtoi(p)) == 0)
                fpart = 1;          /* Avoid divide by 0 */
            
            ipart = (ipart * (1 << INDNUMOFFSETSHIFT)) / fpart;
        }
    }
    else
    {
        fpart = 0 ;
        ipart = meAtoi(buf);
    }
    
    /* Add the offset */
    if (ipart < 0)
    {
        fpart |= INDNUMNEG;
        ipart = -ipart;
    }
    return (fpart | (ipart & INDNUMOFFSETMASK)) ;
}

int
indent(int f, int n)
{
#define noINDTYPES 13
    static meUByte  ctypesChar[meHICMODE_SIZE+2]="sbecxwamu" ;
    static meUByte  typesChar[(noINDTYPES*2)+1]="bcefinostuvwxBCEFINOSTUVWX" ;
    static meUShort typesFlag[noINDTYPES]= { 
        INDBRACKETOPEN, INDCONTINUE, INDEXCLUSION, INDFIXED, INDIGNORE, INDNEXTONWARD, 
        INDCURONWARD, INDSINGLE, INDFILETYPE, INDBOTH, INDBOTH, INDFTCURONWARD, INDFTNEXTONWARD } ;
    static meUShort typesType[noINDTYPES]= {
        0x00, 0x00, HLBRACKET, HLENDLINE|HLSTTLINE, HLENDLINE, 0x00, 0x00, 0x00, 0x00, 0x00,
        HLBRANCH, HLBRANCH, HLBRANCH } ;
    meHilight *root, *node ;
    int itype ;
    meUShort htype ;
    meUByte indno, lindno, buf[meBUF_SIZE_MAX] ;
    
    if(n == 0)
    {
        if((meGetString((meUByte *)"Ind no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
           ((indno = (meUByte) meAtoi(buf)) == 0) || 
           (meGetString((meUByte *)"Flags",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
           ((((itype = meAtoi(buf)) & HICMODE) == 0) &&
            (meGetString((meUByte *)"Lines",0,0,buf,meBUF_SIZE_MAX) <= 0)))
            return meABORT ;
        if((root = createHilight(indno,(itype & HICMODE) ? meHICMODE_SIZE:0,
                                           &noIndents,&indents)) == NULL)
            return meFALSE ;
        meIndentGetFlags(root) = itype ;
        if(itype & HICMODE)
        {
            /* Initialise to t 0 t 3/2t 0 0 -t -1 and no CommentContinue */
            root->token[0] = (meUByte) 0x84 ;
            root->token[1] = (meUByte) 0x00 ;
            root->token[2] = (meUByte) 0x84 ;
            root->token[3] = (meUByte) 0x86 ;
            root->token[4] = (meUByte) 0x00 ;
            root->token[5] = (meUByte) 0x00 ;
            root->token[6] = (meUByte) 0xc4 ;
            root->token[7] = (meUByte) 0x41 ;
        }
        else
        {
            meIndentSetLookBackLines(root,meAtoi(buf)) ;
            if(itype & HILOOKBSCH)
            {
                if((meGetString((meUByte *)"Ind no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
                   ((lindno = (meUByte) meAtoi(buf)) == 0) ||
                   (lindno >= noIndents) || (indents[lindno] == NULL))
                    return meABORT ;
                meIndentGetLookBackScheme(root) = lindno ;
            }
        }
        indents[indno] = root ;
        return meTRUE ;
    }
    if(n == 2)
    {
        if(((indno = frameCur->bufferCur->indent) != 0) &&
           (meIndentGetFlags(indents[indno]) & HILOOKBSCH))
        {
            meHilight **bhis ;
            meLine *lp ;
            int ii ;
            bhis = hilights ;
            hilights = indents ;
            lp = frameCur->windowCur->dotLine ;
            htype = frameCur->windowCur->dotOffset ;
            lindno = meHilightGetLookBackScheme(indents[indno]) ;
            ii = (int) meIndentGetLookBackLines(indents[lindno]) ;
            do {
                if((itype = indentLookBack(lp,lindno,htype)) >= 0)
                {
                    if(itype != 0)
                        indno = itype ;
                    break ;
                }
                htype = 0xffff ;
            } while((--ii >= 0) && ((lp = meLineGetPrev(lp)) != frameCur->bufferCur->baseLine)) ;
            hilights = bhis ;
        }
        meStrcpy(resultStr,meItoa(indno)) ;
        return meTRUE ;
    }
    
    n = (n < 0) ? ADDTOKEN_REMOVE:0 ;
    
    if((meGetString((meUByte *)"Ind no",0,0,buf,meBUF_SIZE_MAX) <= 0) ||
       ((indno = (meUByte) meAtoi(buf)) == 0) ||
       (indno >= noIndents) ||
       ((root  = indents[indno]) == NULL))
        return meFALSE ;
    
    if(meIndentGetFlags(root) & HICMODE)
    {
        if((itype = mlCharReply((meUByte *)"Type: ",0,ctypesChar,NULL)) == -1)
              return meFALSE ;
        itype = (int) (((meUByte *)meStrchr(ctypesChar,itype)) - ctypesChar) ;
        if(itype < meHICMODE_SIZE)
        {
            if((n = meGetIndent((meUByte *)"Indent")) < 0)
                return meFALSE ;
            root->token[itype] = (meUByte) n ;
        }
        else if(meGetString((meUByte *)"Com Cont",0,0,buf,meBUF_SIZE_MAX) <= 0)
            return meFALSE ;
        else if(buf[0] == '\0')
        {
            meNullFree(root->rtoken) ;
            root->rtoken = NULL ;
        }
        else
            root->rtoken = meStrdup(buf) ;
        return meTRUE ;
    }
    
    if(((itype = mlCharReply((meUByte *)"Type: ",0,typesChar,NULL)) == -1) ||
       (meGetString((meUByte *)"Token",0,0,buf,meBUF_SIZE_MAX) <= 0))
        return meFALSE ;

    itype = (int) (((meUByte *)meStrchr(typesChar,itype)) - typesChar) ;
    if(itype >= noINDTYPES)
    {
        itype -= noINDTYPES ;
        htype = (HLTOKEN|typesType[itype]) ;
    }
    else
        htype = typesType[itype] ;
    
    /* add the token */
    node = meHiltTokenAddSearchString(root,NULL,(meUByte *) buf,(htype|n)) ;
    if(n & ADDTOKEN_REMOVE)
    {
        if(itype == 0)
        {
            if(meGetString((meUByte *)"Close",0,0,buf,meBUF_SIZE_MAX) <= 0)
                return meFALSE ;
            meHiltTokenAddSearchString(root,NULL,(meUByte *) buf,(htype|n)) ;
        }
        return meTRUE ;
    }
    if(node == NULL)
        return meFALSE ;
    
    node->scheme = typesFlag[itype] ;
    
    if(itype == 1)
        meIndentGetFlags(root) |= HIGOTCONT ;
    else if(itype <= 2)
    {
        if(meGetString((meUByte *)"Close",0,0,buf,meBUF_SIZE_MAX) <= 0)
            return meFALSE ;
        if(itype == 0)
        {
            if((node = meHiltTokenAddSearchString(root,NULL,(meUByte *) buf,(htype|n))) == NULL)
                return meFALSE ;
            node->scheme = INDBRACKETCLOSE ;
        }
        else
        {
            if((meHiltTokenAddSearchString(root,node,(meUByte *) buf,0) == NULL) ||
               (meGetString((meUByte *)"Ignore",0,0,buf,meBUF_SIZE_MAX) <= 0))
                return meFALSE;
            node->ignore = buf[0] ;
        }
    }
    if(typesFlag[itype] & INDGOTIND)
    {
        if((n = meGetIndent((meUByte *)"Indent")) < 0)
            return meFALSE ;
        node->scheme = (node->scheme & ~INDNUMMASK) | (n & INDNUMMASK)  ;
    }
    if(typesFlag[itype] & INDBOTH)
    {
        int m;
        if((n = meGetIndent((meUByte *)"Current Indent")) < 0)
            return meFALSE ;
        if((m = meGetIndent((meUByte *)"Next Indent")) < 0)
            return meFALSE ;
        /* Check for overflow. */
        if(ind8ToInd7Overflow(n) || ind8ToInd7Overflow(m))            
            return meFALSE ;
        node->scheme = node->scheme & ((meScheme) ~((INDNUM7MASK << 8) | INDNUM7MASK)) ;
        node->scheme |= (ind8ToInd7(n) << 8) | ind8ToInd7(m);
    }
    if((typesFlag[itype] & INDFILETYPE) || (typesType[itype] & HLBRANCH))
    {
        if(meGetString((meUByte *)"Ind No",0,0,buf,meBUF_SIZE_MAX) <= 0)
            return meFALSE ;
        node->ignore = (meUByte) meAtoi(buf) ;
        if(typesFlag[itype] & INDFILETYPE)
            node->scheme |= node->ignore ;
    }
    return meTRUE ;
}


int
indentLine(int *inComment)
{
    meSchemeSet *blkp;
    meHilight **bhilights ;
    meVideoLine vps[2] ;
    meUShort noColChng, fnoz, ii ;
    meUByte *ss, indent, lindent, bdisplayTab, bdisplayNewLine, bdisplaySpace ;
    meLine *lp ;
    int lb, ind, cind, coff ;
    
    indent = frameCur->bufferCur->indent ;
    if(meIndentGetFlags(indents[indent]) & HICMODE)
        return doCindent(indents[indent],inComment) ;
    
    /* change the hilights to the indents, must backup the old value and
     * must also ensure that the hilightLine routine will use a ' ' char,
     * otherwise the show-white-chars functionality will break it */
    bhilights = hilights ;
    bdisplayTab = displayTab ;
    bdisplayNewLine = displayNewLine ;
    bdisplaySpace = displaySpace ;    
    displayTab = ' ';
    displayNewLine = ' ';
    displaySpace = ' ';    
    
    hilights = indents ;
    
    *inComment = 0 ;
    lindent = 0 ;
    if(meIndentGetFlags(indents[indent]) & HILOOKBSCH)
    {
        lindent = meIndentGetLookBackScheme(indents[indent]) ;
        lp = frameCur->windowCur->dotLine ;
        lb = meHilightGetLookBackLines(indents[lindent]) ;
        while((--lb >= 0) && ((lp = meLineGetPrev(lp)) != frameCur->bufferCur->baseLine))
        {
            if((coff = indentLookBack(lp,lindent,0xffff)) >= 0)
            {
                if((coff != 0) && (hilights[coff] != NULL))
                    indent = coff ;
                break ;
            }
        }
    }
    
    lp = frameCur->windowCur->dotLine ;
    /* the flag is used only to set the current line, do the select hilighting
     * and flag the next line as changed if in a bracket, therefore init to 
     * 0 and forget about */
    vps[0].wind = frameCur->windowCur ;
    vps[0].line = lp ;
    vps[0].hilno = indent ;
    vps[0].bracket = NULL ;
    vps[0].flag = 0 ;
    
    noColChng = hilightLine(vps,0) ;
    blkp = hilBlock + 1 ;
    /*    printf("Got %d colour changes\n",noColChng) ;*/
    /*    for(ii=0 ; ii<noColChng ; ii++)*/
    /*        printf("Colour change %d is to 0x%x at column %d\n",ii,hilCol[ii].scheme,hilCol[ii].column) ;*/
    ss = disLineBuff ;
    ss[blkp[noColChng-1].column] = '\0' ;
    while(*ss == ' ')
        ss++ ;
    cind = ss - disLineBuff ;
    if((noColChng > 1) && (blkp[0].scheme == 0x00))
        ii = 1 ;
    else
        ii = 0 ;
    fnoz = blkp[ii].scheme ;
    if((fnoz & 0xff00) == INDFIXED)
        ind = meIndentGetIndent((meUByte) fnoz, frameCur->bufferCur->indentWidth);
    else
    {
        int jj, brace=0, contFlag=0, aind, lind, nind ;
        
        aind = 0 ;
        for(;;)
        {
            if (fnoz & INDBOTH)
                aind += meIndentGetIndent((meUByte) ind7ToInd8(fnoz>>8), frameCur->bufferCur->indentWidth);
            else if((fnoz & INDINDCURLINE) && ((fnoz == blkp[0].scheme) || (cind >= blkp[0].column)))
                aind += meIndentGetIndent((meUByte) fnoz, frameCur->bufferCur->indentWidth);
            else if(fnoz != 0)
                break ;
            if(++ii >= noColChng)
                break ;
            fnoz = blkp[ii].scheme ;
        }
        jj = (int) meHilightGetLookBackLines(indents[indent]) ;
        for(ind=0 ; (--jj >=0) ; )
        {
            if((lp = meLineGetPrev(lp)) == frameCur->bufferCur->baseLine)
                break ;
            vps[0].line = lp ;
            if((lindent != 0) && (indentLookBack(lp,lindent,0xffff) >= 0))
            {
                /* theres a file type change on this line, look back to what its changed from */
                indent = frameCur->bufferCur->indent ;
                lb = meHilightGetLookBackLines(indents[lindent]) ;
                while((--lb >= 0) && ((lp = meLineGetPrev(lp)) != frameCur->bufferCur->baseLine))
                {
                    if((coff = indentLookBack(lp,lindent,0xffff)) >= 0)
                    {
                        if((coff != 0) && (hilights[coff] != NULL))
                            indent = coff ;
                        break ;
                    }
                }
                lp = vps[0].line ;
            }
            vps[0].hilno = indent ;
            vps[0].bracket = NULL ;
            noColChng = hilightLine(vps,0) ;
            blkp = hilBlock + 1 ;
            if((blkp[0].scheme & 0xff00) == INDFIXED)
                continue ;
            ss = disLineBuff ;
            ss[blkp[noColChng-1].column] = '\0' ;
            while(*ss == ' ')
                ss++ ;
            if(*ss == '\0')
                continue ;
            /*            printf("Got prev line breakdown %d colour changes ind %d\n",noColChng,ind) ;*/
            for(ii=0 ; ii<noColChng ; ii++)
            {
                fnoz = (blkp[ii].scheme & 0xff00) ;
                if(fnoz == INDNEXTONWARD)
                    ind += meIndentGetIndent((meUByte) blkp[ii].scheme, frameCur->bufferCur->indentWidth);
                else if((fnoz == INDSINGLE) && 
                        ((ii == 0) || ((ii == 1) && ((ss-disLineBuff) >= blkp[0].column))))
                    ind -= meIndentGetIndent((meUByte) blkp[ii].scheme, frameCur->bufferCur->indentWidth);
                else if((fnoz == INDCURONWARD) && (ii != 0) &&
                        ((ii != 1) || ((ss-disLineBuff) < blkp[0].column)))
                    ind += meIndentGetIndent((meUByte) blkp[ii].scheme, frameCur->bufferCur->indentWidth);
                else if(fnoz == INDBRACKETOPEN)
                {
                    if(brace >= 0)
                        blkp[brace].column = blkp[ii].column ;
                    brace++ ;
                }
                else if(fnoz == INDBRACKETCLOSE)
                    brace-- ;
                else if(fnoz == INDCONTINUE)
                {
                    contFlag |= 0x03 ;
                    if(!(contFlag & 0x04))
                        ind += meIndentGetIndent((meUByte) blkp[ii].scheme, frameCur->bufferCur->indentWidth);
                }
                else if ((fnoz & INDBOTH) == INDBOTH)
                {
                    meUShort scheme = blkp[ii].scheme;
                    
                    /* Process next line */
                    ind += meIndentGetIndent((meUByte) ind7ToInd8(scheme), frameCur->bufferCur->indentWidth);
                    /* Process current line */
                    if ((ii != 0) && ((ii != 1) || ((ss-disLineBuff) < blkp[0].column)))
                    {
                        scheme >>= 8;
                        ind += meIndentGetIndent((meUByte) ind7ToInd8(scheme), frameCur->bufferCur->indentWidth);
                    }
                }
                /*                printf("Colour change %d is to 0x%x at column %d, ind %d\n",ii,blkp[ii].scheme,blkp[ii].column,ind) ;*/
            }
            if(brace < 0)
            {
                if((contFlag == 0) && (meIndentGetFlags(indents[indent]) & HIGOTCONT))
                    contFlag = 0x07 ;
                nind = ind = 0 ;
                continue ;
            }
            if((contFlag & 0x03) == 0x02)
                ind = lind ;
            else if(brace)
                ind = blkp[brace-1].column ;
            else
            {
                if(contFlag == 7)
                    ind += nind ;
                nind = ind ;
                ind += ss - disLineBuff + aind ;
                if((contFlag == 0) && (meIndentGetFlags(indents[indent]) & HIGOTCONT))
                    contFlag = 0x07 ;
            }
            if(ind < 0)
                ind = 0 ;
            if(!brace && (contFlag & 0x01))
            {
                lind = ind ;
                ind = 0 ;
                contFlag &= ~0x01 ;
                continue ;
            }
            break ;
        }
    }
    
    /* restore the hilight setup */
    hilights = bhilights ;
    displayTab = bdisplayTab ;
    displayNewLine = bdisplayNewLine ;
    displaySpace = bdisplaySpace ;    

    /*    printf("\nIndent line to %d\n\n",ind) ;*/
    /* Always do the doto change so the tab on the left hand edge moves
     * the cursor to the first non-white char
     */
    if((coff = getccol()-cind) < 0)
        coff = 0 ;
    coff += ind ;
    /* Now change the indent if required */
    if(cind != ind)
    {
        register int rr ;
        if((rr=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
            return rr ;
        frameCur->windowCur->dotOffset = 0 ;
        ss = frameCur->windowCur->dotLine->text ;
        while((*ss == ' ') || (*ss == '\t'))
            ss++ ;
        cind = ss - frameCur->windowCur->dotLine->text ;
        ldelete(cind,6) ;
        if(meModeTest(frameCur->bufferCur->mode,MDTAB))
            rr = 0 ;
        else
        {
            rr = ind / frameCur->bufferCur->tabWidth ;
            ind -= rr * frameCur->bufferCur->tabWidth ;
            lineInsertChar(rr,'\t') ;
        }
        lineInsertChar(ind,' ') ;
#if MEOPT_UNDO
        if(meModeTest(frameCur->bufferCur->mode,MDUNDO))
        {
            frameCur->bufferCur->undoHead->doto = ind+rr ;
            meUndoAddReplaceEnd(ind+rr) ;
        }
#endif
    }
    setccol(coff) ;
    return meTRUE ;
}

#endif /* MEOPT_HILIGHT */
