/* $XTermId: luitconv.h,v 1.37 2021/02/18 19:23:20 tom Exp $ */

/*
Copyright 2010-2019,2021 by Thomas E. Dickey

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

#ifndef LUITCONV_H
#define LUITCONV_H

#ifdef USE_ICONV

#include <other.h>
#include <iconv.h>

typedef enum {
    umNONE = 0
    ,umPOSIX = 1
    ,umBUILTIN = 2
    ,umFONTENC = 4
    ,umICONV = 8
    ,umANY = (umPOSIX | umBUILTIN | umFONTENC | umICONV)
} UM_MODE;

typedef enum {
    usANY = 1
    ,us8BIT = 2
    ,us16BIT = 4
} US_SIZE;

#define FONT_ENCODING_UNICODE 1
typedef struct _FontMap {
    int type;
    unsigned (*recode) (unsigned, void *);	/* mapping function */
    void *client_data;		/* second parameter of the two above */
    struct _FontMap *next;	/* link to next element in list */
} FontMapRec, *FontMapPtr;

typedef struct _FontMapReverse {
    unsigned int (*reverse) (unsigned, void *);
    void *data;
} FontMapReverseRec, *FontMapReversePtr;

typedef struct _FontEnc {
    char *name;			/* the name of the encoding */
    char **aliases;		/* its aliases, null terminated */
    int size;			/* its size, either in bytes or rows */
    int row_size;		/* the size of a row, or 0 if bytes */
    FontMapPtr mappings;	/* linked list of mappings */
    struct _FontEnc *next;	/* link to next element */
    /* the following two were added in version 0.2 of the font interface */
    /* they should be kept at the end to preserve binary compatibility */
    int first;			/* first byte or row */
    int first_col;		/* first column in each row */
} FontEncRec, *FontEncPtr;

typedef struct {
    size_t size;		/* length of text[] */
    char *text;			/* value, in UTF-8 */
    unsigned ucs;		/* corresponding Unicode value */
} MappingData;

typedef struct {
    unsigned ucs;
    unsigned ch;
} ReverseData;

typedef struct _LuitConv {
    struct _LuitConv *next;
    char *encoding_name;
    iconv_t iconv_desc;
    /* internal tables for input/output */
    MappingData *table_utf8;	/* UTF-8 equivalents of 8-bit codes */
    ReverseData *rev_index;	/* reverse-index */
    size_t len_index;		/* index length */
    size_t table_size;		/* length of table_utf8[] and rev_index[] */
    /* data expected by caller */
    FontMapRec mapping;
    FontMapReverseRec reverse;
} LuitConv;

typedef struct {
    unsigned source;
    unsigned target;
} BuiltInMapping;

typedef struct _BuiltInCharset {
    const char *name;		/* table name, for lookups */
    const BuiltInMapping *table;
    size_t length;		/* length of table[] */
} BuiltInCharsetRec;

extern UM_MODE lookup_order[];

extern FontEncPtr luitGetFontEnc(const char *, UM_MODE);
extern FontMapPtr luitLookupMapping(const char *, UM_MODE, US_SIZE);
extern FontMapReversePtr luitLookupReverse(FontMapPtr);
extern LuitConv *luitLookupEncoding(FontMapPtr);
extern const BuiltInCharsetRec builtin_encodings[];
extern unsigned luitMapCodeValue(unsigned, FontMapPtr);
extern void luitFreeFontEnc(FontEncPtr);

#ifdef NO_LEAKS
extern void luitDestroyReverse(FontMapReversePtr);
#endif

#define LookupMapping(encoding_name,usize) \
	luitLookupMapping(encoding_name, umANY, usize)

#define LookupReverse(fontmap_ptr) \
	luitLookupReverse(fontmap_ptr)

#define MapCodeValue(code, fontmap_ptr) \
	luitMapCodeValue(code, fontmap_ptr)

#else /* !USE_ICONV */

#include <X11/fonts/fontenc.h>

#define LookupMapping(encoding_name,usize) \
	FontEncMapFind(encoding_name, FONT_ENCODING_UNICODE, -1, -1, NULL)

#define LookupReverse(fontmap_ptr) \
	FontMapReverse(fontmap_ptr)

#define MapCodeValue(code, fontmap_ptr) \
	FontEncRecode(code, fontmap_ptr)

#endif /* USE_ICONV */

typedef unsigned short UCode;

typedef struct _FontEncSimpleMap {
    unsigned len;		/* might be 0x10000 */
    UCode row_size;
    UCode first;
    UCode *map;			/* fontenc makes this const */
} FontEncSimpleMapRec, *FontEncSimpleMapPtr;

#ifdef FONT_ENCODING_POSTSCRIPT
typedef struct _FontEncSimpleName {
    unsigned len;
    UCode first;
    char **map;
} FontEncSimpleNameRec, *FontEncSimpleNamePtr;
#endif /* FONT_ENCODING_POSTSCRIPT */

#define MIN_UCODE 0x0000
#define MAX_UCODE 0xffff

#define rowOf(code) ((code) / 0x100)
#define colOf(code) ((code) & 0xff)

extern FontEncPtr lookupOneFontenc(const char *);
extern int reportBuiltinCharsets(void);
extern int reportFontencCharsets(void);
extern int reportIconvCharsets(void);
extern int showBuiltinCharset(const char *);
extern int showFontencCharset(const char *);
extern int showIconvCharset(const char *);
extern int typeOfFontenc(FontEncPtr);
extern unsigned luitRecode(unsigned, void *);
extern unsigned shiftOfFontenc(FontEncPtr);

#endif /* LUITCONV_H */
