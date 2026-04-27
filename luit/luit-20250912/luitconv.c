/* $XTermId: luitconv.c,v 1.131 2025/09/12 19:08:45 tom Exp $ */

/*
Copyright 2010-2024,2025 by Thomas E. Dickey

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

#include <iso2022.h>

#include <sys.h>

#ifdef HAVE_LANGINFO_CODESET
#include <locale.h>
#include <langinfo.h>
#endif

#ifndef ICONV_CONST
#define ICONV_CONST		/* nothing */
#endif

/*
 * This uses a similar approach to vile's support for wide/narrow locales.
 *
 * We use the iconv library to construct a mapping forward for each byte,
 * into UTF-8, and then use that mapping to construct the reverse mapping
 * from UTF-8 into the original set of single byte values.
 */

#define MAX8		0x100
#define MAX16		0x10000

#define NO_ICONV  (iconv_t)(-1)

static LuitConv *all_conversions;

/******************************************************************************/
static int
ConvToUTF32(unsigned *target, const char *source, size_t limit)
{
#define CH(n) (UCHAR)((*target) >> ((n) * 8))
    int rc = 0;
    int j;
    UINT mask = 0;

    /*
     * Find the number of bytes we will need from the source.
     */
    if ((*source & 0x80) == 0) {
	rc = 1;
	mask = (UINT) * source;
    } else if ((*source & 0xe0) == 0xc0) {
	rc = 2;
	mask = (UINT) (*source & 0x1f);
    } else if ((*source & 0xf0) == 0xe0) {
	rc = 3;
	mask = (UINT) (*source & 0x0f);
    } else if ((*source & 0xf8) == 0xf0) {
	rc = 4;
	mask = (UINT) (*source & 0x07);
    } else if ((*source & 0xfc) == 0xf8) {
	rc = 5;
	mask = (UINT) (*source & 0x03);
    } else if ((*source & 0xfe) == 0xfc) {
	rc = 6;
	mask = (UINT) (*source & 0x01);
    }

    if ((size_t) rc > limit) {	/* whatever it is, we cannot decode it */
	TRACE2(("limit failed %d/%ld in vl_conv_to_utf32\n", rc, limit));
	rc = 0;
    }

    /*
     * sanity-check.
     */
    if (rc > 1) {
	for (j = 1; j < rc; j++) {
	    if ((source[j] & 0xc0) != 0x80)
		break;
	}
	if (j != rc) {
	    TRACE2(("check failed %d/%d in vl_conv_to_utf32\n", j, rc));
	    rc = 0;
	}
    }

    if (target != NULL) {
	int shift = 0;
	*target = 0;
	for (j = 1; j < rc; j++) {
	    *target |= (UINT) (source[rc - j] & 0x3f) << shift;
	    shift += 6;
	}
	*target |= mask << shift;

	TRACE2(("encode %2d:%.*s -> %#08x %02X.%02X.%02X.%02X\n",
		rc, rc, source,
		*target,
		CH(3), CH(2), CH(1), CH(0)));
    }
    return rc;
#undef CH
}

static int
ConvToUTF8(UCHAR * target, UINT source, size_t limit)
{
#define CH(n) (UCHAR)((source) >> ((n) * 8))
    int rc = 0;

    if (source <= 0x0000007f)
	rc = 1;
    else if (source <= 0x000007ff)
	rc = 2;
    else if (source <= 0x0000ffff)
	rc = 3;
    else if (source <= 0x001fffff)
	rc = 4;
    else if (source <= 0x03ffffff)
	rc = 5;
    else			/* (source <= 0x7fffffff) */
	rc = 6;

    if ((size_t) rc > limit) {	/* whatever it is, we cannot decode it */
	TRACE2(("limit failed in vl_conv_to_utf8 %d/%ld %#06x\n",
		rc, limit, source));
	rc = 0;
    }

    if (target != NULL) {
	switch (rc) {
	case 1:
	    target[0] = (UCHAR) CH(0);
	    break;

	case 2:
	    target[1] = (UCHAR) (0x80 | (CH(0) & 0x3f));
	    target[0] = (UCHAR) (0xc0 | (CH(0) >> 6) | ((CH(1) & 0x07) << 2));
	    break;

	case 3:
	    target[2] = (UCHAR) (0x80 | (CH(0) & 0x3f));
	    target[1] = (UCHAR) (0x80 | (CH(0) >> 6) | ((CH(1) & 0x0f) << 2));
	    target[0] = (UCHAR) (0xe0 | ((int) (CH(1) & 0xf0) >> 4));
	    break;

	case 4:
	    target[3] = (UCHAR) (0x80 | (CH(0) & 0x3f));
	    target[2] = (UCHAR) (0x80 | (CH(0) >> 6) | ((CH(1) & 0x0f) << 2));
	    target[1] = (UCHAR) (0x80 |
				 ((int) (CH(1) & 0xf0) >> 4) |
				 ((int) (CH(2) & 0x03) << 4));
	    target[0] = (UCHAR) (0xf0 | ((int) (CH(2) & 0x1f) >> 2));
	    break;

	case 5:
	    target[4] = (UCHAR) (0x80 | (CH(0) & 0x3f));
	    target[3] = (UCHAR) (0x80 | (CH(0) >> 6) | ((CH(1) & 0x0f) << 2));
	    target[2] = (UCHAR) (0x80 |
				 ((int) (CH(1) & 0xf0) >> 4) |
				 ((int) (CH(2) & 0x03) << 4));
	    target[1] = (UCHAR) (0x80 | (CH(2) >> 2));
	    target[0] = (UCHAR) (0xf8 | (CH(3) & 0x03));
	    break;

	case 6:
	    target[5] = (UCHAR) (0x80 | (CH(0) & 0x3f));
	    target[4] = (UCHAR) (0x80 | (CH(0) >> 6) | ((CH(1) & 0x0f) << 2));
	    target[3] = (UCHAR) (0x80 | (CH(1) >> 4) | ((CH(2) & 0x03) << 4));
	    target[2] = (UCHAR) (0x80 | (CH(2) >> 2));
	    target[1] = (UCHAR) (0x80 | (CH(3) & 0x3f));
	    target[0] = (UCHAR) (0xfc | ((int) (CH(3) & 0x40) >> 6));
	    break;
	}
	TRACE2(("decode %#08x %02X.%02X.%02X.%02X %d:%.*s\n", source,
		CH(3), CH(2), CH(1), CH(0), rc, rc, target));
    }

    return rc;			/* number of bytes needed in target */
#undef CH
}

/******************************************************************************/

static LuitConv *
newLuitConv(size_t elts)
{
    LuitConv *result = TypeCalloc(LuitConv);
    if (result != NULL) {
	TRACE(("newLuitConv(%u)\n", (unsigned) elts));
	result->table_size = elts;
	result->table_utf8 = TypeCallocN(MappingData, elts);
	result->rev_index = TypeCallocN(ReverseData, elts);
    }
    return result;
}

/*
 * Try to open a conversion from UTF-8 to the given encoding name.  This is
 * iconv(), and different implementations expect different syntax for the
 * name.  So if we do not at first succeed, try permuting the common
 * variations.
 */
static iconv_t
try_iconv_open(const char *guess, char **alias)
{
    int chcase;
    int mkcase;
    char *encoding_name = NULL;
    char *encoding_temp = NULL;
    char *p;
    iconv_t result;

    if (guess == NULL
	|| (encoding_name = malloc(strlen(guess) + 2)) == NULL
	|| (encoding_temp = malloc(strlen(guess) + 2)) == NULL) {
	free(encoding_name);
	return NO_ICONV;
    }

    strcpy(encoding_name, guess);
    TRACE(("try_iconv_open(%s)\n", NonNull(encoding_name)));
    result = iconv_open("UTF-8", encoding_name);

    /*
     * If the first try did not succeed, retry after changing the case of
     * the name and/or inserting a marker between the leading alphabetic
     * prefix and number.
     */
    if (result == NO_ICONV) {
	for (chcase = 0; chcase <= 2; ++chcase) {
	    strcpy(encoding_name, guess);

	    switch (chcase) {
	    case 0:		/* no change */
		break;
	    case 1:		/* uppercase */
		for (p = encoding_name; *p != '\0'; ++p) {
		    *p = (char) toupper(UChar(*p));
		}
		break;
	    case 2:		/* lowercase */
		for (p = encoding_name; *p != '\0'; ++p) {
		    *p = (char) tolower(UChar(*p));
		}
		break;
	    }

	    for (mkcase = 0; mkcase <= 3; ++mkcase) {

		switch (mkcase) {
		case 0:	/* no change */
		    break;
		case 1:	/* remove delimiter */
		    for (p = encoding_name; *p != '\0'; ++p) {
			if (*p == ' ' || *p == '-') {
			    if (p != encoding_name &&
				isalpha(UChar(p[-1])) &&
				isdigit(UChar(p[1]))) {
				while ((p[0] = p[1]) != '\0') {
				    ++p;
				}
			    }
			    break;
			}
		    }
		    break;
		case 2:	/* insert '-' */
		    for (p = encoding_name; *p != '\0'; ++p) {
			if (*p == '-')
			    break;
			if (isalpha(UChar(p[0])) &&
			    isdigit(UChar(p[1]))) {
			    strcpy(encoding_temp, p + 1);
			    p[1] = '-';
			    strcpy(p + 2, encoding_temp);
			    break;
			}
		    }
		    break;
		case 3:	/* change to ' ' */
		    for (p = encoding_name; *p != '\0'; ++p) {
			if (*p == '-') {
			    *p = ' ';
			    break;
			}
		    }
		    break;
		}

		result = iconv_open("UTF-8", encoding_name);
		if (result != NO_ICONV) {
		    TRACE(("...iconv_open'd with different name \"%s\"\n",
			   NonNull(encoding_name)));
		    break;
		}
	    }
	    if (result != NO_ICONV) {
		break;
	    }
	}
    }

    if (strcmp(encoding_name, guess)) {
	if (result != NO_ICONV) {
	    *alias = encoding_name;
	} else {
	    free(encoding_name);
	}
    } else {
	free(encoding_name);
    }

    free(encoding_temp);
    return result;
}

/******************************************************************************/

static int
cmp_rindex(const void *a, const void *b)
{
    const ReverseData *p = (const ReverseData *) a;
    const ReverseData *q = (const ReverseData *) b;
    return (int) (p)->ucs - (int) (q)->ucs;
}

#ifdef OPT_TRACE
static void
trace_convert(LuitConv * data, size_t which, unsigned gs)
{
    size_t j;
    char gsbuf[20];

    if (gs) {
	sprintf(gsbuf, "G%u ", gs);
    } else {
	gsbuf[0] = '\0';
    }

    TRACE(("convert %s%04X:%d:%04X:",
	   gsbuf,
	   (unsigned) which,
	   (int) data->table_utf8[which].size,
	   data->table_utf8[which].ucs));
    if (data->table_utf8[which].size) {
	for (j = 0; j < data->table_utf8[which].size; ++j) {
	    TRACE(("%c", data->table_utf8[which].text[j]));
	}
    }
    TRACE(("\n"));
}
#else
#define trace_convert(data,n,gs)	/* nothing */
#endif

/*
 * Assuming single-byte encoding, count the number of successful translations
 * to UTF-8 from the 0..255 range.
 */
static unsigned
count8bitIconv(iconv_t my_desc)
{
    unsigned result = 0;
    int n;

    for (n = 0; n < MAX8; ++n) {
	size_t converted;
	char input[80];
	ICONV_CONST char *ip = input;
	char output[80];
	char *op = output;
	size_t in_bytes = 1;
	size_t out_bytes = sizeof(output);

	input[0] = (char) n;
	input[1] = 0;
	(void) iconv(my_desc, NULL, NULL, NULL, NULL);
	converted = iconv(my_desc, &ip, &in_bytes, &op, &out_bytes);
	if (converted != (size_t) (-1)) {
	    ++result;
	}
    }
    TRACE(("count8bitIconv -> %u\n", result));
    return result;
}

#define legalUCode(n) ((n) < 0xd800 || (n) > 0xdfff)

/*
 * Given an encoding name, check to see if it is a single-byte encoding.
 * Return a suitable table-size, depending.
 *
 * iconv() provides no function for this purpose.  For demonstration purposes
 * (though it is claimed to be slow...) we can use iconv to convert a series
 * of UTF-8 codes to the given encoding, checking if any of those give more
 * than one byte.
 *
 * If we knew this was always ISO-2022 encoding, we could shorten the scan;
 * however luit does handle a few which are not.
 *
 * As a quick check, we first count the number of codes going _to_ UTF-8 in the
 * series 0..255, and if that is 256 there is no need for this function.
 *
 * FIXME: improve this by constructing a table of known single-byte encoding
 * names.
 */
static unsigned
sizeofIconvTable(const char *encoding_name, unsigned limit)
{
    unsigned result = MAX8;
    iconv_t my_desc = iconv_open(encoding_name, "UTF-8");
    if (my_desc != NO_ICONV) {
	unsigned n;
	unsigned total = 0;
	size_t in_bytes;
	UCHAR input[80];
	ICONV_CONST char *ip;
	size_t out_bytes;
	char output[80];
	char *op;

	TRACE(("sizeofIconvTable(%s, %u) opened...\n",
	       NonNull(encoding_name), limit));
	for (n = 0; n < MAX16; ++n) {
	    if (!legalUCode(n))
		continue;
	    if ((in_bytes = (size_t) ConvToUTF8(input, n, sizeof(input))) == 0) {
		continue;
	    }
	    ip = (ICONV_CONST char *) input;
	    op = output;
	    input[in_bytes] = 0;
	    out_bytes = sizeof(output);
	    (void) iconv(my_desc, NULL, NULL, NULL, NULL);
	    if (iconv(my_desc, &ip, &in_bytes, &op, &out_bytes) == (size_t) -1) {
		continue;
	    }
	    ++total;
	    /* if we have found all codes that the fast check could, quit */
	    if ((limit == 256) && (total >= limit)) {
		result = limit;
		break;
	    }
	}
	iconv_close(my_desc);
	TRACE(("...total codes %u\n", total));
	if (total > 256)
	    result = MAX16;
    }
    TRACE(("sizeofIconvTable(%s, %u) = %u\n", NonNull(encoding_name), limit, result));
    return result;
}

/*
 * Build forward/reverse mappings for single-byte encoding.
 */
static void
initialize8bitTable(LuitConv * data)
{
    unsigned n;

    TRACE(("initialize8bitTable\n"));

    data->len_index = 0;

    for (n = 0; n < MAX8; ++n) {
	size_t converted;
	char input[80];
	ICONV_CONST char *ip = input;
	char output[80];
	char *op = output;
	size_t in_bytes = 1;
	size_t out_bytes = sizeof(output);

	input[0] = (char) n;
	input[1] = 0;
	(void) iconv(data->iconv_desc, NULL, NULL, NULL, NULL);
	converted = iconv(data->iconv_desc, &ip, &in_bytes, &op, &out_bytes);
	if (converted == (size_t) (-1)) {
	    TRACE(("convert err %d\n", n));
	} else {
	    output[sizeof(output) - out_bytes] = 0;
	    data->table_utf8[n].size = sizeof(output) - out_bytes;
	    data->table_utf8[n].text = malloc(data->table_utf8[n].size);
	    memcpy(data->table_utf8[n].text,
		   output,
		   data->table_utf8[n].size);
	    if (ConvToUTF32((UINT *) 0,
			    data->table_utf8[n].text,
			    data->table_utf8[n].size)) {
		ConvToUTF32(&(data->table_utf8[n].ucs),
			    data->table_utf8[n].text,
			    data->table_utf8[n].size);
	    }
	    trace_convert(data, (size_t) n, 0);

	    data->rev_index[data->len_index].ucs = data->table_utf8[n].ucs;
	    data->rev_index[data->len_index].ch = n;
	    data->len_index++;
	}
    }
}

static UINT
dbcsDecode(const char *buffer, int length, int euc, unsigned *gs)
{
    UINT result = UChar(buffer[0]);

    switch (result) {
    case SS2:
	*gs = (unsigned) ((length > 1) ? 2 : 1);
	break;
    case SS3:
	*gs = (unsigned) ((length > 1) ? 3 : 1);
	break;
    default:
	*gs = (unsigned) ((result >= 128));
	break;
    }

    switch (*gs) {
    case 2:
	/* FALLTHRU */
    case 3:
	++buffer;
	--length;
	/* FALLTHRU */
    default:
	switch (length) {
	case 0:
	    break;
	case 1:
	    result = UChar(buffer[0]);
	    break;
	default:
	    result = (UINT) (((UChar(buffer[0]) << 8)
			      | UChar(buffer[1])));
	    if (euc)
		result ^= 0x8080;
	    break;
	}
	break;
    }
    return result;
}

/*
 * Build forward/reverse mappings for multi-byte encoding.
 *
 * As in sizeofIconvTable(), scan the encodings using a translation from UTF-8
 * to the target.  That gives us the reverse-mapping information, from which
 * we later construct the forward-mapping.
 *
 * TODO: update charset size as needed for -show-iconv
 */
static void
initialize16bitTable(const char *charset, LuitConv ** datap, unsigned gmax)
{
    unsigned n;
    unsigned gs;
    LuitConv *data;
    iconv_t my_desc = iconv_open(charset, "UTF-8");

    TRACE(("initialize16bitTable(%s) gmax %d\n", NonNull(charset), gmax));

    for (n = 0; n < gmax; ++n) {
	if (datap[n] != NULL) {
	    datap[n]->len_index = 0;
	}
    }

    if (my_desc != NO_ICONV) {
	int euc = !isOtherCharset(charset);

	TRACE(("...assume %s index\n", euc ? "EUC" : "non-EUC"));
	for (n = 0; n < MAX16; ++n) {
	    UCHAR input[80];
	    ICONV_CONST char *ip;
	    char output[80];
	    char *op = output;
	    size_t in_bytes;
	    size_t out_bytes = sizeof(output);
	    unsigned my_code;

	    if (!legalUCode(n))
		continue;
	    if ((in_bytes = (size_t) ConvToUTF8(input, n, sizeof(input))) == 0) {
		continue;
	    }
	    ip = (ICONV_CONST char *) input;
	    op = output;
	    input[in_bytes] = 0;
	    out_bytes = sizeof(output);
	    (void) iconv(my_desc, NULL, NULL, NULL, NULL);
	    if (iconv(my_desc, &ip, &in_bytes, &op, &out_bytes) == (size_t) -1) {
		continue;
	    }
	    my_code = dbcsDecode(output, (int) (op - output), euc, &gs);
	    if (gs >= gmax) {
		data = (gs == 1) ? datap[0] : NULL;
	    } else {
		data = datap[gs];
	    }
	    if ((data == NULL)
		|| (my_code >= data->table_size)
		|| data->table_utf8[my_code].text != NULL) {
		TRACE(("skip %d:%#x\n", gs, my_code));
		continue;
	    }
	    data->table_utf8[my_code].size = strlen((char *) input);
	    data->table_utf8[my_code].text = strmalloc((char *) input);
	    data->table_utf8[my_code].ucs = n;

	    trace_convert(data, (size_t) my_code, gs);

	    data->rev_index[data->len_index].ucs = n;
	    data->rev_index[data->len_index].ch = my_code;
	    data->len_index++;
	}
	iconv_close(my_desc);
    }
}

static unsigned
luitReverse(unsigned code, void *client_data GCC_UNUSED)
{
    unsigned result = code;
    LuitConv *data = (LuitConv *) client_data;

    TRACE(("luitReverse 0x%04X %p\n", code, (void *) data));

    if (data != NULL) {
	static const ReverseData zero_key;
	ReverseData *p;
	ReverseData key = zero_key;

	key.ucs = (UINT) code;
	p = (ReverseData *) bsearch(&key,
				    data->rev_index,
				    data->len_index,
				    sizeof(data->rev_index[0]),
				    cmp_rindex);

	if (p != NULL) {
	    result = p->ch;
	    TRACE(("...mapped %#x\n", result));
	}
    }
    return result;
}

/*
 * Translate one of luit's encoding names to one which is more likely to
 * work with iconv.
 */
static const char *
findEncodingAlias(const char *encoding_name)
{
    /* *INDENT-OFF* */
    static const struct {
	const char *luit_name;
	const char *iconv_name;
    } table[] = {
	/* 8-bit character sets */
	{ "KOI8-E",		"ISO-IR-111" },
	{ "TCVN-0",		"TCVN5712-1:1993" },
	{ "ibm-cp437",		"cp437" },
	{ "ibm-cp850",		"cp850" },
	{ "ibm-cp865",		"cp865" },
	{ "ibm-cp866",		"cp866" },
	{ "iso646.1973-0",	"US-ASCII" },
	{ "ISO 646 (1973)",     "US-ASCII" },
	{ "microsoft-cp1250",   "windows-1250" },
	{ "microsoft-cp1251",   "windows-1251" },
	{ "microsoft-cp1252",   "windows-1252" },
	/* EUC aliases */
	{ "ksx1001.1997-0",     "eucKR" }, /* fontenc -> ksc5601.1987-0 */
	{ "ksxjohab-1",         "JOHAB" },
	/* other (non-ISO-2022) character sets */
	{ "gb2312.1980-0",	"GB2312" },
	{ "big5.eten-0",	"BIG-5" },
	{ "big5hkscs-0",        "BIG5-HKSCS" },
	{ "gbk-0",	        "GBK" },
	{ "gb18030.2000-0",     "GB18030" },
#if 0
	{ "gb18030.2000-1",     "GB18030" },
#endif
    };
    /* *INDENT-ON* */

    size_t n;
    const char *result = NULL;

    TRACE(("findEncodingAlias(%s)\n", NonNull(encoding_name)));
    for (n = 0; n < SizeOf(table); ++n) {
	if (!lcStrCmp(encoding_name, table[n].luit_name)) {
	    result = table[n].iconv_name;
	    TRACE(("... matched '%s'\n", NonNull(result)));
	    break;
	}
    }
    return result;
}

static void
initializeBuiltInTable(LuitConv * data,
		       const BuiltInCharsetRec * builtIn,
		       int enc_file)
{
    UCHAR buffer[20];
    size_t n;
    size_t need;

    TRACE(("initializing %s '%s'\n",
	   enc_file ? "external" : "built-in",
	   NonNull(builtIn->name)));
    (void) enc_file;

    data->len_index = 0;

    for (n = 0; n < builtIn->length; ++n) {
	data->table_utf8[n].ucs = (unsigned) n;
    }

    for (n = 0; n < builtIn->length; ++n) {
	if (builtIn->table[n].source < data->table_size) {
	    size_t j = builtIn->table[n].source;

	    data->table_utf8[j].ucs = builtIn->table[n].target;

	    if ((need = (size_t) ConvToUTF8(buffer,
					    data->table_utf8[j].ucs,
					    sizeof(buffer) - 1)) != 0) {
		data->table_utf8[j].text = malloc(need + 1);
		data->table_utf8[j].size = need;
		memcpy(data->table_utf8[j].text, buffer, need);
	    }

	    trace_convert(data, j, 0);

	    data->rev_index[data->len_index].ucs = data->table_utf8[j].ucs;
	    data->rev_index[data->len_index].ch = (unsigned) j;
	    data->len_index++;
	}
    }
}

static const BuiltInCharsetRec *
findBuiltinEncoding(const char *encoding_name)
{
    size_t n;
    const BuiltInCharsetRec *result = NULL;

    for (n = 0; builtin_encodings[n].name != NULL; ++n) {
	if (!lcStrCmp(encoding_name, builtin_encodings[n].name)) {
	    result = &(builtin_encodings[n]);
	    break;
	}
    }

    return result;
}

/******************************************************************************/
LuitConv *
luitLookupEncoding(FontMapPtr mapping)
{
    LuitConv *latest;
    LuitConv *result = NULL;
    for (latest = all_conversions; latest != NULL; latest = latest->next) {
	if (&(latest->mapping) == mapping) {
	    result = latest;
	}
    }
    return result;
}

/*
 * Provide all of the data, needed for -show-iconv option to construct a ".enc"
 * representation.
 */
FontEncPtr
luitGetFontEnc(const char *name, UM_MODE mode)
{
    FontEncPtr result = NULL;
    FontMapPtr mp = NULL;
    FontMapPtr mp2 = NULL;
    FontEncSimpleMapPtr mq = NULL;
    UCode *map = NULL;
    LuitConv *lc;
    int n;

    if ((mp = luitLookupMapping(name, mode, usANY)) != NULL
	&& (lc = luitLookupEncoding(mp)) != NULL
	&& (mp2 = TypeCalloc(FontMapRec)) != NULL
	&& (mq = TypeCalloc(FontEncSimpleMapRec)) != NULL
	&& (map = TypeCallocN(UCode, lc->table_size)) != NULL
	&& (result = TypeCalloc(FontEncRec)) != NULL) {
	int max_chr = (MIN_UCODE - 1);
	int min_chr = (MAX_UCODE + 1);

	result->name = strmalloc(name);
	result->size = (int) lc->table_size;
	result->mappings = mp2;

	*mp2 = *mp;
	mp2->client_data = mq;
	mp2->next = NULL;

	mq->len = (unsigned) lc->table_size;
	mq->map = map;

	for (n = 0; n < (int) lc->table_size; ++n) {
	    unsigned ch = lc->rev_index[n].ch;
	    if (ch < mq->len) {
		map[ch] = (UCode) lc->rev_index[n].ucs;
		if (ch != lc->rev_index[n].ucs) {
		    if ((int) ch < min_chr)
			min_chr = (int) ch;
		    if ((int) ch > max_chr)
			max_chr = (int) ch;
		}
	    }
	}
	result->size = max_chr + 1;
	if (result->size > 256)
	    result->first = min_chr;
	else
	    result->size = 256;
    } else {
	if (mp2)
	    free(mp2);
	if (mq)
	    free(mq);
	if (map)
	    free(map);
    }
    return result;
}

/*
 * Free the data allocated in luitGetFontEnc().
 */
void
luitFreeFontEnc(FontEncPtr data)
{
    if (data != NULL) {
	FontMapPtr mp;
	FontEncSimpleMapPtr mq;

	if ((mp = data->mappings) != NULL) {
	    if ((mq = mp->client_data) != NULL) {
		free(mq->map);
		free(mq);
	    }
	    free(mp);
	}
	free(data->name);
	free(data);
    }
}

static void
finishIconvTable(LuitConv * latest)
{
    latest->next = all_conversions;
    latest->mapping.type = FONT_ENCODING_UNICODE;
    latest->mapping.recode = luitRecode;
    latest->reverse.reverse = luitReverse;
    latest->reverse.data = latest;
    all_conversions = latest;
    TRACE(("...finished LuitConv table for \"%s\"\n", NonNull(latest->encoding_name)));
}

static FontMapPtr
initLuitConv(const char *encoding_name,
	     iconv_t my_desc,
	     const BuiltInCharsetRec * builtIn,
	     int enc_file,
	     US_SIZE size)
{
    FontMapPtr result = NULL;
    LuitConv *latest;
    unsigned fast;
    size_t length = MAX8;

    switch (size) {
    case us8BIT:
	length = MAX8;
	break;
    case us16BIT:
	length = MAX16;
	break;
    default:
	if (builtIn) {
	    if (builtIn->length != 0) {
		size_t n;
		for (n = 0; n < builtIn->length; ++n) {
		    if (builtIn->table[n].source >= MAX8) {
			length = MAX16;
			break;
		    }
		}
	    } else {
		length = MAX16;
	    }
	} else if ((fast = count8bitIconv(my_desc)) < 256) {
	    length = sizeofIconvTable(encoding_name, fast);
	}
	break;
    }

    TRACE(("initLuitConv(%s) %u\n", NonNull(encoding_name), (unsigned) length));
    if ((latest = newLuitConv(length)) != NULL) {
	latest->encoding_name = strmalloc(encoding_name);
	latest->iconv_desc = my_desc;
	if (builtIn != NULL) {
	    initializeBuiltInTable(latest, builtIn, enc_file);
	} else if (length == MAX16) {
	    initialize16bitTable(latest->encoding_name, &latest, 1);
	} else {
	    initialize8bitTable(latest);
	}
	finishIconvTable(latest);
	result = &(latest->mapping);

	/* sort the reverse-index, to allow using bsearch */
	if (result != NULL) {
	    qsort(latest->rev_index,
		  latest->len_index,
		  sizeof(latest->rev_index[0]),
		  cmp_rindex);
	}
    }
    return result;
}

/*
 * This uses as input the data loaded from an external ".enc" file to construct
 * a table that initLuitConv() can use.
 */
static FontMapPtr
convertFontEnc(FontEncPtr fontenc)
{
    FontMapPtr mp = fontenc->mappings;
    FontEncSimpleMapPtr mq;
    BuiltInMapping *mapping;
    FontMapPtr result = NULL;

    TRACE(("convertFontEnc: %s\n", NonNull(fontenc->name)));

    while (mp != NULL) {
	if (mp->type == FONT_ENCODING_UNICODE)
	    break;
	mp = mp->next;
    }

    if (mp != NULL
	&& (mq = mp->client_data) != NULL
	&& mq->len
	&& (mapping = TypeCallocN(BuiltInMapping, mq->len + 1)) != NULL) {
	unsigned j, k;
	BuiltInCharsetRec builtIn;
	US_SIZE size = ((fontenc->size <= 256 && fontenc->row_size == 0)
			? us8BIT
			: us16BIT);

	TRACE(("...found mapping for %d items (size:%d)\n", mq->len, size));

	memset(&builtIn, 0, sizeof(builtIn));
	builtIn.name = fontenc->name;
	builtIn.table = mapping;
	builtIn.length = mq->len;

	for (j = k = 0; j < mq->len; ++j) {
	    unsigned code = luitRecode(j, mq);
	    mapping[k].source = j;
	    mapping[k].target = code ? code : j;
	    ++k;
	}
	result = initLuitConv(fontenc->name, NO_ICONV, &builtIn, 1, size);
	free(mapping);
    }

    return result;
}

static int
knownCharset(const FontencCharsetRec * fc)
{
    int result = 0;
    if (!strcmp(fc->name, "ASCII"))
	result = 1;
    return result;
}

/*
 * Portable iconv provides an "EUC-JP" which combines the information for the
 * JIS-X encodings.  For this case we can deduce the separate encodings.  Do
 * that, and return true if successful.
 */
static int
loadCompositeCharset(iconv_t my_desc, const char *composite_name)
{
    LuitConv *work[4];
    unsigned g;
    unsigned gmax = 0;
    unsigned csize = 0;

    /*
     * This is the first time we have tried for the composite.  Make
     * a list of the parts, first.
     */
    memset(work, 0, sizeof(work));
    for (g = 0; g < 4; ++g) {
	const FontencCharsetRec *fc = getCompositePart(composite_name, g);
	work[g] = NULL;
	if (fc != NULL && !knownCharset(fc)) {
	    TRACE(("part %d:%s (%s)\n", g, NonNull(fc->name), NonNull(fc->xlfd)));

	    switch (fc->type) {
	    case T_94:
	    case T_96:
	    case T_128:
		csize = MAX8;
		break;
	    case T_9494:
	    case T_9696:
	    case T_94192:
		csize = MAX16;
		break;
	    default:
		TRACE(("...ignoring for now\n"));
		break;
	    }

	    if ((work[g] = newLuitConv((size_t) csize)) == NULL)
		break;
	    work[g]->encoding_name = strmalloc(fc->name);
	    work[g]->iconv_desc = my_desc;
	    gmax = g + 1;
	}
    }

    /*
     * Now, load the charset, filling out the appropriate forward mapping
     * in each one according to the shift-information embedded in the
     * reverse mapping string.
     */
    initialize16bitTable(composite_name, work, gmax);
    /*
     * Finally, link the parts into the list of loaded charsets so we
     * will not repeat this process.
     */
    for (g = 0; g < 4; ++g) {
	if (work[g] != NULL) {
	    work[g]->iconv_desc = NO_ICONV;
	    finishIconvTable(work[g]);
	}
    }
    return 0;
}

static FontMapPtr
getFontMapByName(const char *encoding_name)
{
    FontMapPtr result = NULL;
    LuitConv *latest;

    for (latest = all_conversions; latest != NULL; latest = latest->next) {
	if (!lcStrCmp(encoding_name, latest->encoding_name)) {
	    result = &(latest->mapping);
	    break;
	}
    }
    TRACE(("getFontMapByName(%s) %s\n", NonNull(encoding_name),
	   result ? "OK" : "FAIL"));
    return result;
}

static FontMapPtr
lookupIconv(const char **encoding_name, char **aliased, US_SIZE size)
{
    LuitConv *latest;
    FontMapPtr result = NULL;
    iconv_t my_desc;
    iconv_t check;
    const FontencCharsetRec *fc;
    const char *alias;
    const char *full;

    my_desc = try_iconv_open(*encoding_name, aliased);
    if (my_desc == NO_ICONV
	&& (alias = findEncodingAlias(*encoding_name)) != NULL) {
	if (alias != NULL) {
	    if (*aliased) {
		free(*aliased);
		*aliased = NULL;
	    }
	    *encoding_name = alias;
	    TRACE(("...retry '%s'\n", NonNull(*encoding_name)));
	    my_desc = try_iconv_open(*encoding_name, aliased);
	}
    }
    if (my_desc != NO_ICONV) {
	TRACE(("...iconv_open succeeded\n"));
	result = initLuitConv(*encoding_name, my_desc, NULL, -1, size);
	iconv_close(my_desc);
	if ((latest = luitLookupEncoding(result)) != NULL) {
	    latest->iconv_desc = NO_ICONV;
	}
    } else if ((full = getCompositeCharset(*encoding_name)) != NULL
	       && (check = try_iconv_open(full, aliased)) != NO_ICONV) {
	loadCompositeCharset(check, full);
	iconv_close(check);
	if ((fc = getFontencByName(*encoding_name)) != NULL) {
	    result = getFontMapByName(fc->name);
	}
    }
    return result;
}

FontMapPtr
luitLookupMapping(const char *encoding_name, UM_MODE mode, US_SIZE size)
{
    FontMapPtr result = NULL;
    FontEncPtr fontenc;
    const BuiltInCharsetRec *builtIn;
    char *aliased = NULL;

    TRACE(("luitLookupMapping '%s' mode %u size %u\n",
	   NonNull(encoding_name), mode, size));

    if ((result = getFontMapByName(encoding_name)) != NULL) {
	TRACE(("...found in cache\n"));
    } else {
	int n;

	for (n = 0; lookup_order[n] != umNONE; ++n) {
	    if (!(mode & lookup_order[n]))
		continue;
	    switch (lookup_order[n]) {
	    case umICONV:
		result = lookupIconv(&encoding_name, &aliased, size);
		if (result != NULL) {
		    TRACE(("...lookupIconv succeeded\n"));
		}
		break;
	    case umFONTENC:
		if ((fontenc = lookupOneFontenc(encoding_name)) != NULL) {
		    result = convertFontEnc(fontenc);
		    if (result != NULL) {
			TRACE(("...convertFontEnc succeeded\n"));
		    }
		}
		break;
	    case umBUILTIN:
		if ((builtIn = findBuiltinEncoding(encoding_name)) != NULL) {
		    TRACE(("...use built-in charset\n"));
		    result = initLuitConv(encoding_name,
					  NO_ICONV,
					  builtIn,
					  0,
					  us8BIT);
		}
		break;
	    case umPOSIX:
		{
		    unsigned ch;
		    BuiltInMapping mapping[MAX8];
		    BuiltInCharsetRec posix;

		    TRACE(("...fallback to POSIX\n"));
		    memset(&posix, 0, sizeof(posix));
		    posix.name = encoding_name;
		    posix.length = SizeOf(mapping);
		    posix.table = mapping;
		    for (ch = 0; ch < posix.length; ++ch) {
			mapping[ch].source = ch;
			mapping[ch].target = (ch < 128) ? ch : 0;
		    }
		    result = initLuitConv(encoding_name,
					  NO_ICONV,
					  &posix,
					  0,
					  us8BIT);
		}
		break;
	    default:
		break;
	    }
	    if (result != NULL)
		break;
	}
    }
    if (aliased) {
	free(aliased);
    }

    TRACE(("...luitLookupMapping ->%p\n", (void *) result));
    return result;
}

FontMapReversePtr
luitLookupReverse(FontMapPtr fontmap_ptr)
{
    FontMapReversePtr result = NULL;
    LuitConv *search;

    TRACE(("luitLookupReverse %p\n", (void *) fontmap_ptr));
    for (search = all_conversions; search != NULL; search = search->next) {
	if (fontmap_ptr == &(search->mapping)) {
	    TRACE(("...found %s\n", NonNull(search->encoding_name)));
	    result = &(search->reverse);
	    break;
	}
    }
    return result;
}

unsigned
luitMapCodeValue(unsigned code, FontMapPtr fontmap_ptr)
{
    unsigned result;
    LuitConv *search;

    result = code;
    for (search = all_conversions; search != NULL; search = search->next) {
	if (&(search->mapping) == fontmap_ptr) {
	    if (code < search->table_size) {
		result = search->table_utf8[code].ucs;
		if (result == 0 && code != 0)
		    result = code;
	    }
	    break;
	}
    }

    TRACE2(("luitMapCodeValue 0x%04X '%c' 0x%04X\n",
	    code,
	    isprint(code) ? code : ' ',
	    result));
    return result;
}

#if defined(HAVE_LANGINFO_CODESET)
typedef struct {
    char *encoding;
    char **locales;
    size_t length;		/* amount used in locales[] */
    size_t actual;		/* allocated size of locales[] */
} CODESET_LOCALE;

static int
compare_encodings(const void *a, const void *b)
{
    const CODESET_LOCALE *p = a;
    const CODESET_LOCALE *q = b;
    return strcmp(p->encoding, q->encoding);
}

static int
compare_locales(const void *a, const void *b)
{
    const char *const *p = a;
    const char *const *q = b;
    return strcmp(*p, *q);
}
#endif

/*
 * Report a list of the built-in encodings (used for fallback)
 */
int
reportBuiltinCharsets(void)
{
    size_t j, k;

    printf("These encodings are used if iconv/fontenc data are missing:\n");
    printf("\n");
    for (j = 0; builtin_encodings[j].name != NULL; ++j) {
	const BuiltInCharsetRec *p = &(builtin_encodings[j]);
	unsigned lo = p->table[0].source;
	unsigned hi = lo;

	printf("%s\n", builtin_encodings[j].name);
	for (k = 1; k < p->length; ++k) {
	    if (lo > p->table[k].source)
		lo = p->table[k].source;
	    if (hi < p->table[k].source)
		hi = p->table[k].source;
	}
	printf("\tData: [%04X..%04X] defined %u\n",
	       lo, hi, (unsigned) p->length);
    }
    return EXIT_SUCCESS;
}

/*
 * Obtain a list of supported locales, and for each obtain the corresponding
 * charset.
 */
int
reportIconvCharsets(void)
{
#if !defined(HAVE_LANGINFO_CODESET)
    int rc = EXIT_FAILURE;

    Message("nl_langinfo(CODESET) not supported\n");
#else
    int rc = EXIT_SUCCESS;

    FILE *fp;
    char *old_locale;
    char **allLs = NULL;
    size_t numLs = 0;
    size_t useLs = 0;

    /* save our current locale */
    if ((old_locale = setlocale(LC_CTYPE, NULL)) != NULL)
	old_locale = strmalloc(old_locale);

    /*
     * next, obtain the list of locales.  Their order does not matter.
     */
    if ((fp = popen("locale -a", "r")) != NULL) {
	char buffer[BUFSIZ];
	char *s;
	while (fgets(buffer, (int) sizeof(buffer) - 1, fp) != NULL) {
	    s = buffer + strlen(buffer);
	    while (s != buffer) {
		--s;
		if (isspace(UChar(*s)))
		    *s = '\0';
		else
		    break;
	    }
	    if ((long) useLs >= (long) numLs) {
		numLs = (useLs + 32) * 3 / 2;
		allLs = realloc(allLs, (numLs + 2) * sizeof(*allLs));
		if (allLs == NULL) {
		    FatalError("Couldn't grow allLs array.\n");
		}
	    }
	    allLs[useLs++] = strmalloc(buffer);
	    allLs[useLs] = NULL;
	}
	pclose(fp);
    }

    /* now, for each locale, set our locale to that and ask for the charset */
    if (allLs != NULL) {
	int j, k, enc, loc;
	char *resolved;
	CODESET_LOCALE *allEs = NULL;
	size_t n, t, col, now;
	size_t useEs = 0;
	size_t numEs = 0;

	for (j = 0; allLs[j] != NULL; ++j) {
	    setlocale(LC_CTYPE, allLs[j]);
	    if ((resolved = nl_langinfo(CODESET)) != NULL) {
		enc = -1;
		for (k = 0; k < (int) useEs; ++k) {
		    if (!strcmp(resolved, allEs[k].encoding)) {
			enc = k;
			break;
		    }
		}
		if (enc < 0) {
		    if ((long) useEs >= (long) numEs) {
			numEs = (useEs + 32) * 3 / 2;
			allEs = realloc(allEs, numEs * sizeof(*allEs));
			if (allEs == NULL) {
			    FatalError("Couldn't grow allEs array.\n");
			}
		    }
		    allEs[useEs].encoding = strmalloc(resolved);
		    allEs[useEs].locales = NULL;
		    allEs[useEs].actual = 0;
		    allEs[useEs].length = 0;
		    enc = (int) useEs;
		    ++useEs;
		}
		loc = -1;
		for (k = 0; k < (int) allEs[enc].length; ++k) {
		    if (!strcmp(allLs[j], allEs[enc].locales[k])) {
			loc = k;
			break;
		    }
		}
		if (loc < 0) {
		    if ((long) allEs[enc].length >=
			(long) allEs[enc].actual) {
			allEs[enc].actual = ((allEs[enc].length
					      + 32) * 3 / 2);
			allEs[enc].locales = realloc(allEs[enc].locales,
						     (allEs[enc].actual + 2)
						     * sizeof(char *));
		    }
		    loc = (int) allEs[enc].length;
		    allEs[enc].length++;
		}
		allEs[enc].locales[loc] = allLs[j];
		allEs[enc].locales[loc + 1] = NULL;
	    }
	}

	/* set our locale back to the starting one */
	setlocale(LC_CTYPE, old_locale);

	/* print the result */
	if (useEs != 0) {
	    printf("Iconv supports %ld encodings\n", (long) useEs);
	    qsort(allEs, useEs, sizeof(allEs[0]), compare_encodings);
	    for (n = 0; n < useEs; ++n) {
		printf("  %s\n", allEs[n].encoding);
		qsort(allEs[n].locales, allEs[n].length, sizeof(char *), compare_locales);
		col = 0;
		for (t = 0; t < allEs[n].length; ++t) {
		    now = strlen(allEs[n].locales[t]);
		    if (col + now + 2 > MAXCOLS) {
			printf("\n");
			col = 0;
		    }
		    if (col == 0) {
			printf("    ");
			col = 4;
		    }
		    printf("%s", allEs[n].locales[t]);
		    col += now;
		    if (t + 1 < allEs[n].length) {
			printf(", ");
			col += 2;
		    }
		}
		if (col)
		    printf("\n");
	    }
#ifdef NO_LEAKS
	    for (n = 0; n < useEs; ++n) {
		free(allEs[n].encoding);
		free(allEs[n].locales);
	    }
	    free(allEs);
#endif
	} else {
	    Message("No encodings found\n");
	    rc = EXIT_FAILURE;
	}
#ifdef NO_LEAKS
	for (n = 0; n < useLs; ++n) {
	    free(allLs[n]);
	}
	free(allLs);
#endif
    } else {
	Message("No locales found\n");
	rc = EXIT_FAILURE;
    }

    /* cleanup */
    free(old_locale);
#endif
    return rc;
}

#ifdef NO_LEAKS
/*
 * Given a reverse-pointer, remove all of the corresponding cached information
 * from this module.
 */
void
luitDestroyReverse(FontMapReversePtr reverse)
{
    LuitConv *p, *q;
    size_t n;

    for (p = all_conversions, q = NULL; p != NULL; q = p, p = p->next) {
	if (&(p->reverse) == reverse) {

	    free(p->encoding_name);
	    if (p->iconv_desc != NO_ICONV)
		iconv_close(p->iconv_desc);

	    for (n = 0; n < p->table_size; ++n) {
		if (p->table_utf8[n].text) {
		    free(p->table_utf8[n].text);
		}
	    }

	    /* delink and destroy */
	    if (q != NULL)
		q->next = p->next;
	    else
		all_conversions = p->next;
	    free(p->table_utf8);
	    free(p->rev_index);
	    free(p);
	    break;
	}
    }
}

void
luitconv_leaks(void)
{
    TRACE(("luitconv_leaks %p\n", (void *) all_conversions));
    while (all_conversions != NULL) {
	luitDestroyReverse(&(all_conversions->reverse));
    }
}
#endif
