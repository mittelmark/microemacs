/*
 * access.c - Define access mode flags for Windows/MinGW.
 *
 * These flags may not be defined in all Windows headers.
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com)
 * This is free software; you can redistribute it and/or modify
 * under the terms of the GNU General Public License.
 */

#ifdef _WIN32

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

#endif /* _WIN32 */