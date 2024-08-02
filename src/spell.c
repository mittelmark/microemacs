/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spell.c - Spell checking routines.
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
 * Synopsis:    Spell checking routines.
 * Authors:     Steven Phillips
 * Notes:
 *      The external input/output mostly complies with ispell, there are a 
 *      few extenstions such as the disabling of hypens and auto-replacement
 *      words. The internals, including dictionaries, are completely different.
 */

#define __SPELLC                        /* Define filename */

#include <string.h>
#include "emain.h"

#if MEOPT_SPELL

#define DTACTIVE        0x01
#define DTCHNGD         0x02
#define DTCREATE        0x04

#define SP_REG_ROOT  "/history/spell"   /* Root of spell in registry */
#define SP_REGI_SAVE "autosave"         /* Registry item - autosave */

#define TBLINCSIZE 512
#define NOTBLSIZES 11

meUInt tableSizes[NOTBLSIZES+1] = {
    211,   419,   811,  1601,  3203, 4801, 6421, 
    8419, 10831, 13001, 16103, 20011
} ;

#define FRSTSPELLRULE 'A'
#define LASTSPELLRULE 'z'
#define NOSPELLRULES  LASTSPELLRULE-FRSTSPELLRULE+1
#define RULE_PREFIX   1
#define RULE_SUFFIX   2
#define RULE_MIXABLE  4

typedef struct meSpellRule {
    struct meSpellRule *next ;
    meUByte *ending ;
    meUByte *remove ;
    meUByte *append ;
    meUByte endingLen ;
    meUByte removeLen ;
    meUByte appendLen ;
    meByte  changeLen ;
    /* rule is used by find-words for continuation &
     * guessList to eliminate suffixes
     */
    meUByte rule ;
} meSpellRule ;

meUByte meRuleScore[NOSPELLRULES] ;
meUByte meRuleFlags[NOSPELLRULES+1] ;
meSpellRule  *meRuleTable[NOSPELLRULES+1] ;

typedef meUByte meDictAddr[3] ;
#define meWORD_SIZE_MAX 127
#define meGUESS_MAX     20
#define meSCORE_MAX     60

typedef struct {
    meDictAddr    next ;
    meUByte wordLen ;     
    meUByte flagLen ;     
    meUByte data[1] ;
} meDictWord ;

typedef struct {
    meDictAddr next ;
    meUByte    wordLen ;     
    meUByte    flagLen ;     
    meUByte data[meWORD_SIZE_MAX+meWORD_SIZE_MAX] ;
} meDictAddWord ;

#define meDICTWORD_SIZE  meIntFromPtr(&((meDictWord *)0)->data)

typedef struct meDictionary {
    meUByte   flags ;
    meUByte  *fname ;
    meUInt  noWords ;
    meUInt  tableSize ;
    meUInt  dSize ;
    meUInt  dUsed ;
    meDictAddr *table ;
    struct meDictionary *next ;
} meDictionary ;

typedef meUByte meWORDBUF[meWORD_SIZE_MAX] ;
static int           longestPrefixChange ;
static int           longestSuffixRemove ;
static meDictionary *dictHead ;
static meDictionary *dictIgnr ;
static meDictWord   *wordCurr ;
static int           caseFlags ;
static int           hyphenCheck ;
static int           maxScore ;

/* find-words static variables */
static meUByte *sfwCurMask ;
static meDictionary  *sfwCurDict ;
static meUInt         sfwCurIndx ;
static meDictWord    *sfwCurWord ;
static meSpellRule   *sfwPreRule ;
static meSpellRule   *sfwSufRule ;
static int            sfwFlags ;

#define SPELL_ERROR           0x80

#define SPELL_CASE_FUPPER     0x01
#define SPELL_CASE_FLOWER     0x02
#define SPELL_CASE_CUPPER     0x04
#define SPELL_CASE_CLOWER     0x08

#define toLatinLower(c)       (isUpper(toUserFont(c)) ? toLatinFont(toggleCase(toUserFont(c))):(c))
#define toggleLatinCase(c)    toLatinFont(toggleCase(toUserFont(c)))

#define meEntryGetAddr(ed)       (((ed)[0] << 16) | ((ed)[1] << 8) | (ed)[2])
#define meEntrySetAddr(ed,off)   (((ed)[0] = (meUByte) ((off)>>16)),((ed)[1] = (meUByte) ((off)>> 8)),((ed)[2] = (meUByte) (off)))
#define meWordGetErrorFlag(wd)   (wd->flagLen &  SPELL_ERROR)
#define meWordSetErrorFlag(wd)   (wd->flagLen |= SPELL_ERROR)
#define meWordGetAddr(wd)        meEntryGetAddr((wd)->next)
#define meWordSetAddr(wd,off)    meEntrySetAddr((wd)->next,off)
#define meWordGetWordLen(wd)     (wd->wordLen)
#define meWordSetWordLen(wd,len) (wd->wordLen=len)
#define meWordGetWord(wd)        (wd->data)
#define meWordGetFlagLen(wd)     (wd->flagLen & ~SPELL_ERROR)
#define meWordSetFlagLen(wd,len) (wd->flagLen = meWordGetErrorFlag(wd) | len)
#define meWordGetFlag(wd)        (wd->data+meWordGetWordLen(wd))


static meUInt
meSpellHashFunc(meUInt tableSize, meUByte *word, int len)
{
    register meUInt h=0, g ;
    register meUByte c ;
    
    while(--len >= 0)
    {
        c = *word++ ;
        h = (h << 4) + c ;
        if((g=h & 0xf0000000) != 0)
        {
            h ^= (g >> 24) ;
            h ^= g ;
        }
    }
    return (h % tableSize) ;
}


static meDictWord *
meDictionaryLookupWord(meDictionary *dict, meUByte *word, int len)
{
    meUInt  n, off ;
    meDictWord    *ent ;
    int            s, ll ;
    
    n = meSpellHashFunc(dict->tableSize,word,len) ;
    off = meEntryGetAddr(dict->table[n]) ;
    
    while(off != 0)
    {
        ent = (meDictWord *) mePtrOffset(dict->table,off) ;
        ll = meWordGetWordLen(ent) ;
        if(ll >= len)
        {
            if(ll > len)
                break ;
            s = meStrncmp(meWordGetWord(ent),word,ll) ;
            if(s > 0)
                break ;
            if(s == 0)
                return ent ;
        }
        off = meWordGetAddr(ent) ;
    }
    return NULL ;
}

static void
meDictionaryDeleteWord(meDictionary *dict, meUByte *word, int len)
{
    meDictWord  *ent ;
    if((ent = meDictionaryLookupWord(dict,word,len)) != NULL)
    {
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0' ;
        (dict->noWords)-- ;
        dict->flags |= DTCHNGD ;
    }
}

static int
meDictionaryAddWord(meDictionary *dict, meDictWord *wrd)
{
    meUInt  n, len, off ;
    meDictWord *ent, *lent, *nent ;
    meUByte  wlen, flen, olen, *word ;
    
    word = meWordGetWord(wrd) ;
    wlen = meWordGetWordLen(wrd) ;
    flen = meWordGetFlagLen(wrd) ;
    if((ent = meDictionaryLookupWord(dict,word,wlen)) != NULL)
    {
        if(flen == 0)
            return meTRUE ;
        /* see if the word list already exists in the rule list */
        if(!meWordGetErrorFlag(ent) &&
           !meWordGetErrorFlag(wrd) &&
           ((olen=meWordGetFlagLen(ent)) > 0))
        {
            meUByte *ff, *ef ;
            ef = meWordGetFlag(ent) ;
            ff = meWordGetFlag(wrd) ;
            ff[flen] = '\0' ;
            if(meStrstr(ef,ff) != NULL)
                return meTRUE ;
            ff[flen++] = '_' ;
            memcpy(ff+flen,ef,olen) ;
            flen += olen ;
        }
        /* Remove the old entry */
        meWordGetWord(ent)[0] = '\0' ;
        (dict->noWords)-- ;
    }
    dict->flags |= DTCHNGD ;
    len = meDICTWORD_SIZE + wlen + flen ;
        
    if((dict->dUsed+len) > dict->dSize)
    {
        dict->dSize = dict->dUsed+len+TBLINCSIZE ;
        if((dict->table = (meDictAddr *) meRealloc(dict->table,dict->dSize)) == NULL)
            return meFALSE ;
    }
    nent = (meDictWord *) mePtrOffset(dict->table,dict->dUsed) ;
    nent->wordLen = wrd->wordLen ;
    nent->flagLen = wrd->flagLen ;
    meWordSetFlagLen(nent,flen) ;
    memcpy(meWordGetWord(nent),word,wlen) ;
    memcpy(meWordGetFlag(nent),meWordGetFlag(wrd),flen) ;
    
    lent = NULL ;
    n = meSpellHashFunc(dict->tableSize,word,wlen) ;
    off  = meEntryGetAddr(dict->table[n]) ;
    while(off != 0)
    {
        ent = (meDictWord *) mePtrOffset(dict->table,off) ;
        if(meWordGetWordLen(ent) >= wlen)
        {
            if((meWordGetWordLen(ent) > wlen) ||
               meStrncmp(meWordGetWord(ent),word,wlen) > 0)
                break ;
        }
        off = meEntryGetAddr(ent->next) ;
        lent = ent ;
    }
    if(lent == NULL)
        meEntrySetAddr(dict->table[n],dict->dUsed) ;
    else
        meWordSetAddr(lent,dict->dUsed) ;
    meWordSetAddr(nent,off) ;
    dict->dUsed += len ;
    (dict->noWords)++ ;
    return meTRUE ;
}


static void
meDictionaryRehash(meDictionary *dict)
{
    meDictWord     *ent ;
    meDictAddr     *table, *tbl ;
    meUInt  tableSize, oldtableSize, ii, 
                   dUsed, dSize, noWords, off ;
    
    oldtableSize = dict->tableSize ;
    noWords = dict->noWords >> 2 ;
    for(ii=0 ; ii<NOTBLSIZES ; ii++)
        if(tableSizes[ii] > noWords)
            break ;
    if((tableSize = tableSizes[ii]) == oldtableSize)
        return ;
    dUsed = sizeof(meDictAddr)*tableSize ;
    dSize = dict->dUsed+dUsed ;
    
    if((table = (meDictAddr *) meMalloc(dSize)) == NULL)
        return ;
    memset(table,0,dUsed) ;
    tbl = dict->table ;
    dict->dUsed = dUsed ;
    dict->dSize = dSize ;
    dict->table = table ;
    dict->tableSize = tableSize ;
    dict->noWords = 0 ;
    for(ii=0 ; ii<oldtableSize ; ii++)
    {
        off = meEntryGetAddr(tbl[ii]) ;
        while(off != 0)
        {
            ent = (meDictWord *) mePtrOffset(tbl,off) ;
            /* Check this word has not been erased, if not then add */
            if(meWordGetWord(ent)[0] != '\0')
                meDictionaryAddWord(dict,ent) ;
            off = meWordGetAddr(ent) ;
        }
    }
    free(tbl) ;
}

            
static int
meDictionaryLoad(meDictionary *dict)
{
    FILE           *fp ;
    meDictAddr     *table, info[2] ;
    meUInt   dSize, dUsed ;
    
    if((fp = fopen((char *)dict->fname,"rb")) == NULL)
        return mlwrite(MWABORT,(meUByte *)"Failed to open dictionary [%s]",dict->fname) ;
    fseek(fp,0,SEEK_END) ;
    dUsed = ftell(fp) ;
    fseek(fp,0,SEEK_SET) ;
    if((fgetc(fp) != 0xED) || (fgetc(fp) != 0xF1))
        return mlwrite(MWABORT,(meUByte *)"[%s does not have correct id string]",dict->fname) ;
    fread(info,sizeof(meDictAddr),2,fp) ;
    dUsed -= ftell(fp) ;
    dSize = dUsed + TBLINCSIZE ;
    
#ifdef BUILD_INSURE_VERSIONS
    table = (meDictAddr *) meMalloc(10) ;
#endif
    if ((table = (meDictAddr *) meMalloc(dSize)) == NULL)
        return meFALSE ;
    dict->table = table ;
    dict->dUsed = dUsed ;
    dict->dSize = dSize ;
    dict->noWords = meEntryGetAddr(info[0]) ;
    dict->tableSize = meEntryGetAddr(info[1]) ;
    if((dSize = fread(table,1,dUsed,fp)) != dUsed)
    {
        free(table) ;
        return mlwrite(MWABORT,(meUByte *)"failed to read dictionary %s",dict->fname) ;
    }
    fclose(fp) ;
    
    dict->flags |= DTACTIVE ;
    meDictionaryRehash(dict) ;
    
    return meTRUE ;
}


static int
meSpellInitDictionaries(void)
{
    meDictionary   *dict, *ldict ;
    
    ldict = NULL ;
    dict = dictHead ;
    if(dict == NULL)
        return mlwrite(MWABORT,(meUByte *)"[No dictionaries]") ;
    while(dict != NULL)
    {
        if(!(dict->flags & DTACTIVE) &&
           (meDictionaryLoad(dict) <= 0))
        {
            if(ldict == NULL)
                dictHead = dict->next ;
            else
                ldict->next = dict->next ;
            meFree(dict->fname) ;
            meFree(dict) ;
            return meFALSE ;
        }
        ldict = dict ;
        dict = ldict->next ;
    }
    if(dictIgnr == NULL)
    {
        meDictAddr    *table ;
        meUInt  dSize, dUsed ;
        
        dUsed = sizeof(meDictAddr)*tableSizes[0] ;
        dSize = dUsed + TBLINCSIZE ;
        if(((dictIgnr = (meDictionary *) meMalloc(sizeof(meDictionary))) == NULL) ||
           ((table = (meDictAddr *) meMalloc(dSize)) == NULL))
            return meFALSE ;
        memset(table,0,dUsed) ;
        dictIgnr->dSize = dSize ;
        dictIgnr->dUsed = dUsed ;
        dictIgnr->table = table ;
        dictIgnr->noWords = 0 ;
        dictIgnr->tableSize = tableSizes[0] ;
        dictIgnr->flags = DTACTIVE ;
        dictIgnr->fname = NULL ;
        dictIgnr->next = NULL ;
    }
    return meTRUE ;
}

static meDictionary *
meDictionaryFind(int flag)
{
    meDictionary *dict ;
    meUByte fname[meBUF_SIZE_MAX], tmp[meBUF_SIZE_MAX] ;
    int found ;
    
    if(meGetString((meUByte *)"Dictionary name",MLFILECASE,0,tmp,meBUF_SIZE_MAX) <= 0)
        return meFALSE ;
    
    if(!fileLookup(tmp,(meUByte *)".edf",meFL_CHECKDOT|meFL_USESRCHPATH,fname))
    {
        meStrcpy(fname,tmp) ;
        found = 0 ;
    }
    else
        found = 1 ;
    dict = dictHead ;
    while(dict != NULL)
    {
        if(!fnamecmp(dict->fname,fname))
            return dict ;
        dict = dict->next ;
    }
    if(!(flag & 1) ||
       (!found && !(flag & 2)))
    {
        mlwrite(MWABORT,(meUByte *)"[Failed to find dictionary %s]",fname) ;
        return NULL ;
    }
    if(((dict = (meDictionary *) meMalloc(sizeof(meDictionary))) == NULL) ||
       ((dict->fname = meStrdup(fname)) == NULL))
        return NULL ;
    dict->table = NULL ;
    dict->flags = 0 ;
    dict->next = dictHead ;
    dictHead = dict ;
    if(!found || (flag & 4))
    {
        meDictAddr        *table ;
        meUInt   tableSize, dSize, dUsed ;
        tableSize = tableSizes[0] ;
        dUsed = sizeof(meDictAddr)*tableSize ;
        dSize = dUsed + TBLINCSIZE ;
        if((table = (meDictAddr *) meMalloc(dSize)) == NULL)
            return NULL ;
        memset(table,0,dUsed) ;
        dict->dSize = dSize ;
        dict->dUsed = dUsed ;
        dict->table = table ;
        dict->noWords = 0 ;
        dict->tableSize = tableSize ;
        dict->flags = (DTACTIVE | DTCREATE) ;
    }
    return dict ;
}

static void
meDictWordDump(meDictWord *ent, meUByte *buff)
{
    meUByte cc, *ss=meWordGetWord(ent), *dd=buff ;
    int len ;
    
    len = meWordGetWordLen(ent) ;
    do {
        cc = *ss++ ;
        *dd++ = toUserFont(cc) ;
    } while(--len > 0) ;
    
    if((len = meWordGetFlagLen(ent)) > 0)
    {
        if(meWordGetErrorFlag(ent))
            *dd++ = '>' ;
        else
            *dd++ = '/' ;
        ss = meWordGetFlag(ent) ;
        do {
            cc = *ss++ ;
            *dd++ = toUserFont(cc) ;
        } while(--len > 0) ;
    }
    *dd = '\0' ;
}

int
dictionaryAdd(int f, int n)
{
    meDictionary *dict ;
    
    if     (n == 0) f = 7 ;
    else if(n >  0) f = 3 ;
    else            f = 1 ;
    if((dict = meDictionaryFind(f)) == NULL)
        return meFALSE ;
    if(n < 0)
    {
        meDictAddr    *tbl ;
        meUInt  tableSize, ii, off ;
        
        if(meModeTest(frameCur->bufferCur->mode,MDVIEW))
            /* don't allow character insert if in read only */
            return (rdonly()) ;
        if((meSpellInitDictionaries() <= 0) ||
           (lineInsertString(0,(meUByte *)"Dictionary: ") <= 0) ||
           (lineInsertString(0,dict->fname) <= 0) ||
           (lineInsertNewline(0) <= 0) ||
           (lineInsertNewline(0) <= 0))
            return meABORT ;
        
        tableSize = dict->tableSize ;
        tbl = dict->table ;
        frameCur->windowCur->dotOffset = 0 ;
        for(ii=0 ; ii<tableSize ; ii++)
        {
            off = meEntryGetAddr(tbl[ii]) ;
            while(off != 0)
            {
                meDictWord    *ent ;
                meUByte  buff[meBUF_SIZE_MAX] ;
                
                ent = (meDictWord *) mePtrOffset(tbl,off) ;
                off = meWordGetAddr(ent) ;
                if(meWordGetWord(ent)[0] != '\0')
                {
                    meDictWordDump(ent,buff) ;
                    if((lineInsertString(0,buff) <= 0) ||
                       (lineInsertNewline(0) <= 0))
                        return meABORT ;
                }
            }
        }
    }
    return meTRUE ;
}


int
spellRuleAdd(int f, int n)
{
    meSpellRule  *rr ;
    if(n == 0)
    {
        meUByte *ss=NULL ;
        for(n=0 ; n<NOSPELLRULES+1 ; n++)
        {
            while((rr=meRuleTable[n]) != NULL)
            {
                meRuleTable[n] = rr->next ;
                if(ss != rr->ending)
                {
                    ss = rr->ending ;
                    free(ss) ;
                }
                meNullFree(rr->remove) ;
                meNullFree(rr->append) ;
                free(rr) ;
            }
            meRuleFlags[n] = 0 ;
        }
        hyphenCheck = 1 ;
        maxScore = meSCORE_MAX ;
        longestPrefixChange = 0 ;
        /* reset the find-words */
        if(sfwCurMask != NULL)
        {
            free(sfwCurMask) ;
            sfwCurMask = NULL ;
        }
    }
    else
    {
        meSpellRule   *pr ;
        meUByte *ss, cc, bb ;
        meUByte buff[meBUF_SIZE_MAX] ;
        int rule ;
        
        if((rule = mlCharReply((meUByte *)"Rule flag: ",0,NULL,NULL)) < 0)
            return meABORT ;
        if((f == meFALSE) && (rule == '-'))
        {
            if((rule = mlyesno((meUByte *)"Enable hyphen")) == meABORT)
                return meABORT ;
            hyphenCheck = rule ;
            return meTRUE ;
        }
        else if((f == meFALSE) && (rule == '#'))
        {
            if(meGetString((meUByte *)"Guess score",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0)
                return meABORT ;
            maxScore = meAtoi(buff) ;
            return meTRUE ;
        }
        if(rule == '*')
        {
            rule = NOSPELLRULES ;
            if(((rr = meMalloc(sizeof(meSpellRule))) == NULL) || 
               (meGetString((meUByte *)"Rule",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((rr->ending = meStrdup(buff)) == NULL))
                return meABORT ;
            rr->remove = NULL ;
            rr->append = NULL ;
            rr->removeLen = 0 ;
            rr->appendLen = 0 ;
            rr->changeLen = 0 ;
            rr->endingLen = 0 ;
        }
        else
        {
            if((rule != '*') && ((rule < FRSTSPELLRULE) || (rule > LASTSPELLRULE) || (rule == '_')))
                return mlwrite(MWABORT,(meUByte *)"[Invalid spell rule flag]") ;
            
            rule -= FRSTSPELLRULE ;
            if(((rr = meMalloc(sizeof(meSpellRule))) == NULL) || 
               (meGetString((meUByte *)"Ending",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((rr->ending = (meUByte *) meStrdup(buff)) == NULL) ||
               (meGetString((meUByte *)"Remove",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((rr->remove = (meUByte *) meStrdup(buff)) == NULL) ||
               (meGetString((meUByte *)"Append",MLNOSPACE,0,buff,meBUF_SIZE_MAX) <= 0) ||
               ((rr->append = (meUByte *) meStrdup(buff)) == NULL))
                return meABORT ;
            rr->removeLen = meStrlen(rr->remove) ;
            rr->appendLen = meStrlen(rr->append) ;
            rr->changeLen = rr->appendLen - rr->removeLen ;
            f = 0 ;
            bb = 1 ;
            ss = rr->ending ;
            while((cc=*ss++) != '\0')
            {
                if(cc == '[')
                    bb = 0 ;
                else if(cc == ']')
                    bb = 1 ;
                if(bb)
                    f++ ;
            }
            if(!bb)
                return mlwrite(MWABORT,(meUByte *)"[Spell rule - ending has no closing ']']") ;
            rr->endingLen = f ;
            if(n < 0)
            {
                meRuleFlags[rule] = RULE_PREFIX ;
                if(rr->changeLen > longestPrefixChange)
                    longestPrefixChange = rr->changeLen ;
                n = -n ;
            }
            else
                meRuleFlags[rule] = RULE_SUFFIX ;
            if(n & 2)
                meRuleFlags[rule] |= RULE_MIXABLE ;
        }
        if((pr = meRuleTable[rule]) == NULL)
        {
            rr->next = meRuleTable[rule] ;
            meRuleTable[rule] = rr ;
        }
        else
        {
            for(;; pr = pr->next)
            {
                if(!meStrcmp(rr->ending,pr->ending))
                {
                    free(rr->ending) ;
                    rr->ending = pr->ending ;
                    break ;
                }
                if(pr->next == NULL)
                    break ;
            }
            rr->next = pr->next ;
            pr->next = rr ;
        }
    }
    return meTRUE ;
}

/* Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
static int
meDictionarySave(meDictionary *dict, int n)
{
    meRegNode *reg ;
    int ii ;
    
    if(!(dict->flags & DTCHNGD))
        return meTRUE ;
    
    /* Never auto-save created dictionaries */
    if((dict->flags & DTCREATE) ||
       ((n & 0x01) && 
        (((reg=regFind(NULL,(meUByte *)SP_REG_ROOT "/" SP_REGI_SAVE))==NULL) || (regGetLong(reg,0) == 0))))
    {
        meUByte prompt[meBUF_SIZE_MAX] ;
        int  ret ;
        meStrcpy(prompt,dict->fname) ;
        meStrcat(prompt,": Save dictionary") ;
        if((ret = mlyesno(prompt)) < 0)
        {
            ctrlg(meFALSE,1) ;
            return meFALSE ;
        }
        if(ret == meFALSE)
            return meTRUE ;
        if(dict->flags & DTCREATE)
        {
            meUByte fname[meBUF_SIZE_MAX], *pp, *ss ;
            /* if the ctreated dictionary does not have a complete path then
             * either use $user-path or get one */
            if(meStrrchr(dict->fname,DIR_CHAR) != NULL)
                meStrcpy(fname,dict->fname) ;
            else if(meUserPath != NULL)
            {
                meStrcpy(fname,meUserPath) ;
                meStrcat(fname,dict->fname) ;
            }
            else
            {
                ss = dict->fname ;
                if(inputFileName((meUByte *)"Save to directory",fname,1) <= 0)
                    return meFALSE ;
                pp = fname + meStrlen(fname) ;
                if(pp[-1] != DIR_CHAR)
                    *pp++ = DIR_CHAR ;
                meStrcpy(pp,ss) ;
            }
            
            if(((ii=meStrlen(fname)) < 4) ||
#ifdef _INSENSE_CASE
               meStricmp(fname+ii-4,".edf")
#else
               meStrcmp(fname+ii-4,".edf")
#endif
               )
                meStrcpy(fname+ii,".edf") ;
            free(dict->fname) ;
            dict->fname = meStrdup(fname) ;
        }
    }
    if(ffWriteFileOpen(dict->fname,meRWFLAG_WRITE,NULL) > 0)
    {    
        meUByte header[8] ;
                  
        header[0] = 0xED ;
        header[1] = 0xF1 ;
        meEntrySetAddr(header+2,dict->noWords) ;
        meEntrySetAddr(header+5,dict->tableSize) ;
        
        if(ffWriteFileWrite(8,header,0) > 0)
            ffWriteFileWrite(dict->dUsed,(meUByte *) dict->table,0) ;
        if(ffWriteFileClose(dict->fname,meRWFLAG_WRITE,NULL) > 0)
        {
            dict->flags &= ~DTCHNGD ;
            return meTRUE ;
        }
    }
    return mlwrite(MWABORT|MWPAUSE,(meUByte *)"[Failed to write dictionary %s]",dict->fname) ;
}

/* Note the return value for this is:
 * meABORT - there was a major failure (i.e. couldn't open the file)
 * meFALSE - user quit
 * meTRUE  - succeded
 * this is used by the exit function which ignore the major failures
 */
int
dictionarySave(int f, int n)
{
    meDictionary   *dict ;
    
    if(n & 0x02)
    {
        dict = dictHead ;
        while(dict != NULL)
        {
            if((f=meDictionarySave(dict,n)) <= 0)
                return f ;
            dict = dict->next ;
        }
    }
    else
    {
        /* when saving a single disable the prompt */
        if(((dict = meDictionaryFind(0)) == NULL) ||
           ((f=meDictionarySave(dict,0)) <= 0))
            return f ;
    }

    return meTRUE ;
}

/* returns true if any dictionary needs saving */
int
anyChangedDictionary(void)
{
    meDictionary *dict ;
        
    dict = dictHead ;
    while(dict != NULL)
    {
        if(dict->flags & DTCHNGD)
            return meTRUE ;
        dict = dict->next ;
    }
    return meFALSE ;
}

static void
meDictionaryFree(meDictionary *dict)
{
    meNullFree(dict->table) ;
    meNullFree(dict->fname) ;
    free(dict) ;
}

/* dictionaryDelete
 * 
 * Argument is a bitmask where:
 * 
 * 0x01 prompt before losing changes
 * 0x02 delete ignore dictionary
 * 0x04 delete all non-ignore dictionaries
 */
int
dictionaryDelete(int f, int n)
{
    /* reset the find-words */
    if(sfwCurMask != NULL)
    {
        free(sfwCurMask) ;
        sfwCurMask = NULL ;
    }
    if(n & 0x04)
    {
        if(dictIgnr != NULL)
        {
            meDictionaryFree(dictIgnr) ;
            dictIgnr = NULL ;
        }
        if(!(n & 0x02))
            /* just remove the ignore */
            return meTRUE ;
    }
    
    if(n & 0x02)
    {
        meDictionary *dd ;
        
        while((dd=dictHead) != NULL)
        {
            if((n & 0x01) && (meDictionarySave(dd,0x01) <= 0))
                return meFALSE ;
            dictHead = dd->next ;
            meDictionaryFree(dd) ;
        }
    }
    else
    {
        meDictionary *dict, *dd ;
    
        if((dict = meDictionaryFind(0)) == NULL)
            return meFALSE ;
        
        if((n & 0x01) && (meDictionarySave(dict,0x01) <= 0))
            return meFALSE ;
        if(dict == dictHead)
            dictHead = dict->next ;
        else
        {
            dd = dictHead ;
            while(dd->next != dict)
                dd = dd->next ;
            dd->next = dict->next ;
        }
        meDictionaryFree(dict) ;
    }
    return meTRUE ;
}


static int
meSpellGetCurrentWord(meWORDBUF word)
{
    register meUByte  *dp, c, alnm, nalnm=1 ;
    register int sz=0, alphaCnt=0 ;
    
    dp = (meUByte *) word ;
    do
    {
        alnm = nalnm ;
        c = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset) ;
        frameCur->windowCur->dotOffset++ ;
        *dp++ = c ;
        if(isAlpha(c))
            alphaCnt = 1 ;
        nalnm = isAlphaNum(c) ;
    } while((++sz < meWORD_SIZE_MAX) && 
            (nalnm || (alnm && isSpllExt(c)))) ;
    
    dp[-1] = 0 ;
    frameCur->windowCur->dotOffset-- ;
    if(!alphaCnt)
        return -1 ;
    
    return sz - 1 ;
}

#define wordPrefixRuleAdd(bwd,rr)         memcpy(bwd,rr->append,rr->appendLen) ;
#define wordPrefixRuleRemove(bwd,rr)                                             \
do {                                                                             \
    if(rr->removeLen)                                                            \
        memcpy(bwd,rr->remove, rr->removeLen) ;                                  \
} while(0)

#define wordSuffixRuleGetEnd(wd,wlen,rr)  (wd+wlen-rr->removeLen)
#define wordSuffixRuleAdd(ewd,rr)         strcpy((char *) ewd,(char *) rr->append)
#define wordSuffixRuleRemove(ewd,rr)      strcpy((char *) ewd,(char *) rr->remove)

static meDictWord *
wordTrySuffixRules(meUByte *word, int wlen, meUByte prefixRule)
{
    meDictionary  *dict ;
    meSpellRule   *rr ;
    meDictWord    *wd ;
    meUByte  flags ;
    int ii ;
    
    flags = (prefixRule == 0) ? RULE_SUFFIX:(RULE_SUFFIX|RULE_MIXABLE) ;
    
    /* Try all the suffix rules to see if we can derive the word from another */
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if((meRuleFlags[ii] & flags) == flags)
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                meUByte cc ;
                int alen, nlen, clen, elen ;
                clen = rr->changeLen ;
                elen = rr->endingLen ;
                if(wlen >= elen+clen)
                {
                    nlen = wlen - clen ;
                    alen = meStrlen(rr->append) ;
                    if(!meStrncmp(rr->append,word+wlen-alen,alen))
                    {
                        cc = word[nlen] ;
                        wordSuffixRuleRemove(word+wlen-alen,rr) ;
                        if((elen == 0) || (regexStrCmp(word+nlen-elen,rr->ending,meRSTRCMP_WHOLE) > 0))
                        {
                            dict = dictHead ;
                            while(dict != NULL)
                            {
                                if((wd=meDictionaryLookupWord(dict,word,nlen)) != NULL)
                                {
                                    /* check that the word found allows this rule */
                                    meUByte *flag, *sflag, ff ;
                                    int len, slen ;
                                    
                                    sflag = flag = meWordGetFlag(wd) ;
                                    slen = len = meWordGetFlagLen(wd) ;
                                    while(--len >= 0)
                                    {
                                        if((ff=*flag++) == '_')
                                        {
                                            sflag = flag ;
                                            slen = len ;
                                        }
                                        else if(ff == ii+FRSTSPELLRULE)
                                        {
                                            if(prefixRule == 0)
                                            {
                                                /* no prefix - found it, return */
                                                word[nlen] = cc ;
                                                return wd ;
                                            }
                                            /* check for the prefix rule as well if given */
                                            while(--slen >= 0)
                                            {
                                                if((ff=*sflag++) == '_')
                                                    break ;
                                                if(ff == prefixRule)
                                                {
                                                    /* found it, return */
                                                    word[nlen] = cc ;
                                                    return wd ;
                                                }
                                            }
                                        }
                                    }
                                }
                                dict = dict->next ;
                            }
                        }
                        word[nlen] = cc ;
                        memcpy(word+wlen-alen,rr->append,alen) ;
                    }
                }
                rr = rr->next ;
            }
        }
    }
    return NULL ;
}

static meDictWord *
wordTryPrefixRules(meUByte *word, int wlen)
{
    meDictionary *dict ;
    meSpellRule  *rr ;
    meDictWord   *wd ;
    int ii ;
    
    /* Now try all the prefix rules to see if we can derive the word */
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if(meRuleFlags[ii] & RULE_PREFIX)
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                meUByte *nw, cc ;
                int nlen, elen ;
                
                if((wlen >= rr->appendLen) &&
                   !meStrncmp(rr->append,word,rr->appendLen))
                {
                    nlen = wlen - rr->changeLen ;
                    nw = word + rr->changeLen ;
                    wordPrefixRuleRemove(nw,rr) ;
                    if((elen=rr->endingLen) > 0)
                    {
                        cc = nw[elen] ;
                        nw[elen] = '\0' ;
                        if(regexStrCmp(nw,rr->ending,meRSTRCMP_WHOLE) <= 0)
                        {
                            nw[elen] = cc ;
                            wordPrefixRuleAdd(word,rr) ;
                            rr = rr->next ;
                            continue ;
                        }
                        nw[elen] = cc ;
                    }
                    /* Try the word on its own first */
                    dict = dictHead ;
                    while(dict != NULL)
                    {
                        if((wd=meDictionaryLookupWord(dict,nw,nlen)) != NULL)
                        {
                            /* check that the word found allows this rule */
                            meUByte *flag ;
                            int len ;
                            
                            flag = meWordGetFlag(wd) ;
                            len = meWordGetFlagLen(wd) ;
                            while(--len >= 0)
                                if(*flag++ == ii+FRSTSPELLRULE)
                                    /* yes, lets get out of here */
                                    return wd ;
                        }
                        dict = dict->next ;
                    }
                    if(meRuleFlags[ii] & RULE_MIXABLE)
                    {
                        /* Now try all the suffixes for this prefix */
                        if((wd = wordTrySuffixRules(nw,nlen,(meUByte) (ii+FRSTSPELLRULE))) != NULL)
                            /* yes, lets get out of here */
                            return wd ;
                    }
                    wordPrefixRuleAdd(word,rr) ;
                }
                rr = rr->next ;
            }
        }
    }
    return NULL ;
}

static int
wordTrySpecialRules(meUByte *word, int wlen)
{
    meUByte cc ;
    meSpellRule  *rr ;
    
    if((rr = meRuleTable[NOSPELLRULES]) != NULL)
    {
        /* Now try all the special rules - e.g. [0-9]*1st for 1st, 21st etc */
        cc = word[wlen] ;
        word[wlen] = '\0' ;
        while(rr != NULL)
        {
            if(regexStrCmp(word,rr->ending,meRSTRCMP_WHOLE) > 0)
            {
                word[wlen] = cc ;
                return meTRUE ;
            }
            rr = rr->next ;
        }
        word[wlen] = cc ;
    }
    return meFALSE ;
}

/* wordCheckSearch
 * 
 * Looks in all dictionaries for the given word, then tries all
 * the rules. Returns:
 *   meFALSE  Word not found
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 */
static int
wordCheckSearch(meUByte *word, int wlen)
{
    meDictionary *dict ;
    meDictWord   *wd ;
    
    if((wd=meDictionaryLookupWord(dictIgnr,word,wlen)) == NULL)
    {
        dict = dictHead ;
        while(dict != NULL)
        {
            if((wd=meDictionaryLookupWord(dict,word,wlen)) != NULL)
                break ;
            dict = dict->next ;
        }
        if(wd == NULL)
        {
            /* Now try all the suffixes on the original word */
            wd = wordTrySuffixRules(word,wlen,0) ;
        
            if(wd == NULL)
            {
                /* Now try all the prefixes on the original word */
                wd = wordTryPrefixRules(word,wlen) ;
                if(wd == NULL)
                    return wordTrySpecialRules(word,wlen) ;
            }
        }
    }
    wordCurr = wd ;
    if(meWordGetErrorFlag(wd))
        return meABORT ;
    return meTRUE ;
}

/* wordCheckBase
 * 
 * Checks
 * 1) The word is a word
 * 2) Tries wordCheckSearch
 * 3) If upper case then capitalises and tries wordCheckSearch
 * 4) If capitalised the makes lower case and tries wordCheckSearch
 * 
 * If all the above fail then it fails. Returns:
 *   meFALSE  Word is okay
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 */
static int
wordCheckBase(meUByte *word, int wlen)
{
    meUByte *ss, cc ;
    int ii ;
    
    if(caseFlags == 0)
        /* No letters in the word -> not a word -> word okay */
        return meTRUE ;
    
    if((ii=wordCheckSearch(word,wlen)) != meFALSE)
        return ii ;
    
    /* If all upper then capitalise */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen ;
        ss = word+1 ;
        while(--ii > 0)
        {
            cc = *ss ;
            *ss++ = toggleLatinCase(cc) ;
        }
        if(wordCheckSearch(word,wlen) > 0)
            return meTRUE ;
    }
    if(caseFlags & SPELL_CASE_FUPPER)
    {
        cc = *word ;
        *word = toggleLatinCase(cc) ;
        if(wordCheckSearch(word,wlen) > 0)
            return meTRUE ;
        *word = cc ;
    }
    /* We failed to find it, restore the case if we changed it */
    if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        ii = wlen ;
        ss = word+1 ;
        while(--ii > 0)
        {
            cc = *ss ;
            *ss++ = toggleLatinCase(cc) ;
        }
    }
    return meFALSE ;
}

/* wordCheck
 * 
 * Check for the given word in all dictionaries, returns:
 *   meFALSE  Word not found
 *   meTRUE   found and word is good
 *   meABORT  found and word is erroneous
 */
static int
wordCheck(meUByte *word)
{
    meUByte *ss ;
    int ii, len, hyphen ;
    
    len = meStrlen(word) ;
    
    if((ii=wordCheckBase(word,len)) != meFALSE)
        return ii ;
    if(isSpllExt(word[len-1]))
    {
        len-- ;
        if(wordCheckBase(word,len) > 0)
            return meTRUE ;
    }
    if(hyphenCheck)
    {
        /* check for hyphenated words such as wise-cracks */
        for(ss=word,hyphen=0 ; len ; len--,ss++)
        {
            if(*ss == '-')
            {
                if((ss != word) &&
                   (wordCheckBase(word,(meUByte)(ss-word)) <= 0))
                    return meFALSE ;
                word = ss+1 ;
                hyphen = 1 ;
            }
        }
        if(hyphen && 
           (((len=ss-word) == 0) || (wordCheckBase(word,(meUByte)len) > 0)))
        {
            wordCurr = NULL ;
            return meTRUE ;
        }
    }
    return meFALSE ;
}    

static meUByte *
wordApplyPrefixRule(meUByte *wd, meSpellRule *rr)
{
    meUByte *nw, cc ;
    int len ;
    
    if((len=rr->endingLen) != 0)
    {
        cc = wd[len] ;
        wd[len] = '\0' ;
        if(regexStrCmp(wd,rr->ending,meRSTRCMP_WHOLE) <= 0)
        {
            wd[len] = cc ;
            return NULL ;
        }
        wd[len] = cc ;
    }
    nw = wd - rr->changeLen ;
    wordPrefixRuleAdd(nw,rr) ;
    return nw ;
}

static meUByte *
wordApplySuffixRule(meUByte *wd, int wlen, meSpellRule *rr)
{
    meUByte *ew, len ;
    
    ew = wd+wlen ;
    if((len=rr->endingLen) != 0)
    {
        if((len > wlen) || (regexStrCmp(ew-len,rr->ending,meRSTRCMP_WHOLE) <= 0))
            return NULL ;
    }
    ew -= rr->removeLen ;
    wordSuffixRuleAdd(ew,rr) ;
    return ew ;
}

static int
wordTestSuffixRule(meUByte *wd, int wlen, meSpellRule *rr)
{
    meUByte len ;
    
    if(((len=rr->endingLen) != 0) &&
       ((len > wlen) || (regexStrCmp(wd+wlen-len,rr->ending,meRSTRCMP_WHOLE) <= 0)))
       return 0 ;
    return 1 ;
}


#if 0
#define __LOG_FILE
#endif

#ifdef __LOG_FILE
FILE *fp ;
#endif

/* wordGuessCalcScore
 * 
 * Given the user source word (swd) and a dictionary word (dwd) the 
 */
static int
wordGuessCalcScore(meUByte *swd, int slen, meUByte *dwd, int dlen, int testFlag)
{
    int  scr=0 ;
    meUByte cc, lcc='\0', dd, ldd='\0' ;
    
#ifdef __LOG_FILE
    {
        cc = swd[slen] ;
        swd[slen] = '\0' ;
        dd = dwd[dlen] ;
        dwd[dlen] = '\0' ;
        fprintf(fp,"WordScore [%s] with [%s] ",swd,dwd) ;
        swd[slen] = cc ;
        dwd[dlen] = dd ;
    }
#endif
    for(;;)
    {
        if(slen == 0)
        {
            while(--dlen >= 0)
            {
                dd = *dwd++ ;
                if(!isSpllExt(dd))
                    /* ignore missed ' s etc */
                    scr += 22 ;
            }
            break ;
        }
        if(dlen == 0)
        {
            while(--slen >= 0)
            {
                cc = *swd++ ;
                if(!isSpllExt(cc))
                    /* ignore missed ' s etc */
                    scr += 25 ;
            }
            break ;
        }
        cc = *swd ;
        dd = *dwd ;
        dd = toLatinLower(dd) ;
        if(dd == cc) 
        {
            swd++ ;
            slen-- ;
            lcc = cc ;
            dwd++ ;
            dlen-- ;
            ldd = dd ;
            continue ;
        }
        if((dlen > 1) && (dwd[1] == cc))
        {
            if((slen > 1) && (swd[1] == dd) && 
               ((dlen <= slen) || (dwd[2] != dd)))
            {
                /* transposed chars */
                scr += 23 ;
                swd += 2 ;
                slen -= 2 ;
                lcc = dd ;
            }
            else
            {
                if(ldd == dd)
                    /* missing double letter i.e. leter -> letter */
                    scr += 21 ;
                else if(!isSpllExt(dd))
                    /* missed letter i.e. ltter -> letter, ignore missed ' s etc */
                    scr += 22 ;
                swd++ ;
                lcc = cc ;
                if(testFlag && (slen > 1))
                    /* if this is only a test on part of the word the adjust the
                     * other words length as well to keep the end point aligned
                     * this is because of base word tests. when testing the base
                     * of anomaly with anommoly the comparison is between [anommol]
                     * with [anomaly] the realignment caused by the double m means
                     * that it should be between [anommol] and [anomal]
                     */
                    slen -= 2 ;
                else
                    slen-- ;
            }
            dwd += 2 ;
            dlen -= 2 ;
            ldd = cc ;
        }
        else if((slen > 1) && (swd[1] == dd))
        {
            if(lcc == cc)
                /* extra double letter i.e. vowwel -> vowel */
                scr += 24 ;
            else if(!isSpllExt(cc))
                /* extra letter i.e. lertter -> letter, ignore an extra ' etc. */
                scr += 25 ;
            swd += 2 ;
            slen -= 2 ;
            lcc = dd ;
            dwd++ ;
            ldd = dd ;
            if(testFlag && (dlen > 1))
                /* if this is only a test on part of the word the adjust the
                 * other words length as well to keep the end point aligned
                 * this is because of base word tests. when testing the base
                 * of anomaly with anommoly the comparison is between [anommol]
                 * with [anomaly] the realignment caused by the double m means
                 * that it should be between [anommol] and [anomal]
                 */
                dlen -= 2 ;
            else
                dlen-- ;
        }
        else
        {
            scr += 27 ;
            swd++ ;
            slen-- ;
            lcc = cc ;
            dwd++ ;
            dlen-- ;
            ldd = dd ;
        }
        if(scr >= maxScore)
            break ;
    }
#ifdef __LOG_FILE
    fprintf(fp,"scr = %d, len %d\n",scr,dlen-slen) ;
#endif
    return (scr >= maxScore) ? maxScore:scr ;
}

static void
wordGuessAddToList(meUByte *word, int curScr,
                   meWORDBUF *words, int *scores)
{
    int ii, jj ;
    
    for(ii=0 ; ii<meGUESS_MAX ; ii++)
    {
        if(curScr < scores[ii])
        {
            for(jj=meGUESS_MAX-2 ; jj>=ii ; jj--)
            {
                scores[jj+1] = scores[jj] ;
                meStrcpy(words[jj+1],words[jj]) ;
            }
            scores[ii] = curScr ;
            meStrcpy(words[ii],word) ;
            break ;
        }
        /* check for duplicate */
        if((curScr == scores[ii]) && !meStrcmp(words[ii],word))
            break ;
    }
}

static int
wordGuessAddSuffixRulesToList(meUByte *sword, int slen,
                           meUByte *bword, int blen,
                           meUByte *flags, int noflags,
                           meWORDBUF *words, int *scores, 
                           int bscore, int pflags)
{
    meSpellRule   *rr ;
    meUByte *nwd, *lend=NULL ;
    int ii ;
    
    /* try all the allowed suffixes */
    while(--noflags >= 0)
    {
        ii = flags[noflags] - FRSTSPELLRULE ;
        if(((bscore+meRuleScore[ii]) < scores[meGUESS_MAX-1]) &&
           ((meRuleFlags[ii] & pflags) == pflags))
        {
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                if(bscore+rr->rule < scores[meGUESS_MAX-1])
                {
                    int curScr, nlen ;
                    if(lend != rr->ending)
                    {
                        lend = rr->ending ;
                        if(!wordTestSuffixRule(bword,blen,rr))
                        {
                            do {
                                rr = rr->next ;
                            } while((rr != NULL) && (rr->ending == lend)) ;
                            continue ;
                        }
                    }
                    nwd = wordSuffixRuleGetEnd(bword,blen,rr) ;
                    wordSuffixRuleAdd(nwd,rr) ;
                    nlen = blen+rr->changeLen ;
                    if((curScr = wordGuessCalcScore(sword,slen,bword,nlen,0)) < maxScore)
                        wordGuessAddToList(bword,curScr,words,scores) ;
                    wordSuffixRuleRemove(nwd,rr) ;
                    if(TTbreakTest(0))
                        return 1 ;
                }
                rr = rr->next ;
            }
        }
    }
    return 0 ;
}


static int
wordGuessScoreSuffixRules(meUByte *word, int olen)
{
    meSpellRule *rr ;
    int ii, jj, scr, rscr, bscr=maxScore ;
    
    longestSuffixRemove = 0 ;
    if(isSpllExt(word[olen-1]))
        /* if the last letter is a '.' then ignore it, i.e. "anommolies."
         * should compare suffixes with the "ies", not "es."
         */
        olen-- ;
    for(ii=0 ; ii<NOSPELLRULES ; ii++)
    {
        if(meRuleFlags[ii] & RULE_SUFFIX)
        {
            rscr = maxScore ;
            rr = meRuleTable[ii] ;
            while(rr != NULL)
            {
                jj = rr->appendLen ;
                if((jj < olen) &&
                   ((scr = wordGuessCalcScore(word+olen-jj,jj,
                                             rr->append,jj,1)) < maxScore))
                {
                    rr->rule = scr ;
                    if(scr < rscr)
                        rscr = scr ;
                    if(rr->removeLen > longestSuffixRemove)
                        longestSuffixRemove = rr->removeLen ;
                }
                else
                    rr->rule = maxScore ;
                rr = rr->next ;
            }
            meRuleScore[ii] = rscr ;
            if(rscr < bscr)
                bscr = rscr ;
        }
    }
    return bscr ;
}

static int
wordGuessGetList(meUByte *word, int olen, meWORDBUF *words)
{
    meDictionary *dict ;
    meDictWord *ent ;
    meWORDBUF wbuff ;
    meUByte *wb, *ww ;
    meUInt ii, off, noWrds ;
    int scores[meGUESS_MAX], curScr, bsufScr ;
    int wlen, mlen ;
    
#ifdef __LOG_FILE
    fp = fopen("log","w+") ;
#endif
    for(noWrds=0 ; noWrds<meGUESS_MAX ; noWrds++)
    {
        scores[noWrds] = maxScore ;
        words[noWrds][0] = '\0' ;
    }
    mlen = (olen < 3) ? olen:3 ;
    /* check for two words with a missing ' ' or '-' */
    if(olen > 1)
    {
        meUByte cc ;
        
        ww = wbuff ;
        for(wlen=1 ; wlen < olen ; wlen++)
        {
            cc = resultStr[wlen] ;
            memcpy(wbuff,resultStr+1,wlen) ;
            wbuff[wlen] = '\0' ;
            if(((wlen == 1) && isSpllExt(cc)) ||
               (wordCheck(wbuff) > 0))
            {
                meStrcpy(wbuff,resultStr+wlen+1) ;
                if(wordCheck(wbuff) > 0)
                {
                    memcpy(wbuff,resultStr+1,wlen) ;
                    meStrcpy(wbuff+wlen+1,resultStr+wlen+1) ;
                    wbuff[wlen] = ' ' ;
                    wordGuessAddToList(wbuff,22,words,scores) ;
                    if(hyphenCheck && !isSpllExt(cc))
                    {
                        wbuff[wlen] = '-' ;
                        wordGuessAddToList(wbuff,22,words,scores) ;
                    }
                    if(scores[meGUESS_MAX-1] <= 22)
                        break ;
                }
            }
        }
    }
    wb = wbuff+longestPrefixChange ;
    if(caseFlags & (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
    {
        meUByte cc ;
        ww = word ;
        while((cc = *ww) != '\0')
            *ww++ = toLatinLower(cc) ;
    }
    bsufScr = wordGuessScoreSuffixRules(word,olen) ;
    dict = dictHead ;
    while(dict != NULL)
    {
        for(ii=0 ; ii<dict->tableSize ; ii++)
        {
            off = meEntryGetAddr(dict->table[ii]) ;
            while(off != 0)
            {
                ent = (meDictWord *) mePtrOffset(dict->table,off) ;
                /* Check this word has not been erased or is an error word,
                 * if not then do guess */
                if(!meWordGetErrorFlag(ent) && ((ww=meWordGetWord(ent))[0] != '\0'))
                {
                    int noflags ;
                    
                    wlen = meWordGetWordLen(ent) ;
                    if((noflags=meWordGetFlagLen(ent)) > 0)
                    {
                        meUByte *flags ;
                        meSpellRule *rr ;
                        int jj, ff, nlen ;
                        
                        memcpy(wb,ww,wlen) ;
                        wb[wlen] = '\0' ;
                        flags = meWordGetFlag(ent) ;
                        if((jj = wlen - longestSuffixRemove) < mlen)
                            jj = (wlen < mlen) ? wlen:mlen ;
                        else if(jj > olen)
                            jj = olen ;
                        curScr = wordGuessCalcScore(word,jj,wb,jj,1) ;
                        if(curScr < scores[meGUESS_MAX-1])
                        {
                            /* try all suffixes on the word, wordGuessAddSuffixRulesToList returns
                             * true if the user aborted */
                            if(((curScr+bsufScr) < scores[meGUESS_MAX-1]) &&
                               wordGuessAddSuffixRulesToList(word,olen,wb,wlen,flags,noflags,words,
                                                          scores,curScr,RULE_SUFFIX))
                                return -1 ;
                            if((curScr = wordGuessCalcScore(word,olen,wb,wlen,0)) < scores[meGUESS_MAX-1])
                                wordGuessAddToList(wb,curScr,words,scores) ;
                            if(TTbreakTest(0))
                                return -1 ;
                        }
                        /* try all the allowed prefixes */
                        ff = noflags ;
                        while(--ff >= 0)
                        {
                            if((jj = flags[ff]) == '_')
                                noflags = ff ;
                            else if(meRuleFlags[(jj-=FRSTSPELLRULE)] & RULE_PREFIX)
                            {
                                rr = meRuleTable[jj] ;
                                while(rr != NULL)
                                {
                                    if((ww = wordApplyPrefixRule(wb,rr)) != NULL)
                                    {
                                        nlen = wlen+rr->changeLen ;
                                        if((jj = nlen - longestSuffixRemove) < mlen)
                                            jj = (nlen < mlen) ? nlen:mlen ;
                                        else if(jj > olen)
                                            jj = olen ;
                                        curScr = wordGuessCalcScore(word,jj,ww,jj,1) ;
                                        if(curScr < scores[meGUESS_MAX-1])
                                        {
                                            /* try all suffixes on the word */ 
                                            if(((curScr+bsufScr) <  scores[meGUESS_MAX-1]) && (meRuleFlags[jj] & RULE_MIXABLE))
                                            {
                                                int f1, f2 ;
                                                for(f1=f2=0 ; f2<ff ; )
                                                    if(flags[f2++] == '_')
                                                        f1 = f2 ;
                                                if(wordGuessAddSuffixRulesToList(word,olen,ww,nlen,flags+f1,noflags-f1,
                                                                              words,scores,curScr,RULE_SUFFIX|RULE_MIXABLE))
                                                    return -1 ;
                                            }
                                            if((curScr = wordGuessCalcScore(word,olen,ww,nlen,0)) < scores[meGUESS_MAX-1])
                                                wordGuessAddToList(ww,curScr,words,scores) ;
                                            if(TTbreakTest(0))
                                                return -1 ;
                                        }
                                        wordPrefixRuleRemove(wb,rr) ;
                                    }
                                    rr = rr->next ;
                                }
                            }
                        }
                    }
                    else if((curScr = wordGuessCalcScore(word,olen,ww,wlen,0)) < scores[meGUESS_MAX-1])
                    {
                        memcpy(wb,ww,wlen) ;
                        wb[wlen] = '\0' ;
                        wordGuessAddToList(wb,curScr,words,scores) ;
                    }
                }
                off = meWordGetAddr(ent) ;
            }
        }
        dict = dict->next ;
    }
    for(noWrds=0 ; (noWrds<meGUESS_MAX) && (scores[noWrds] < maxScore) ; noWrds++)
        ;
    {
        meUByte cc ;
        cc = word[olen-1] ;
        if(isSpllExt(cc))
        {
            /* if the last letter is a '.' etc then loop through the guesses adding one */
            for(ii=0 ; ii<noWrds ; ii++)
            {
                wlen = meStrlen(words[ii]) ;
                /* must check that its not there already */
                if(words[ii][wlen-1] != cc)
                {
                    words[ii][wlen]   = cc ;
                    words[ii][wlen+1] = '\0' ;
                }
            }
        }
    }
    return (int) noWrds ;
}


#define SPELLWORD_GET     0x01       /* Get the word from the user */
#define SPELLWORD_GETNEXT 0x02       /* Keep going till we come to a problem */
#define SPELLWORD_ADD     0x04       /* Add the given word */
#define SPELLWORD_IGNORE  0x08       /* Add word to the ignore dictionary */
#define SPELLWORD_ERROR   0x10       /* The given word is an erroneous word */
#define SPELLWORD_GUESS   0x20       /* Return the words guest list */
#define SPELLWORD_DERIV   0x40       /* Return the words derivatives */
#define SPELLWORD_DOUBLE  0x80       /* Check for double word */
#define SPELLWORD_INFO    0x100      /* Get info on word */
#define SPELLWORD_DELETE  0x200      /* Delete the given word */

static void
spellWordToLatinFont(meUByte *dd, meUByte *ss)
{
    meUByte cc ;
    cc = *ss++ ;
    if(isUpper(cc))
        caseFlags = SPELL_CASE_FUPPER ;
    else if(isLower(cc))
        caseFlags = SPELL_CASE_FLOWER ;
    else
        caseFlags = 0 ;
    *dd++ = toLatinFont(cc) ;
    while((cc=*ss++) != '\0')
    {
        if(isUpper(cc))
            caseFlags |= SPELL_CASE_CUPPER ;
        else if(isLower(cc))
            caseFlags |= SPELL_CASE_CLOWER ;
        *dd++ = toLatinFont(cc) ;
    }
    *dd = '\0' ;
}


static void
spellWordToUserFont(meUByte *dd, meUByte *ss)
{
    meUByte cc ;
    
    cc = *ss++ ;
    cc = toUserFont(cc) ;
    if((caseFlags & SPELL_CASE_FUPPER) || (caseFlags == (SPELL_CASE_FLOWER|SPELL_CASE_CUPPER)))
        cc = toUpper(cc) ;
    *dd++ = cc ;
    while((cc=*ss++) != '\0')
    {
        cc = toUserFont(cc) ;
        if(caseFlags == (SPELL_CASE_FUPPER|SPELL_CASE_CUPPER))
            cc = toUpper(cc) ;
        *dd++ = cc ;
    }
    *dd = '\0' ;
}

int
spellWord(int f, int n)
{
    meWORDBUF word ;
    int       len ;
    
    if(meSpellInitDictionaries() <= 0)
        return meABORT ;
    
    if(n & SPELLWORD_GET)
    {
        if(meGetString((meUByte *)"Enter word", MLNOSPACE,0,resultStr+1,meWORD_SIZE_MAX) <= 0)
            return meABORT ;
        spellWordToLatinFont(word,resultStr+1) ;
    }
    else if(n & SPELLWORD_GETNEXT)
    {
        meUByte  cc, chkDbl, curDbl ;
        meUShort soff, eoff ;
        meWORDBUF lword ;
        
        chkDbl = (n & SPELLWORD_DOUBLE) ;
        curDbl = 0 ;
        while((frameCur->windowCur->dotOffset > 0) && isAlphaNum(meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)))
            frameCur->windowCur->dotOffset-- ;
        for(;;)
        {
            while(((cc = meLineGetChar(frameCur->windowCur->dotLine,frameCur->windowCur->dotOffset)) != '.') && 
                  !isAlphaNum(cc))
            {
                if(!isSpace(cc))
                    curDbl = 0 ;
                if(meWindowForwardChar(frameCur->windowCur, 1) == meFALSE)
                {
                    resultStr[0] = 'F' ;
                    resultStr[1] = '\0' ;
                    return meTRUE ;
                }
            }
            soff = frameCur->windowCur->dotOffset ;
            if(meSpellGetCurrentWord((meUByte *) resultStr+1) < 0)
                continue ;
            eoff = frameCur->windowCur->dotOffset ;
            if(curDbl && !meStricmp(lword,resultStr+1))
            {
                resultStr[0] = 'D' ;
                setShowRegion(frameCur->bufferCur,frameCur->windowCur->dotLineNo,soff,frameCur->windowCur->dotLineNo,eoff) ;
                frameCur->windowCur->updateFlags |= WFMOVEL|WFSELHIL ;
                return meTRUE ;
            }
            spellWordToLatinFont(word,(meUByte *) resultStr+1) ;
            if(wordCheck(word) <= 0)
                break ;
            if(chkDbl)
            {
                /* store the last word for double word check */
                meStrcpy(lword,resultStr+1) ;
                curDbl = 1 ;
            }
        }
        setShowRegion(frameCur->bufferCur,frameCur->windowCur->dotLineNo,soff,frameCur->windowCur->dotLineNo,eoff) ;
        frameCur->windowCur->updateFlags |= WFMOVEL|WFSELHIL ;
    }
    else
    {
        meUByte  cc ;
        meUShort off, soff, eoff ;
        
        soff = off = frameCur->windowCur->dotOffset ;
        for(;;soff--)
        {
            cc = meLineGetChar(frameCur->windowCur->dotLine,soff) ;
            if((soff==0) || isSpllWord(cc))
                    break ;
        }
        if(!isSpllWord(cc))
            return meFALSE ;
        while(soff > 0)
        {
            --soff ;
            cc = meLineGetChar(frameCur->windowCur->dotLine,soff) ;
            if(!isSpllWord(cc))
            {
                soff++ ;
                break ;
            }
            if(!isAlphaNum(cc) && 
               ((soff == 0) || !isAlphaNum(meLineGetChar(frameCur->windowCur->dotLine,soff-1))))
                break ;
        }
        /* if the first character is not alphanumeric or a '.' then move
         * on one, this stops misspellings of 'quoted words'
         */
        if(((cc = meLineGetChar(frameCur->windowCur->dotLine,soff)) != '.') && !isAlphaNum(cc))
            soff++ ;
        frameCur->windowCur->dotOffset = soff ;
        len = meSpellGetCurrentWord((meUByte *) resultStr+1) ;
        eoff = frameCur->windowCur->dotOffset ;
        frameCur->windowCur->dotOffset = off ;
        setShowRegion(frameCur->bufferCur,frameCur->windowCur->dotLineNo,soff,frameCur->windowCur->dotLineNo,eoff) ;
        frameCur->windowCur->updateFlags |= WFMOVEL|WFSELHIL ;
        if(len < 0)
        {
            resultStr[0] = 'N' ;
            return meTRUE ;
        }
        spellWordToLatinFont(word,(meUByte *) resultStr+1) ;
    }
    len = meStrlen(word) ;
    if(n & SPELLWORD_ADD)
    {
        meDictAddWord  wdbuff ;
        meDictionary  *dict ;
        meDictWord    *wd= (meDictWord *) &wdbuff ;
        
        memcpy(meWordGetWord(wd),word,len) ;
        meWordSetWordLen(wd,len) ;
        
        if(meGetString((meUByte *)"Enter flags", MLNOSPACE,0,word,meWORD_SIZE_MAX) <= 0)
            return meABORT ;
        len = meStrlen(word) ;
        memcpy(meWordGetFlag(wd),word,len) ;
        /* set flag len directly to clear the ERROR flag */
        wd->flagLen = len ;
        if(n & SPELLWORD_ERROR)
            meWordSetErrorFlag(wd) ;
        if(n & SPELLWORD_IGNORE)
            dict = dictIgnr ;
        else
            dict = dictHead ;
        return meDictionaryAddWord(dict,wd) ;
    }
    else if(n & SPELLWORD_DELETE)
    {
        meDictionary  *dict ;
        
        if(n & SPELLWORD_IGNORE)
            meDictionaryDeleteWord(dictIgnr,word,len) ;
        else
        {
            dict = dictHead ;
            while(dict != NULL)
            {
                meDictionaryDeleteWord(dict,word,len) ;
                dict = dict->next ;
            }
        }
        return meTRUE ;
    }
    else if(n & SPELLWORD_DERIV)
    {
        meSpellRule  *rr ;
        meUByte buff[meBUF_SIZE_MAX], *wd, *nwd, *swd ;
        
        if(n & SPELLWORD_INFO)
        {
            /* dump all derivatives into the current buffer */
            int ii ;
            
            if(meModeTest(frameCur->bufferCur->mode,MDVIEW))
                /* don't allow character insert if in read only */
                return (rdonly()) ;
            wd = buff+longestPrefixChange ;
            meStrcpy(wd,word) ;
            if(wordCheck(wd) > 0)
            {
                lineInsertString(0,wd) ;
                lineInsertNewline(0) ;
            }
            for(ii=0 ; ii<NOSPELLRULES ; ii++)
            {
                f = (meRuleFlags[ii] & RULE_PREFIX) ? 1:0 ;
                rr = meRuleTable[ii] ;
                while(rr != NULL)
                {
                    if(f)
                        nwd = wordApplyPrefixRule(wd,rr) ;
                    else if((swd = wordApplySuffixRule(wd,len,rr)) != NULL)
                        nwd = wd ;
                    else
                        nwd = NULL ;
                    if(nwd != NULL)
                    {
                        meStrcpy(word,nwd) ;
                        if(wordCheck(word) > 0)
                        {
                            lineInsertString(0,nwd) ;
                            lineInsertNewline(0) ;
                            if(meRuleFlags[ii] == (RULE_PREFIX|RULE_MIXABLE))
                            {
                                meSpellRule  *sr ;
                                int jj, ll ;
                                ll = len + rr->changeLen ;
                                for(jj=0 ; jj<NOSPELLRULES ; jj++)
                                {
                                    if(meRuleFlags[jj] == (RULE_SUFFIX|RULE_MIXABLE))
                                    {                                    
                                        sr = meRuleTable[jj] ;
                                        while(sr != NULL)
                                        {
                                            if((swd = wordApplySuffixRule(nwd,ll,sr)) != NULL)
                                            {
                                                meStrcpy(word,nwd) ;
                                                if(wordCheck(word) > 0)
                                                {
                                                    lineInsertString(0,nwd) ;
                                                    lineInsertNewline(0) ;
                                                }
                                                wordSuffixRuleRemove(swd,sr) ;
                                            }
                                            sr = sr->next ;
                                        }
                                    }
                                }
                            }
                        }
                        if(f)
                            wordPrefixRuleRemove(wd,rr) ;
                        else
                            wordSuffixRuleRemove(swd,rr) ;
                    }
                    rr = rr->next ;
                }
            }
            return meTRUE ;
        }
        if(((f = mlCharReply((meUByte *)"Rule flag: ",0,NULL,NULL)) < FRSTSPELLRULE) || (f > LASTSPELLRULE))
            return meABORT ;
        f -= FRSTSPELLRULE ;
        wd = buff+longestPrefixChange ;
        meStrcpy(wd,word) ;

        rr = meRuleTable[f] ;
        f = (meRuleFlags[f] & RULE_PREFIX) ? 1:0 ;
        while(rr != NULL)
        {
            if(f)
                nwd = wordApplyPrefixRule(wd,rr) ;
            else if((nwd = wordApplySuffixRule(wd,len,rr)) != NULL)
                nwd = wd ;
            if(nwd != NULL)
            {
                spellWordToUserFont((meUByte *) resultStr,nwd) ;
                return meTRUE ;
            }
            rr = rr->next ;
        }
        resultStr[0] = '\0' ;
        return meTRUE ;
    }
    else if(n & SPELLWORD_GUESS)
    {
        meWORDBUF words[meGUESS_MAX] ;
        meUByte *ss, *dd ;
        int nw, cw, ll, ii ;
        
        if((nw = wordGuessGetList(word,len,words)) < 0)
            return meABORT ;
        dd = (meUByte *) resultStr ;
        *dd++ = '|' ;
        cw = 0 ;
        ll = meBUF_SIZE_MAX-3 ;
        while((cw < nw) && ((ii=meStrlen((ss=words[cw++]))) < ll))
        {
            spellWordToUserFont(dd,ss) ;
            dd += ii ;
            *dd++ = '|' ;
            ll -= ii+1 ;
        }
        *dd = '\0' ;
        return meTRUE ;
    }
    wordCurr = NULL ;
    if(((f=wordCheck(word)) == meABORT) && (wordCurr != NULL))
    {
        resultStr[0] = 'A' ;
        f = meWordGetFlagLen(wordCurr) ;
        memcpy(resultStr+1,meWordGetFlag(wordCurr),f) ;
        resultStr[f+1] = '\0' ;
    }
    else if(f == meTRUE)
        resultStr[0] = 'O' ;
    else
        resultStr[0] = 'E' ;
    if((n & SPELLWORD_INFO) && (wordCurr != NULL))
        meDictWordDump(wordCurr,(meUByte *) resultStr+1) ;
    return meTRUE ;
}


void
findWordsInit(meUByte *mask)
{
    if(sfwCurMask != NULL)
    {
        free(sfwCurMask) ;
        sfwCurMask = NULL ;
    }
    if(meSpellInitDictionaries() <= 0)
        return ;
    
    sfwCurWord = NULL ;
    sfwCurDict = dictHead ;
    sfwPreRule = NULL ;
    sfwSufRule = NULL ;
    if((sfwCurMask = meMalloc(meStrlen(mask)+1)) != NULL)
    {
        meUByte *ww, cc ;
        spellWordToLatinFont(sfwCurMask,mask) ;
        /* must convert to lower as the regexp works in user font */
        ww = sfwCurMask ;
        while((cc = *ww) != '\0')
            *ww++ = toLatinLower(cc) ;
        sfwFlags = (((cc = *sfwCurMask) == '.') || (cc == '\\') || (cc == '[') || (cc == '^')) ;
    }
}

meUByte *
findWordsNext(void)
{
    meUInt off ;
    meUByte *wp, *pwp, *swp, *ww, *flags ;
    int len, flen, preRule, sufRule ;
    
    if(sfwCurMask == NULL)
        return abortm ;
    wp = evalResult+longestPrefixChange ;
    if(sfwCurWord != NULL)
    {
        /* Get the word */
        len = meWordGetWordLen(sfwCurWord) ;
        ww = meWordGetWord(sfwCurWord) ;
        for(flen=0 ; flen<len ; flen++)
            wp[flen] = toLatinLower(ww[flen]) ;
        wp[len] = '\0' ;
        /* Get the flags */
        flen = meWordGetFlagLen(sfwCurWord) ;
        flags = meWordGetFlag(sfwCurWord) ;
        if(sfwPreRule != NULL)
        {
            pwp = wordApplyPrefixRule(wp,sfwPreRule) ;
            len += sfwPreRule->changeLen ;
            preRule = sfwPreRule->rule ;  
            if(sfwSufRule != NULL)
            {
                sufRule = sfwSufRule->rule ;  
                goto sfwJumpGotPreGotSuf ;
            }
            goto sfwJumpGotPreNotSuf ;
        }
        if(sfwSufRule != NULL)
        {
            sufRule = sfwSufRule->rule ;  
            goto sfwJumpNotPreGotSuf ;
        }
        goto sfwJumpNotPreNotSuf ;
    }
    while(sfwCurDict != NULL)
    {
        for(sfwCurIndx=0 ; sfwCurIndx<sfwCurDict->tableSize ; sfwCurIndx++)
        {
            off = meEntryGetAddr(sfwCurDict->table[sfwCurIndx]) ;
            while(off != 0)
            {
                sfwCurWord = (meDictWord *) mePtrOffset(sfwCurDict->table,off) ;
                if(!meWordGetErrorFlag(sfwCurWord) && 
                   ((ww=meWordGetWord(sfwCurWord))[0] != '\0'))
                {
                    /* Get the word */
                    len = meWordGetWordLen(sfwCurWord) ;
                    for(flen=0 ; flen<len ; flen++)
                        wp[flen] = toLatinLower(ww[flen]) ;
                    wp[len] = '\0' ;
                    if((sfwFlags || (wp[0] == sfwCurMask[0])) &&
                       (regexStrCmp(wp,sfwCurMask,meRSTRCMP_WHOLE) > 0))
                    {
                        spellWordToUserFont(wp,wp) ;
                        return wp ;
                    }
                    /* Get the flags */
sfwJumpNotPreNotSuf:
                    if((flen = meWordGetFlagLen(sfwCurWord)) > 0)
                    {
                        flags = meWordGetFlag(sfwCurWord) ;
                        if(sfwFlags || (wp[0] == sfwCurMask[0]))
                        {
                            /* try all the allowed suffixes */
                            for(sufRule=0 ; sufRule<flen ; sufRule++)
                            {
                                if(meRuleFlags[flags[sufRule]-FRSTSPELLRULE] & RULE_SUFFIX)
                                {
                                    sfwSufRule = meRuleTable[flags[sufRule]-FRSTSPELLRULE] ;
                                    while(sfwSufRule != NULL)
                                    {
                                        if((swp = wordApplySuffixRule(wp,len,sfwSufRule)) != NULL)
                                        {
                                            if(regexStrCmp(wp,sfwCurMask,meRSTRCMP_WHOLE) > 0)
                                            {
                                                sfwSufRule->rule = sufRule ;  
                                                spellWordToUserFont(wp,wp) ;
                                                return wp ;
                                            }
                                            wordSuffixRuleRemove(swp,sfwSufRule) ;
                                        }
sfwJumpNotPreGotSuf:
                                        sfwSufRule = sfwSufRule->next ;
                                    }
                                }
                            }
                        }
                        for(preRule=0 ; preRule<flen ; preRule++)
                        {
                            if(meRuleFlags[flags[preRule]-FRSTSPELLRULE] & RULE_PREFIX)
                            {
                                sfwPreRule = meRuleTable[flags[preRule]-FRSTSPELLRULE] ;
                                while(sfwPreRule != NULL)
                                {
                                    if((sfwFlags || (sfwPreRule->append[0] == sfwCurMask[0])) &&
                                       ((pwp = wordApplyPrefixRule(wp,sfwPreRule)) != NULL))
                                    {
                                        len += sfwPreRule->changeLen ;
                                        if(regexStrCmp(pwp,sfwCurMask,meRSTRCMP_WHOLE) > 0)
                                        {
                                            sfwPreRule->rule = preRule ;  
                                            spellWordToUserFont(pwp,pwp) ;
                                            return pwp ;
                                        }
sfwJumpGotPreNotSuf:
                                        if(meRuleFlags[flags[preRule]-FRSTSPELLRULE] & RULE_MIXABLE)
                                        {
                                            /* try all the allowed suffixes */
                                            for(sufRule=preRule ; sufRule>=0 ; sufRule--)
                                                if(flags[sufRule] == '_')
                                                    break ;
                                            sufRule++ ;
                                            for( ; sufRule<flen ; sufRule++)
                                            {
                                                if(flags[sufRule] == '_')
                                                    break ;
                                                if(meRuleFlags[flags[sufRule]-FRSTSPELLRULE] == (RULE_SUFFIX|RULE_MIXABLE))
                                                {
                                                    sfwSufRule = meRuleTable[flags[sufRule]-FRSTSPELLRULE] ;
                                                    while(sfwSufRule != NULL)
                                                    {
                                                        if((swp = wordApplySuffixRule(pwp,len,sfwSufRule)) != NULL)
                                                        {
                                                            if(regexStrCmp(pwp,sfwCurMask,meRSTRCMP_WHOLE) > 0)
                                                            {
                                                                sfwPreRule->rule = preRule ;  
                                                                sfwSufRule->rule = sufRule ;  
                                                                spellWordToUserFont(pwp,pwp) ;
                                                                return pwp ;
                                                            }
                                                            wordSuffixRuleRemove(swp,sfwSufRule) ;
                                                        }
sfwJumpGotPreGotSuf:
                                                        sfwSufRule = sfwSufRule->next ;
                                                    }
                                                }
                                            }
                                        }
                                        len -= sfwPreRule->changeLen ;
                                        wordPrefixRuleRemove(wp,sfwPreRule) ;
                                    }
                                    sfwPreRule = sfwPreRule->next ;
                                }
                            }
                        }
                    }
                }
                off = meWordGetAddr(sfwCurWord) ;
            }
        }
        sfwCurDict = sfwCurDict->next ;
    }
    sfwCurWord = NULL ;
    return emptym ;
}

#endif

