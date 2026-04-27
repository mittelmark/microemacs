/* $XTermId: trace.h,v 1.9 2019/01/06 20:53:11 tom Exp $ */

/*
Copyright 2010-2018,2019 by Thomas E. Dickey

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

#ifndef TRACE_H
#define TRACE_H

#include <luit.h>

void Trace(const char *fmt, ...) GCC_PRINTFLIKE(1,2);

#ifdef OPT_TRACE
#define TRACE(params) Trace params
#if OPT_TRACE > 1
#define TRACE2(params) Trace params
#else
#define TRACE2(params)		/* nothing */
#endif
#else
#define TRACE(params)		/* nothing */
#define TRACE2(params)		/* nothing */
#endif

#define TRACE_ERR(msg) TRACE((msg ": %s\n", NonNull(strerror(errno))))

#endif /* TRACE_H */
