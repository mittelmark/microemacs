/* $XTermId: fontenc.c,v 1.102 2025/09/12 08:24:47 tom Exp $ */

/*
Copyright 2013-2024,2025 by Thomas E. Dickey

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

#include <other.h>
#include <sys.h>

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#define flatUCode(row,col) (((row) << 8) + (col))

typedef enum {
    ftUnknown
    ,ftComment
    ,ftDefine
    ,ftStartEncoding
    ,ftStartMapping
    ,ftEndMapping
    ,ftEndEncoding
    ,ftFirstIndex
    ,ftSize
    ,ftUndefine
    ,ftAlias
} FONTENC_TYPES;

/*
 * The fontenc library is single-use (luit-only) and undocumented (read the
 * source code).  Rather than incorporate it directly here, this module adapts
 * as needed, and replaces where fontenc is unsuitable for reuse.
 */
#ifdef USE_FONTENC
extern FontEncPtr FontEncReallyLoad(const char *charset, const char *fontFileName);
#endif

/*
 * fontenc uses FontEncDirectory to find the "encodings.dir" file.  It uses a
 * compiled-in default value which can be overridden by an environment
 * variable.
 */
#ifndef USE_FONTENC
extern char *FontEncDirectory(void);

char *
FontEncDirectory(void)
{
    static char default_dir[] = ENCODINGS_DIR_FILE;
    static char *dir = NULL;

    if (dir == NULL) {
	char *c = getenv("FONT_ENCODINGS_DIRECTORY");
	if (c) {
	    dir = strmalloc(c);
	    if (!dir)
		return NULL;
	} else if (default_dir[0] != '\0') {
	    dir = default_dir;
	}
    }
    return dir;
}
#endif /* !USE_FONTENC */

/*
 * fontenc uses an "encodings.dir" file which can appear in the X fonts tree in
 * more than one place (because the file is created as a side-effect of
 * mkfontdir).
 *
 * The "encodings.dir" file has a count of the number of records at the start.
 * Subsequent lines contain an alias and a pathname (for a ".enc" file).  The
 * pathname can be either absolute or relative to the directory containing
 * "encodings.dir".
 *
 * The ".enc" files typically are gzip'd (fontenc is hardcoded to expect this,
 * and has been known to dump core when a stray uncompressed file is put into
 * the tree).  The alias in "encodings.dir" is obtained from the first line
 * of the ".enc" file, and may be unrelated to the actual filename.
 */
typedef struct {
    int used;
    char *alias;
    char *path;
    FontEncPtr data;
} ENCODINGS_DIR;

static ENCODINGS_DIR *encodings_dir;

/*
 * Read a buffer, trimming trailing whitespace (unlike getline, which in turn
 * is not portable enough in any case).
 */
static char *
getFileBuffer(char **bufferp, size_t *lengthp, FILE *fp)
{
    size_t used = 0;
    char extra[BUFSIZ];
    int finished = 1;

    if (*bufferp != NULL)
	**bufferp = '\0';

    while (fgets(extra, (int) sizeof(extra) - 1, fp)) {
	size_t have = strlen(extra);
	int found = 0;
	finished = 0;
	while (have != 0) {
	    --have;
	    if (extra[have] == '\n') {
		found = 1;
	    }
	    if (isspace(UChar(extra[have]))) {
		extra[have] = '\0';
	    } else {
		break;
	    }
	}
	if (have) {
	    size_t need = have + used + 2;
	    if (*lengthp < need) {
		char *next = realloc(*bufferp, need);
		if (next == NULL) {
		    free(*bufferp);
		    *bufferp = NULL;
		    return NULL;
		}
		*bufferp = next;
		*lengthp = need;
	    } else if (*bufferp == NULL) {
		return NULL;
	    }
	    strcpy(*bufferp + used, extra);
	    used += have;
	}
	if (found) {
	    break;
	}
    }

    return !finished ? *bufferp : NULL;
}

#ifndef USE_FONTENC
#ifdef USE_ZLIB
/*
 * Read a buffer, trimming trailing whitespace (unlike getline, which in turn
 * is not portable enough in any case).
 */
static char *
getGzipBuffer(char **bufferp, size_t *lengthp, gzFile fp)
{
    size_t used = 0;
    char extra[BUFSIZ];
    int finished = 1;

    if (*bufferp != NULL)
	**bufferp = '\0';

    while (gzgets(fp, extra, (int) sizeof(extra) - 1)) {
	size_t have = strlen(extra);
	int found = 0;
	finished = 0;
	while (have != 0) {
	    --have;
	    if (extra[have] == '\n') {
		found = 1;
	    }
	    if (isspace(UChar(extra[have]))) {
		extra[have] = '\0';
	    } else {
		break;
	    }
	}
	if (have) {
	    size_t need = have + used + 2;
	    if (*lengthp < need) {
		*bufferp = realloc(*bufferp, need);
		*lengthp = need;
	    }
	    strcpy(*bufferp + used, extra);
	    used += have;
	}
	if (found) {
	    break;
	}
    }

    return !finished ? *bufferp : NULL;
}
#endif /* USE_ZLIB */
#endif /* !USE_FONTENC */

static char *
absolutePath(char *given, const char *refPath)
{
    char *result;
    char *leaf;

    if (*given != '/') {
	if (!strncmp(given, "./", (size_t) 2)) {
	    given += 2;
	}
	result = malloc(strlen(refPath) + strlen(given) + 2);
	if (result != NULL) {
	    strcpy(result, refPath);
	    if ((leaf = strrchr(result, '/')) == NULL) {
		leaf = result + strlen(result);
		*leaf++ = '/';
	    }
	    strcpy(leaf + 1, given);
	} else {
	    FatalError("cannot allocate absolute path for %s\n", given);
	}
    } else {
	result = strmalloc(given);
    }
    return result;
}

static int
compare_aliases(const void *a, const void *b)
{
    const ENCODINGS_DIR *p = a;
    const ENCODINGS_DIR *q = b;
    return strcmp(p->alias, q->alias);
}

static char *
skipToWhite(char *value)
{
    char *result = NULL;
    while (*value != '\0') {
	if (isspace(UChar(*value))) {
	    result = value;
	    break;
	}
	++value;
    }
    return result;
}

static void
loadEncodingsDir(void)
{
    if (encodings_dir == NULL) {
	FILE *fp;
	const char *path = FontEncDirectory();
	char *buffer = NULL;
	size_t entry = 0;
	size_t length = 0;
	size_t entries = 0;
	char *value;
	int used = 0;
	int n, found;
	int row = 0;

	if (path == NULL) {
	    TRACE(("cannot find encodings.dir\n"));
	} else if ((fp = fopen(path, "r")) == NULL) {
	    FatalError("cannot open %s\n", path);
	} else {
	    while (getFileBuffer(&buffer, &length, fp) != NULL) {
		++row;
		if (*buffer == '\0')
		    continue;
		if (encodings_dir == NULL) {
		    char *next = NULL;
		    long count = strtol(buffer, &next, 10);
		    if (count <= 0) {
			FatalError("found no count in %s\n", path);
		    }
		    entries = (size_t) count;
		    encodings_dir = calloc(entries + 1, sizeof(*encodings_dir));
		    if (encodings_dir == NULL) {
			FatalError("cannot allocate %ld encodings\n", count);
		    }
		} else if ((value = skipToWhite(buffer)) != NULL) {
		    *value++ = '\0';
		    /* get rid of duplicates - they do occur */
		    for (n = found = 0; n < used; ++n) {
			if (!strcmp(buffer, encodings_dir[n].alias)) {
			    found = 1;
			    break;
			}
		    }
		    if (!found) {
			encodings_dir[used].alias = strmalloc(buffer);
			encodings_dir[used].path = absolutePath(value, path);
			++used;
		    }

		    if (++entry >= entries)
			break;
		} else {
		    FatalError("incorrect format of line %d:%s\n", row, buffer);
		}
	    }
	    fclose(fp);
	    if (used > 1) {
		qsort(encodings_dir,
		      (size_t) used,
		      sizeof(encodings_dir[0]),
		      compare_aliases);
	    }
	}
	free(buffer);
    }
}

#ifndef USE_FONTENC
/*
 * The fontenc library attempts to fill in the one-one mapping in the setCode
 * function, but it is inconsistently applied and the loaded map contains gaps.
 * This function sets one-one for any codes which map to NUL.
 */
static void
fillCharMap(FontEncSimpleMapPtr mp, int size)
{
    int n;

    if (mp != NULL && mp->map != NULL) {
	for (n = 0; n < size; ++n) {
	    if (mp->map[n] == 0) {
		mp->map[n] = (UCode) (n + mp->first);
	    }
	}
    }
}

/*
 * This function does the opposite of fillCharMap(), replacing one-one mappings
 * with a NUL.
 */
static void
trimCharMap(FontEncSimpleMapPtr mp, int size)
{
    int n;

    if (mp != NULL && mp->map != NULL) {
	for (n = 0; n < size; ++n) {
	    if (mp->map[n] == (UCode) (n + mp->first)) {
		mp->map[n] = 0;
	    }
	}
    }
}

static void
trim_or_fill(FontEncSimpleMapPtr mp, int size)
{
    if (fill_fontenc) {
	fillCharMap(mp, size);
    } else {
	trimCharMap(mp, size);
    }
}
#else /* USE_FONTENC */
#define fillCharMap(mp, size)	/* nothing */
#define trimCharMap(mp, size)	/* nothing */
#define trim_or_fill(mp, size)	/* nothing */
#endif /* !USE_FONTENC */

static size_t
fontencSize(FontEncPtr enc)
{
    size_t result = (size_t) (enc->size ? enc->size : 256);
    if (enc->row_size)
	result = (result) * 256;
    return result;
}

#ifndef USE_FONTENC
#ifdef USE_ZLIB
static char *
skipBlanks(char *value)
{
    while (isspace(UChar(*value)))
	++value;
    return value;
}

static char *
skipNonblanks(char *value)
{
    while (*value != '\0' && !isspace(UChar(*value)))
	++value;
    return value;
}

static char *
allocateToken(char *value)
{
    char *result;
    char *next;
    char save;

    value = skipBlanks(value);
    next = skipNonblanks(value);
    save = *next;
    *next = '\0';
    result = strmalloc(value);
    *next = save;

    return result;
}

static int
compareKeyword(char *value, const char *expected)
{
    int result;
    char save;
    char *next;

    next = skipNonblanks(value);
    save = *next;
    *next = '\0';

    result = StrCaseCmp(value, expected);

    *next = save;
    return result;
}

static FONTENC_TYPES
getLineType(char *line, char **nextp)
{
    FONTENC_TYPES result = ftUnknown;
    char save;

    line = skipBlanks(line);
    *nextp = skipNonblanks(line);
    save = **nextp;
    **nextp = '\0';

    if (*line == '\0' || *line == '#') {
	result = ftComment;
    } else if (isdigit(UChar(*line))) {
	result = ftDefine;
    } else if (!StrCaseCmp(line, "ALIAS")) {
	result = ftAlias;
    } else if (!StrCaseCmp(line, "STARTENCODING")) {
	result = ftStartEncoding;
    } else if (!StrCaseCmp(line, "STARTMAPPING")) {
	result = ftStartMapping;
    } else if (!StrCaseCmp(line, "ENDENCODING")) {
	result = ftEndEncoding;
    } else if (!StrCaseCmp(line, "ENDMAPPING")) {
	result = ftEndMapping;
    } else if (!StrCaseCmp(line, "SIZE")) {
	result = ftSize;
    } else if (!StrCaseCmp(line, "FIRSTINDEX")) {
	result = ftFirstIndex;
    } else if (!StrCaseCmp(line, "UNDEFINE")) {
	result = ftUndefine;
    }
    **nextp = save;
    return result;
}

#define MAX_NUMBERS 4

static int
getNumbers(char *line, int *results)
{
    int count = 0;
    char *next;
    long value;

    while (count + 1 < MAX_NUMBERS) {
	line = skipBlanks(line);
	value = strtol(line, &next, 0);
	if (next == line)
	    break;
	results[count++] = (int) value;
	line = next;
    }

    return count;
}

static int
fontencFirst(FontEncPtr enc)
{
    int result;

    if (enc->row_size) {
	result = (enc->row_size * enc->first) + enc->first_col;
    } else {
	result = enc->first;
    }
    return result;
}

static int
fontencIndex(FontEncPtr enc, unsigned from)
{
    int result;

    if ((int) from < MIN_UCODE) {
	result = -1;
    } else if (from <= MAX_UCODE) {
	if (enc->row_size == 0) {
	    result = (int) from;
	} else {
	    int row = (int) rowOf(from);
	    int col = (int) colOf(from);

	    if (col >= enc->row_size) {
		result = -1;
	    } else {
		result = (row * enc->row_size) + col;
	    }
	}
    } else {
	result = -1;
    }
    return result;
}

static void
defineCode(FontEncPtr enc, int from, int to)
{
    int inx = fontencIndex(enc, (unsigned) from);

    if (inx >= 0 && enc->mappings != NULL) {
	int first = fontencFirst(enc);
	FontEncSimpleMapPtr data = enc->mappings->client_data;
	int limit = (int) fontencSize(enc);

	if (inx >= first && (unsigned) inx < data->len && inx < limit) {
	    data->map[inx - first] = (UCode) to;
	}
    }
}
#endif /* USE_ZLIB */
#endif /* !USE_FONTENC */

/*
 * fontenc uses this indexing scheme to reduce tablesize.
 * The function returns a valid index into the forward map to Unicode;
 */
static int
fontencUnmap(FontEncSimpleMapPtr map, int from)
{
    int result;

    if (map != NULL) {
	if (map->row_size) {
	    from += map->first;
	    if (map->row_size == 0) {
		result = from;
	    } else {
		int row = (from / (int) map->row_size);
		int col = (from % (int) map->row_size);
		result = (row * 256) + col;
	    }
	} else {
	    result = from + map->first;
	}

	if (result < MIN_UCODE) {
	    result = -1;
	} else if (result >= MAX_UCODE) {
	    result = -1;
	}
    } else {
	result = -1;
    }
    return result;
}

unsigned
luitRecode(unsigned code, void *client_data)
{
    unsigned result;
    FontEncSimpleMapPtr map;
    unsigned inx;

    map = client_data;

    if (code >= MAX_UCODE) {
	result = 0;
    } else {
	unsigned col = (unsigned) (map->row_size ? colOf(code) : 0);

	if (map->row_size && col >= map->row_size) {
	    result = 0;
	} else {
	    if (map->row_size) {
		inx = col + (unsigned) rowOf(code) * map->row_size;
	    } else {
		inx = code;
	    }

	    if (map->map
		&& inx >= map->first
		&& inx < map->first + map->len) {
		result = map->map[inx - map->first];
	    } else {
		result = code;
	    }
	}
    }
    return result;
}

#define UpdateLoChar(value) \
	    if (lo_char < 0) { \
		lo_char = value; \
	    } else if (lo_char > value) { \
		lo_char = value; \
	    }
#define UpdateHiChar(value) \
	    if (hi_char < 0) { \
		hi_char = value; \
	    } else if (hi_char < value) { \
		hi_char = value; \
	    }

/*
 * Read an encoding file, report summary statistics.
 */
static FontEncPtr
loadFontEncRec(const char *charset, const char *path)
{
    FontEncPtr result;
#if defined(USE_FONTENC)
    result = FontEncReallyLoad(charset, path);
#elif defined(USE_ZLIB)
    gzFile fp;
    char *buffer = NULL;
    size_t length = 0;
    size_t numAliases = 0;
    size_t result_size = 0;
    int numbers[MAX_NUMBERS];
    FontMapPtr mapping = NULL;
    int done = 0;

    (void) charset;

    if ((result = calloc((size_t) 1, sizeof(*result))) == NULL)
	return NULL;

    fp = gzopen(path, "r");
    if (fp != NULL) {
	int ignore = 0;
	while (!done && getGzipBuffer(&buffer, &length, fp)) {
	    char *later;
	    switch (getLineType(buffer, &later)) {
	    case ftComment:
		break;
	    case ftStartEncoding:
		/*
		 * Parameter should agree with "charset", but sometimes
		 * does not.
		 */
		result->name = allocateToken(later);
		result->size = 256;
		break;
	    case ftAlias:
		result->aliases = realloc(result->aliases,
					  (numAliases + 2) * sizeof(char *));
		result->aliases[numAliases++] = allocateToken(later);
		result->aliases[numAliases] = NULL;
		break;
	    case ftSize:
		/*
		 * A single parameter gives the highest value directly.
		 * Two parameters give number of rows and row size.
		 */
		switch (getNumbers(later, numbers)) {
		default:
		    result->row_size = numbers[1];
		    /* FALLTHRU */
		case 1:
		    result->size = numbers[0];
		    break;
		case 0:
		    break;
		}
		break;
	    case ftFirstIndex:
		/*
		 * Referring to SIZE, this gives the first defined code in
		 * the range.  Use one or two parameters together with the
		 * row size from the SIZE statement.
		 */
		switch (getNumbers(later, numbers)) {
		default:
		    result->first_col = numbers[1];
		    /* FALLTHRU */
		case 1:
		    result->first = numbers[0];
		    break;
		case 0:
		    break;
		}
		break;
	    case ftUndefine:
		/*
		 * Mark one or more codes within the overall range as
		 * undefined.  There is no mapping to unicode for those codes.
		 *
		 * One or two parameters are expected.  Two parameters specify
		 * a range of codes to undefine.
		 */
		if (!ignore) {
		    int code;
		    switch (getNumbers(later, numbers)) {
		    default:
			/* ignore */
			break;
		    case 2:
			for (code = numbers[0]; code <= numbers[1]; ++code) {
			    defineCode(result, code, 0);
			}
			break;
		    case 1:
			defineCode(result, numbers[0], 0);
			break;
		    }
		}
		break;
	    case ftDefine:
		if (!ignore) {
		    int from;
		    int to;

		    switch (getNumbers(buffer, numbers)) {
		    case 1:
			/* ignore */
			break;
		    case 2:
			defineCode(result, numbers[0], numbers[1]);
			break;
		    case 3:
			to = numbers[2];
			for (from = numbers[0]; from <= numbers[1]; ++from) {
			    defineCode(result, from, to++);
			}
			break;
		    }
		}
		break;
	    case ftStartMapping:
		/*
		 * The encoding can have more than one mapping, but we care
		 * only about the mapping to unicode.
		 */
		later = skipBlanks(later);
		if (!compareKeyword(later, "unicode")) {
		    FontEncSimpleMapPtr mq;

		    ignore = 0;
		    result_size = fontencSize(result);
		    mapping = TypeCalloc(FontMapRec);
		    if (mapping == NULL)
			FatalError("cannot allocate map record\n");
		    mapping->type = FONT_ENCODING_UNICODE;
		    mapping->recode = luitRecode;

		    if ((mq = TypeCalloc(FontEncSimpleMapRec)) == NULL
			|| (mq->map = TypeCallocN(UCode, result_size)) == NULL) {
			FatalError("cannot allocate map for %ld codes\n",
				   (long) result_size);
		    }

		    if (result->row_size) {
			mq->first = (UCode) flatUCode(result->first, result->first_col);
		    } else {
			mq->first = (UCode) result->first;
		    }

		    mq->len = (unsigned) fontencSize(result);
		    mq->first = (UCode) fontencFirst(result);
		    mq->row_size = (UCode) result->row_size;

		    mapping->client_data = mq;
		    result->mappings = mapping;
		} else {
		    ignore = 1;
		}
		break;
	    case ftEndMapping:
		ignore = 0;
		break;
	    case ftEndEncoding:
		done = 1;
		/* should be end of file */
		break;
	    default:
		if (!ignore)
		    printf("\t->%s\n", buffer);
		break;
	    }
	}
	gzclose(fp);
	free(buffer);
    }

    if (result->name == NULL)
	result->name = strmalloc(charset);
#else /* !USE_FONTENC && !USE_ZLIB */
    (void) charset;
    (void) path;
    result = 0;
#endif /* USE_FONTENC/USE_ZLIB */
    return result;
}

/*
 * Find an encoding, given its name.
 */
FontEncPtr
lookupOneFontenc(const char *name)
{
    int n;
    FontEncPtr result = NULL;

#ifdef USE_FONTENC
    result = FontEncFind(name, NULL);
    if (result == NULL)
#endif
    {
	loadEncodingsDir();
	if (encodings_dir != NULL) {
	    for (n = 0; encodings_dir[n].alias != NULL; ++n) {
		if (!StrCaseCmp(name, encodings_dir[n].alias)) {
		    if ((result = encodings_dir[n].data) == NULL
			&& encodings_dir[n].used == 0) {
			result = loadFontEncRec(encodings_dir[n].alias,
						encodings_dir[n].path);
			if (result == NULL) {
			    Warning("cannot load data for %s\n",
				    encodings_dir[n].path);
			} else {
			    VERBOSE(1, ("load alias \"%s\" from \"%s\"\n",
					encodings_dir[n].alias,
					encodings_dir[n].path));
			}
			encodings_dir[n].used = 1;
			encodings_dir[n].data = result;
		    }
		    break;
		}
	    }
	}
    }
    return result;
}

static const char *
fontmapTypename(int type)
{
    const char *result = "?";
    switch (type) {
#ifdef FONT_ENCODING_TRUETYPE
    case FONT_ENCODING_TRUETYPE:
	result = "cmap";
	break;
#endif
    case FONT_ENCODING_UNICODE:
	result = "unicode";
	break;
#ifdef FONT_ENCODING_POSTSCRIPT
    case FONT_ENCODING_POSTSCRIPT:
	result = "postscript";
	break;
#endif
    }
    return result;
}

static FontEncSimpleMapPtr
findUnicodeMapping(FontEncPtr data)
{
    FontEncSimpleMapPtr mq = NULL;
    FontMapPtr mp;

    for (mp = data->mappings; mp != NULL; mp = mp->next) {
	if (mp->type == FONT_ENCODING_UNICODE) {
	    mq = mp->client_data;
	    if (mq->map == NULL)
		mq = NULL;
	    break;
	}
    }
    return mq;
}

/*
 * Read an encoding file, report summary statistics.
 */
static FontEncPtr
reportOneFontenc(const char *alias, const char *path)
{
    FontEncPtr data = loadFontEncRec(alias, path);
    if (data != NULL) {
	int n;
	int lo_char = -1;
	int hi_char = -1;
	int num_def = 0;
	int inx;
	FontEncSimpleMapPtr mq;

	printf("\tName: %s\n", data->name);
	if (data->aliases) {
	    for (n = 0; data->aliases[n]; ++n) {
		printf("\t      %s\n", data->aliases[n]);
	    }
	}
	if (data->row_size) {
	    printf("\tSize: %d %d (%d)\n",
		   data->size,
		   data->row_size,
		   data->size * data->row_size);
	} else {
	    printf("\tSize: %d\n", data->size);
	}
	if (data->first_col) {
	    printf("\tBase: %04X\n", flatUCode(data->first, data->first_col));
	} else {
	    printf("\tBase: %04X\n", data->first);
	}
	mq = findUnicodeMapping(data);
	if (mq != NULL) {
	    trim_or_fill(mq, (int) fontencSize(data));
	    for (n = 0; n < (int) mq->len; ++n) {
		inx = fontencUnmap(mq, n);
		if (inx >= 0 && mq->map[n]) {
		    UpdateLoChar(inx);
		    UpdateHiChar(inx);
		    if (mq->map[n] != inx)
			num_def++;
		}
	    }
	    if (lo_char < 0) {
		lo_char = mq->first;
		hi_char = data->size - 1;
	    }
	    printf("\tData: [%04x..%04x] defined %d\n", lo_char, hi_char, num_def);
	}
    }
    return data;
}

/*
 * Make a report of the encoding files which could be loaded using either
 * the fontenc library (using its hardcoded tables) or dynamically via luit.
 */
int
reportFontencCharsets(void)
{
    int rc = EXIT_FAILURE;
    int n;

    printf("Available encodings listed in:\n\t%s\n", FontEncDirectory());
    loadEncodingsDir();
    if (encodings_dir != NULL) {
	for (n = 0; encodings_dir[n].alias != NULL; ++n) {
	    printf("%s\n\t%s\n",
		   encodings_dir[n].alias,
		   encodings_dir[n].path);
	    encodings_dir[n].data = reportOneFontenc(encodings_dir[n].alias,
						     encodings_dir[n].path);
	    if (encodings_dir[n].data != NULL) {
		rc = EXIT_SUCCESS;
	    }
	}
    }

    if (rc != EXIT_SUCCESS) {
	Warning("no encodings found\n");
    }
    return rc;
}

/*
 * Regurgitate an encoding from the in-memory copy.
 */
static int
showOneCharset(const char *name, FontEncPtr data)
{
    int rc = EXIT_FAILURE;

    if (data != NULL) {
	FontMapPtr mp;
	int n;

	printf("# %s\n", name);
	printf("STARTENCODING %s\n", data->name ? data->name : "unknown");

	if (data->aliases) {
	    for (n = 0; data->aliases[n]; ++n) {
		printf("ALIAS %s\n", data->aliases[n]);
	    }
	}

	if (data->row_size)
	    printf("SIZE %d %d\n", data->size, data->row_size);
	else if (data->size)
	    printf("SIZE %d\n", data->size);

	if (data->first_col)
	    printf("FIRSTINDEX %d %d\n", data->first, data->first_col);
	else if (data->first)
	    printf("FIRSTINDEX %d\n", data->first);

	for (mp = data->mappings; mp != NULL; mp = mp->next) {
	    printf("STARTMAPPING %s\n", fontmapTypename(mp->type));
	    if (mp->type == FONT_ENCODING_UNICODE) {
		int limit = (int) fontencSize(data);
		unsigned ch;

		if (mp->client_data == NULL)
		    printf("# no client_data-array\n");
		if (mp->recode == NULL) {
		    printf("# no recode-function\n");
		} else {
		    trim_or_fill(mp->client_data, (int) fontencSize(data));
		    for (n = 0; n < limit; ++n) {
			ch = mp->recode((unsigned) n, mp->client_data);
			if (ch || !n) {
			    if (!fill_fontenc && ((unsigned) n == ch))
				continue;
			    printf("0x%04X 0x%04X\n", n, ch);
			}
		    }
		}
	    }
	    printf("ENDMAPPING\n");
	}
	printf("# vile:tblmode\n");
	printf("ENDENCODING\n");
	rc = EXIT_SUCCESS;
    } else {
	Warning("no encoding data found for %s\n", name);
    }
    return rc;
}

int
showFontencCharset(const char *name)
{
    return showOneCharset(name, lookupOneFontenc(name));
}

/*
 * Returns 94, 96 or 128 for an 8-bit character-set, based on the mapping.
 */
int
typeOfFontenc(FontEncPtr enc)
{
    FontEncSimpleMapPtr mp = findUnicodeMapping(enc);
    int result = 0;

    if (mp != NULL) {
	int limit = (int) fontencSize(enc);
	int n;
	int hi = 0;
	int lo = limit;

	for (n = 0; n < limit; ++n) {
	    UCode target = mp->map[n - mp->first];
	    if (target != 0 && target != n) {
		if (n > hi)
		    hi = n;
		if (n < lo)
		    lo = n;
	    }
	}
	if (lo >= 128 && lo < 160) {
	    result = 128;
	} else if (lo >= 160 && hi < 256) {
	    result = 96;
	} else {
	    result = hi - lo + 1;
	}
	VERBOSE(2, ("character-set size %d\n", result));
    }
    return result;
}

/*
 * Returns 0x00 or 0x80 for an 8-bit character-set, according to where the
 * lowest mapping is done.
 */
unsigned
shiftOfFontenc(FontEncPtr enc)
{
    FontEncSimpleMapPtr mp = findUnicodeMapping(enc);
    unsigned result = 0x80;

    if (mp != NULL) {
	int limit = (int) fontencSize(enc);
	int n;

	for (n = 0; n < limit; ++n) {
	    UCode target = mp->map[n - mp->first];
	    if (n >= 128) {
		break;
	    } else if (target != 0 && target != n) {
		result = 0;
		break;
	    }
	}
	VERBOSE(2, ("character-set offset %u\n", result));
    }
    return result;
}

#ifdef USE_ICONV
/*
 * Display built-in encoding as ".enc" format.
 */
int
showBuiltinCharset(const char *name)
{
    FontEncPtr data = luitGetFontEnc(name, umBUILTIN);
    int rc = showOneCharset(name, data);
    luitFreeFontEnc(data);
    return rc;
}

/*
 * Display iconv encoding as ".enc" format.
 */
int
showIconvCharset(const char *name)
{
    FontEncPtr data = luitGetFontEnc(name, umICONV);
    int rc = showOneCharset(name, data);
    luitFreeFontEnc(data);
    return rc;
}
#endif /* USE_ICONV */

#ifdef NO_LEAKS
static void
freeFontMapRec(FontMapPtr data)
{
    switch (data->type) {
#ifdef FONT_ENCODING_TRUETYPE
    case FONT_ENCODING_TRUETYPE:
	/* FALLTHRU */
#endif
    case FONT_ENCODING_UNICODE:
	{
	    FontEncSimpleMapPtr mapHead = data->client_data;
	    free((void *) mapHead->map);
	    free(mapHead);
	}
	break;
#ifdef FONT_ENCODING_POSTSCRIPT
    case FONT_ENCODING_POSTSCRIPT:
	{
	    FontEncSimpleNamePtr mapHead = data->client_data;
	    if (mapHead->map) {
		int n;
		for (n = 0; mapHead->map[n]; ++n) {
		    free(mapHead->map[n]);
		}
		free(mapHead->map);
	    }
	    free(mapHead);
	}
	break;
#endif /* FONT_ENCODING_POSTSCRIPT */
    default:
	free(data->client_data);
	break;
    }
    free(data);
}

static void
freeFontEncRec(FontEncPtr data)
{
    if (data != NULL) {
	while (data->mappings != NULL) {
	    FontMapPtr next = data->mappings->next;
	    freeFontMapRec(data->mappings);
	    data->mappings = next;
	}
	if (data->aliases) {
	    int n;
	    for (n = 0; data->aliases[n]; ++n) {
		free(data->aliases[n]);
	    }
	    free(data->aliases);
	}
	free(data->name);
	free(data);
    }
}

void
fontenc_leaks(void)
{
    if (encodings_dir != NULL) {
	int enc;
	for (enc = 0; encodings_dir[enc].alias != NULL; ++enc) {
	    freeFontEncRec(encodings_dir[enc].data);
	    free(encodings_dir[enc].alias);
	    free(encodings_dir[enc].path);
	}
	free(encodings_dir);
    }
}
#endif /* NOLEAKS */
