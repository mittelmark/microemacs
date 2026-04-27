/*
 * winsock2.h - Fix select() conflict for MSYS2/MinGW
 *
 * The problem: sys/select.h declares select() with struct timeval *,
 * but winsock2.h declares it with const TIMEVAL *.
 * This header fixes the conflict by undefining select before
 * including the system winsock2.h.
 *
 * Copyright (C) 2025 JASSPA
 */
#ifndef _WINSOCK2_H
#define _WINSOCK2_H

#ifdef _WIN32
/* Undef the POSIX select to prevent conflict - winsock2.h will provide its own */
#ifdef select
#undef select
#endif

/* Include the system winsock2.h */
#include <winsock2.h>

#endif /* _WIN32 */
#endif /* _WINSOCK2_H */