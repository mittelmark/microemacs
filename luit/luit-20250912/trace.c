/* $XTermId: trace.c,v 1.8 2025/09/12 08:26:35 tom Exp $ */

/*
Copyright 2010-2019,2025 by Thomas E. Dickey

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

#include <config.h>

#include <unistd.h>
#include <trace.h>

void
Trace(const char *fmt, ...)
{
    static pid_t first_pid;
    static FILE *fp = NULL;
    va_list ap;
    if (first_pid == 0)
	first_pid = getpid();
    if (fp == NULL)
	fp = fopen("Trace.out", "w");
    va_start(ap, fmt);
    if (getpid() != first_pid)
	fprintf(fp, "child:");
    vfprintf(fp, fmt, ap);
    fflush(fp);
    va_end(ap);
}
