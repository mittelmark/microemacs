/*
 * win32dir.c - Windows directory compatibility stubs
 *
 * These functions provide Windows-specific directory functionality
 * that isn't available in POSIX systems.
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com)
 * This is free software; you can redistribute it and/or modify
 * under the terms of the GNU General Public License.
 */

#ifdef _WIN32

#include <stdio.h>

/*
 * _chdrive - Change the current drive
 * Windows-specific - not available in POSIX.
 * Returns 0 on success, -1 on error.
 */
int
_chdrive(int drive)
{
    /* Not supported in POSIX - stub that always succeeds */
    (void)drive;
    return 0;
}

/*
 * _getdrive - Get the current drive number
 * Windows-specific - not available in POSIX.
 * Returns the drive number (1=A, 2=B, 3=C, etc.)
 * Default to C: (3) for POSIX systems.
 */
int
_getdrive(void)
{
    /* Default to C: drive */
    return 3;
}

#endif /* _WIN32 */