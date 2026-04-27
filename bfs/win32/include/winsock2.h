/*
 * winsock2.h - Compatibility header to fix select() conflict
 *
 * MSYS2 winsock2.h declares select() with TIMEVAL *, but sys/select.h declares it
 * with struct timeval *. This header resolves the conflict.
 *
 * Copyright (C) 2025 JASSPA (www.jasspa.com)
 * This is free software; you can redistribute it and/or modify
 * under the terms of the GNU General Public License.
 */

#ifndef _WINSOCK2_H
#define _WINSOCK2_H

#ifdef _WIN32

/*
 * Fix the select() conflict - undef the POSIX select before including winsock2.h.
 * This prevents "conflicting types for 'select'" error.
 */
#ifdef select
#undef select
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#endif /* _WIN32 */

#endif /* _WINSOCK2_H */