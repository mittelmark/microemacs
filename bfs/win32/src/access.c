/*
 * access() implementation for Windows/MinGW.
 *
 * Copyright (C) 2025 Jon Green.
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License.
 */

#ifdef _WIN32

#include <errno.h>
#include <fcntl.h>
#include <io.h>

/* Define access mode flags in case they're not in system headers */
#ifndef F_OK
#define F_OK    0x00
#endif

#ifndef R_OK
#define R_OK    0x04
#endif

#ifndef W_OK
#define W_OK    0x02
#endif

#ifndef X_OK
#define X_OK    0x01
#endif

/*
 * Windows/MSVCRT provides _access() which we can wrap.
 */

int
access(const char *path, int mode)
{
    int msvcmode = 0;

    /* Map Unix mode flags to MSVCRT flags */
    if (mode & R_OK)
        msvcmode |= 4;  /* _R_OK */
    if (mode & W_OK)
        msvcmode |= 2;  /* _W_OK */
    if (mode & X_OK)
        msvcmode |= 1;  /* _X_OK */
    if (mode & F_OK)
        msvcmode |= 0;  /* _F_OK - just check existence */

    return _access(path, msvcmode);
}

#endif /* _WIN32 */