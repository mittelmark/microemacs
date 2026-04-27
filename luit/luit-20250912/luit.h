/* $XTermId: luit.h,v 1.26 2025/09/12 19:12:02 tom Exp $ */

/*
Copyright 2010-2018,2025 by Thomas E. Dickey
Copyright (c) 2001 by Juliusz Chroboczek

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

#ifndef LUIT_LUIT_H
#define LUIT_LUIT_H 1

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <sys/types.h>

#ifndef GCC_UNUSED
#define GCC_UNUSED		/* ARGSUSED */
#endif

#ifndef GCC_NORETURN
#define GCC_NORETURN		/* nothing */
#endif

#ifndef GCC_PRINTFLIKE
#define GCC_PRINTFLIKE(a,b)	/* nothing */
#endif

extern const char *locale_alias;
extern int fill_fontenc;
extern int ignore_locale;
extern int iso2022;
extern int sevenbit;
extern int ilog;
extern int olog;
extern int verbose;

#define MAXCOLS 78

void Message(const char *f, ...) GCC_PRINTFLIKE(1,2);
void Warning(const char *f, ...) GCC_PRINTFLIKE(1,2);
void FatalError(const char *f, ...) GCC_PRINTFLIKE(1,2) GCC_NORETURN;

#define VERBOSE(level,params) if (verbose >= level) Message params

#define IsEmpty(s) ((s) == NULL || (*(s)) == '\0')
#define NonNull(s) ((s) ? (s) : "<null>")

#endif /* LUIT_LUIT_H */
