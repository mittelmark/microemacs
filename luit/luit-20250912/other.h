/* $XTermId: other.h,v 1.13 2025/09/12 07:52:28 tom Exp $ */

/*
Copyright 2006-2010,2025 by Thomas E. Dickey
Copyright (c) 2002 by Tomohiro KUBOTA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef LUIT_OTHER_H
#define LUIT_OTHER_H 1

#include <luit.h>		/* include this, for self-contained headers */

#include <luitconv.h>
#include <trace.h>

#undef UCHAR
#define UCHAR unsigned char

#undef UINT
#define UINT unsigned int

#define UChar(n) ((UCHAR)(n))

typedef struct {
    FontMapPtr mapping;
    FontMapReversePtr reverse;
    int buf;
} aux_gbk;

typedef struct {
    UCHAR buf[4];
    int buf_ptr, len;
} aux_utf8;

typedef struct {
    FontMapPtr x0208mapping;
    FontMapPtr x0201mapping;
    FontMapReversePtr x0208reverse;
    FontMapReversePtr x0201reverse;
    int buf;
} aux_sjis;

typedef struct {
    FontMapPtr mapping;
    FontMapReversePtr reverse;
    int buf;
} aux_hkscs;

typedef struct {
    FontMapPtr cs0_mapping;	/* gb18030.2000-0 */
    FontMapReversePtr cs0_reverse;

    FontMapPtr cs1_mapping;	/* gb18030.2000-1 */
    FontMapReversePtr cs1_reverse;

    int linear;			/* set to '1' if stack_gb18030 linearized a 4bytes seq */
    int buf[3];
    int buf_ptr;
} aux_gb18030;

typedef union {
    aux_gbk gbk;
    aux_utf8 utf8;
    aux_sjis sjis;
    aux_hkscs hkscs;
    aux_gb18030 gb18030;
} OtherState, *OtherStatePtr;

int init_gbk(OtherStatePtr);
UINT mapping_gbk(UINT, OtherStatePtr);
UINT reverse_gbk(UINT, OtherStatePtr);
int stack_gbk(UINT, OtherStatePtr);

int init_utf8(OtherStatePtr);
UINT mapping_utf8(UINT, OtherStatePtr);
UINT reverse_utf8(UINT, OtherStatePtr);
int stack_utf8(UINT, OtherStatePtr);

int init_sjis(OtherStatePtr);
UINT mapping_sjis(UINT, OtherStatePtr);
UINT reverse_sjis(UINT, OtherStatePtr);
int stack_sjis(UINT, OtherStatePtr);

int init_hkscs(OtherStatePtr);
UINT mapping_hkscs(UINT, OtherStatePtr);
UINT reverse_hkscs(UINT, OtherStatePtr);
int stack_hkscs(UINT, OtherStatePtr);

int init_gb18030(OtherStatePtr);
UINT mapping_gb18030(UINT, OtherStatePtr);
UINT reverse_gb18030(UINT, OtherStatePtr);
int stack_gb18030(UINT, OtherStatePtr);

#endif /* LUIT_OTHER_H */
