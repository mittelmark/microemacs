/*
 * access() implementation for Windows/MinGW.
 *
 * Copyright (C) 2025 Jon Green.
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License.
 */

#include <errno.h>

#ifdef _WIN32

/*
 * Windows/MSVCRT provides _access() which we can wrap.
 * MinGW also provides access() in io.h, but we can just use _access
 * and map the mode flags.
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