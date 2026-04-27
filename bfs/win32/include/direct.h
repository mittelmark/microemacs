/*
 * direct.h - Compatibility header for MinGW-w64 / MSYS2
 *
 * MinGW-w64 and MSYS2 do not provide direct.h - this provides
 * compatibility wrappers for the MSVC-style underscore functions.
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com)
 * This is free software; you can redistribute it and/or modify
 * under the terms of the GNU General Public License.
 */

#ifndef _DIRECT_H
#define _DIRECT_H

#include <unistd.h>
#include <sys/stat.h>

/*
 * _getcwd - Get current working directory
 * Note: POSIX getcwd() takes two arguments, _getcwd takes buffer and size.
 */
#define _getcwd(buf, size) getcwd(buf, size)

/*
 * _chdir - Change directory (POSIX uses chdir directly)
 */
#define _chdir(dir) chdir(dir)

/*
 * _mkdir - Make directory
 * Use POSIX mkdir with mode 0755 as default for single-arg case.
 */
#define _mkdir(path) mkdir(path, 0755)

/*
 * _rmdir - Remove directory (POSIX uses rmdir directly)
 */
#define _rmdir(dir) rmdir(dir)

/*
 * _chdrive - Change current drive (Windows only)
 * Not available in POSIX - this is a no-op stub.
 */
int _chdrive(int drive);

/*
 * _getdrive - Get current drive (Windows only)
 * POSIX doesn't have this, so we default to C: (drive 3).
 */
int _getdrive(void);

/*
 * _getdcwd - Get current directory for specific drive
 * POSIX doesn't have this, so we use getcwd on the root.
 */
#define _getdcwd(drive, buf, size) getcwd(buf, size)

/*
 * _fullpath - Get full path of a file
 * POSIX equivalent is realpath(), but with different signature.
 */
#define _fullpath(buf, path, size) realpath(path, buf)

#endif /* _DIRECT_H */