/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * key.c - Key and name table porcessing.
 *
 * Copyright (C) 1991-2009 JASSPA (www.jasspa.com)
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
 * Created:     02/12/91
 * Synopsis:    Key and name table porcessing.
 * Authors:     Jon Green & Steven Phillips
 */

#define	__KEYC			/* Define the filename */

#include "emain.h"
#include "efunc.h"

/* Counts the size of the key table and stores it in the global variable *
 * keyTableSize
 */
void
count_key_table(void)
{
    register meBind *ktp;		/* Keyboard character array */
    register int     ii;		/* Index counter */

/*---	Find end of table, incrementing i. */

    for (ktp=keytab, ii=0 ; ktp->code != ME_INVALID_KEY ; ii++, ktp++)
        ;
    keyTableSize = (meUShort) ii ;
}

/*****************************************************************************
*								x.
*	Function Title:	decode_key ()
*
******************************************************************************
*
*	Synopsis:	Decode a key into an index value.
*
*	Invocation:	int	decode_key (code)
*			int	code;
*
*	Description:	This routine will decode a character sequence into
*			a character key.
*
*	Inputs:		code - The character code.
*
*	Outputs:	Returns the function name or NULL.
*
*****************************************************************************/

/* This is altered by isearch because it uses mlBinds, but when gets exit
 * command it must re-evaluate the same key but with no mlBinds
 */

int
decode_key(meUShort ocode, meUInt *arg)
{
    register meBind  *ktp;			/* Keyboard character array */
    register int      low;			/* Lowest index in table. */
    register int      hi;			/* Hightest index in table. */
    register int      mid;			/* Mid value. */
    register int      status;			/* Status of comparison. */
    register meUShort code=ocode;    
try_again:
#if MEOPT_LOCALBIND
    if(useMlBinds)
    {
        ktp = mlBinds ;
        mid = mlNoBinds ;
    }
    else
    {
        mid = frameCur->bufferCur->bindCount ;
        ktp = frameCur->bufferCur->bindList ;
    }
    while(mid>0)
        if(ktp[--mid].code == code)
        {
            *arg = ktp[mid].arg ;
            return ktp[mid].index ;
        }

#endif
    /* binary chop through the key table looking for the character code.  
    ** If found then return the index into the names table.
    */
    ktp = keytab ;
    hi  = keyTableSize-1;	/* Set hi water to end of table */
    low = 0;			/* Set low water to start of table */
    
    do
    {
        mid = (low + hi) >> 1;		/* Get mid value. */
        if ((status=code-ktp[mid].code) == 0)
        {
            /* Found - return index */
            *arg = ktp[mid].arg ;
            return ktp[mid].index ;
        }
        else if (status < 0)
            hi = mid - 1;		/* Discard bottom half */
        else
            low = mid + 1;		/* Discard top half */
    } while (low <= hi);		/* Until converges */
    /* If this is an Alt key without any prefix's (esc, C-x etc.) then
     * change the key to be the 1st prefix without the Alt, e.g.
     *     A-space -> esc space
     * and then look this up
     */
    if(code & ME_ALT)
    {
#if MEOPT_OSD
        if((meSystemCfg & meSYSTEM_ALTMENU) &&
           ((code & (ME_SPECIAL|ME_PREFIX_MASK)) == 0) &&
           ((status = osdMainMenuCheckKey(code & 0x0ff)) != 0))
        {
            /* Found - return index */
            *arg = (meUInt) (status+0x80000000) ;
            return CK_OSD ;
        }
#endif
        if((meSystemCfg & meSYSTEM_ALTPRFX1) &&
           ((code & ME_PREFIX_MASK) == 0))
        {
            code = (code & ~ME_ALT)|ME_PREFIX1 ;
            goto try_again ;
        }
    }
    if((charKeyboardMap != NULL) && ((code & ME_SPECIAL) == 0) &&
       (charKeyboardMap[code & 0x0ff] != 0))
    {
        meUByte *ckm=charKeyboardMap ;
        code = (ocode & 0xff00) | charKeyboardMap[code & 0x0ff] ;
        charKeyboardMap = NULL ;
        status = decode_key(code,arg) ;
        charKeyboardMap = ckm ;
        return status ;
    }
    *arg = 0 ;
    return -1 ;				/* Not found - return error */
}

/*****************************************************************************
*								x.
*	Function Title:	delete_key ()
*
******************************************************************************
*
*	Synopsis:	Delete a key from the key table.
*
*	Invocation:	void	delete_key (index)
*			int	index;
*
*	Description:	This deletes the  specified key from  the key table.
*			The key is speficied with it's absolute table index.
*
*	Inputs:		index - The key table item to delete.
*
*	Outputs:	None - deletes item from table.
*
*****************************************************************************/

int
delete_key(register meUShort code)
{
    register meBind	*ktp;			/* Keyboard character array */
    register int	ii ;			/* Index counter */

    /* move to the right part of the table */
    ii = keyTableSize ;
    ktp = keytab ;
    while(--ii >= 0)
    {
        if(ktp->code == code)
            break ;
        ktp++ ;
    }
    if(ii < 0)
        return meFALSE ;
    keyTableSize-- ;
    /* move all infront back one */
    while(--ii >= 0)
    {
        memcpy(ktp,ktp+1,sizeof(meBind)) ;
        ktp++ ;
    }
    return meTRUE ;
}	/* End of "delete_key" () */

/*****************************************************************************
*								x.
*	Function Title:	insert_key ()
*
******************************************************************************
*
*	Synopsis:	Insert a new key into the key table.
*
*	Invocation:	int	insert_key (code, index)
*			int	code;
*			int	index;
*
*	Description:	This routine inserts a new key into the key table in
*			key-code order.
*
*	Inputs:		code - The new key code to add.
*			index - The new index to add.
*
*	Outputs:	Returns NULL if error.
*
*****************************************************************************/

int
insert_key (register meUShort code, meUShort index, meUInt arg)
/*  code - Code to add to key table. */
/* index - Index for the code. */
{
    register int	i;			/* Index counter */

    /* Is there room in the table */
    if(keyTableSize >= meBIND_MAX-1)		/* No space in table ?? */
        return meFALSE ;                          /* No - return error. */

    /* Find the insertion point, then move the trailing bindings and sentinel. */
    i = keyTableSize ;
    while((i > 0) && (keytab[i-1].code > code))
        i-- ;
    memmove(&keytab[i+1],&keytab[i],(keyTableSize-i+1)*sizeof(meBind)) ;
    keyTableSize++ ;
    keytab[i].index = index ;	/* Add new index */
    keytab[i].code  = code ;	/* Add new function code */
    keytab[i].arg   = arg ;	/* Add new argument */

    return meTRUE ;
}	/* End of "insert_key" () */
