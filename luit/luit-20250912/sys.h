/* $XTermId: sys.h,v 1.23 2021/01/13 22:13:24 tom Exp $ */

/*
Copyright 2010-2013,2021 by Thomas E. Dickey
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

#ifndef LUIT_SYS_H
#define LUIT_SYS_H 1

#if defined(__GNUC__) && defined(_FORTIFY_SOURCE)
#define USE_IGNORE_RC
extern int ignore_unused;
#define IGNORE_RC(func) ignore_unused = (int) func
#else
#define IGNORE_RC(func) (void) func
#endif /* gcc workarounds */

#define IO_CanRead   1
#define IO_CanWrite  2
#define IO_Closed    4

#define TypeCalloc(type)    (type *) calloc((size_t) 1, sizeof(type))
#define TypeCallocN(type,n) (type *) calloc((size_t) (n), sizeof(type))

#define SizeOf(v)        (sizeof(v) / sizeof(v[0]))

int waitForOutput(int fd);
int waitForInput(int fd1, int fd2);
int setWindowSize(int sfd, int dfd);
int installHandler(int signum, void (*handler) (int));
int copyTermios(int sfd, int dfd);
int restoreTermios(int sfd);
int setRawTermios(int sfd);
char *my_basename(char *path);
int allocatePty(int *pty_return, char **line_return);
int openTty(char *line);
int droppriv(void);
char *strmalloc(const char *value);

#ifdef HAVE_STRCASECMP
#define StrCaseCmp(a,b) strcasecmp(a,b)
#else
int my_strcasecmp(const char *a, const char *b);
#define StrCaseCmp(a,b) my_strcasecmp(a,b)
#endif

#ifdef NO_LEAKS
void luit_leaks(void);
void charset_leaks(void);
void fontenc_leaks(void);
void iso2022_leaks(void);
void luitconv_leaks(void);
void ExitProgram(int code) GCC_NORETURN;
#else
#define ExitProgram(code) exit(code)
#endif

#define ExitSuccess() ExitProgram(EXIT_SUCCESS)
#define ExitFailure() ExitProgram(EXIT_FAILURE)

#endif /* LUIT_SYS_H */
