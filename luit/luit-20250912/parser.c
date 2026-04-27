/* $XTermId: parser.c,v 1.29 2025/09/12 08:20:14 tom Exp $ */

/*
Copyright 2011-2022,2025 by Thomas E. Dickey
Copyright (c) 2001 by Juliusz Chroboczek

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

#include <luit.h>

#include <parser.h>
#include <sys.h>
#include <trace.h>

#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

static char keyword[MAX_KEYWORD_LENGTH];

static void
skipEndOfLine(FILE *f, int c)
{
    if (c == 0)
	c = getc(f);

    for (;;)
	if (c <= 0 || c == '\n')
	    return;
	else
	    c = getc(f);
}

static int
drainWhitespace(FILE *f, int c)
{
    if (c == 0)
	c = getc(f);

    while (c > 0 && (c == '#' || c == ' ' || c == '\t')) {
	if (c == '#') {
	    skipEndOfLine(f, c);
	    c = '\n';
	    break;
	}
	c = getc(f);
    }

    return (c > 0) ? c : 0;
}

static int
getString(FILE *f, int string_end, int *c_return)
{
    int i = 0;
    int c;

    c = getc(f);
    while (c > 0) {
	if (c == string_end)
	    break;
	if (c == '\\') {
	    c = getc(f);
	    if (c == '\n')
		continue;
	}
	keyword[i++] = (char) c;
	if (i >= MAX_KEYWORD_LENGTH)
	    return TOK_ERROR;
	c = getc(f);
    }

    if (c <= 0)
	return TOK_ERROR;
    keyword[i] = '\0';
    *c_return = c;
    return TOK_KEYWORD;
}

static int
getToken(FILE *f, int c, int parse_assignments, int *c_return)
{
    int i;
    c = drainWhitespace(f, c);

    if (c < 0)
	return TOK_EOF;
    if (c == '\n') {
	*c_return = 0;
	return TOK_EOL;
    }

    if (parse_assignments && c == '=') {
	*c_return = 0;
	return TOK_EQUALS;
    }

    if (c == '\'' || c == '"')
	return getString(f, c, c_return);

    i = 0;
    while (c > 0 && c != ' ' && c != '\t' && c != '\n') {
	if (c == '\\') {
	    c = getc(f);
	    if (c == '\n')
		continue;
	}
	keyword[i++] = (char) c;
	if (i >= MAX_KEYWORD_LENGTH)
	    return TOK_ERROR;
	c = getc(f);
	if (parse_assignments && c == '=')
	    break;
    }

    *c_return = c < 0 ? 0 : c;
    keyword[i] = '\0';
    return TOK_KEYWORD;
}

/* Can parse both the old and new formats for locale.alias */
static int
parseTwoTokenLine(FILE *f, char *first, char *second)
{
    int rc = 0;
    int c = 0;
    int tok;
    size_t len;

    do {
	tok = getToken(f, c, 0, &c);
	if (tok == TOK_EOF)
	    return -1;
    } while (tok == TOK_EOL);

    if (tok != TOK_KEYWORD) {
	rc = -2;
    } else {
	if ((len = strlen(keyword)) != 0 && keyword[len - 1] == ':')
	    keyword[len - 1] = '\0';
	strcpy(first, keyword);

	tok = getToken(f, c, 0, &c);
	if (tok != TOK_KEYWORD) {
	    rc = -2;
	} else {
	    strcpy(second, keyword);

	    tok = getToken(f, c, 0, &c);
	    if (tok != TOK_EOL) {
		rc = -2;
	    }
	}
    }

    return rc;
}

/*
 * Check if the result from locale.alias has the encoding specified.
 */
static int
has_encoding(const char *locale)
{
    int result = 0;

    if (locale != NULL && *locale != 0) {
	char *dot = strchr(locale, '.');
	result = (dot != NULL
		  && dot != locale
		  && dot[1] != 0
		  && strchr(dot + 1, '.') == NULL);
    }
    return result;
}

char *
resolveLocale(const char *locale)
{
    FILE *f;
    char first[MAX_KEYWORD_LENGTH];
    char second[MAX_KEYWORD_LENGTH];
    char *resolved = NULL;
    int rc;
    int found = 0;

    TRACE(("resolveLocale(%s)\n", NonNull(locale)));
    if (locale == NULL)
	ExitFailure();

    TRACE(("...looking in %s\n", NonNull(locale_alias)));
    if (locale_alias == NULL)
	ExitFailure();

    f = fopen(locale_alias, "r");

    if (f != NULL) {
	do {
	    rc = parseTwoTokenLine(f, first, second);
	    if (rc < -1)
		break;
	    if (!strcmp(first, locale)) {
		resolved = strmalloc(second);
		found = 1;
		break;
	    }
	} while (rc >= 0);

	if (!found) {
	    if (resolved == NULL) {
		TRACE(("...not found in %s\n", NonNull(locale_alias)));
		resolved = strmalloc(locale);
	    }
	}

	fclose(f);
    }

    /*
     * If we did not find the data in the locale.alias file (or as happens with
     * some, the right column does not appear to specify a valid locale), see
     * if we can get a better result from the system's locale tables.
     */
    if (!found || !has_encoding(resolved)) {
#ifdef HAVE_LANGINFO_CODESET
	char *improved;
	if (!ignore_locale
	    && strcmp(locale, "C")
	    && strcmp(locale, "POSIX")
	    && strcmp(locale, "US-ASCII")
	    && (improved = nl_langinfo(CODESET)) != NULL) {
	    TRACE(("...nl_langinfo ->%s\n", NonNull(improved)));
	    free(resolved);
	    resolved = strmalloc(improved);
	} else
#endif
	if (f == NULL) {
	    if ((f = fopen(locale_alias, "r")) == NULL) {
		perror(locale_alias);
	    } else {
		fclose(f);
	    }
	}
    }

    TRACE(("...resolveLocale ->%s\n", NonNull(resolved)));
    return resolved;
}
