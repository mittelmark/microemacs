/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * encoding.h - Encoding conversion types and declarations.
 *
 * Provides on-the-fly conversion between:
 * - UTF-8 (terminal I/O)
 * - ISO-8859-* (Latin-1 through Latin-16)
 * - Windows-125x (CP1250 through CP1254)
 * - KOI8-R, CP437, CP866
 * - ASCII
 *
 * The encoding conversion approach (storing text in a single-byte
 * internal encoding and converting on I/O) is inspired by luit
 * by Juliusz Chroboczek (https://www.cl.cam.ac.uk/~mgk25/unicode.html),
 * licensed under the MIT license.
 *
 * Copyright (C) 2025 Detlef Groth (dgroth@gmx.de)
 * Copyright (C) 1999-2009 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ME_ENCODING_H
#define ME_ENCODING_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Encoding types */
typedef enum {
    ME_ENC_UTF8 = 0,
    ME_ENC_ISO8859_1,
    ME_ENC_CP1252,
    ME_ENC_CP1251,
    ME_ENC_CP1253,
    ME_ENC_CP1250,
    ME_ENC_CP1254,
    ME_ENC_KOI8_R,
    ME_ENC_ISO8859_2,
    ME_ENC_ISO8859_3,
    ME_ENC_ISO8859_4,
    ME_ENC_ISO8859_5,
    ME_ENC_ISO8859_7,
    ME_ENC_ISO8859_9,
    ME_ENC_ISO8859_10,
    ME_ENC_ISO8859_13,
    ME_ENC_ISO8859_15,
    ME_ENC_ISO8859_16,
    ME_ENC_CP437,
    ME_ENC_CP866,
    ME_ENC_ASCII
} meEncoding;

/* Converter context */
typedef struct {
    meEncoding from;
    meEncoding to;
    int strict;
    char replacement;
} meConv;

void meConvInit(meConv *conv, meEncoding from, meEncoding to);
int meConvChar(meConv *conv, const unsigned char *in, size_t in_len,
               unsigned char *out, size_t out_len);
int meConvString(meConv *conv, const unsigned char *in, size_t in_len,
                 unsigned char *out, size_t out_len);
meEncoding meDetectBOM(const unsigned char *data, size_t len);
const char *meEncodingName(meEncoding enc);
int meUtf8CharSize(const unsigned char *c);
int meUtf8IsValid(const unsigned char *data, size_t len);
int32_t meUtf8Decode(const unsigned char *c);
size_t meUtf8Encode(int32_t codepoint, unsigned char *out);
int meUtf8ValidSeqLen(const unsigned char *s);
meEncoding meEncodingFromName(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* ME_ENCODING_H */
