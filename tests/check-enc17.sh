#!/bin/sh
# Verify the output of tests/test-enc17-mn.emf (strict encoding tests).
# Usage: sh tests/check-enc17.sh
# Expects tests/test-enc17-mn-out.txt and tests/test-enc17-mn-iso-out.txt
# from a completed run. Exits 0 when everything matches.
cd "$(dirname "$0")/.." || exit 1

status=0

# Windows builds write CRLF, Unix builds LF - normalise before comparing
norm() { tr -d '\r' < "$1"; }

expected_out='TEST:iso-srcenc=ISO-8859-1
TEST:iso-dstenc=UTF-8
TEST:iso2utf-text=äXöY
TEST:mn-utf8=<αX😀Y
βZ😀W>
TEST:all-tests=complete'

expected_iso='TEST:mn-iso=<\u03B1X\uD83D\uDE00Y
\u03B2Z\uD83D\uDE00W>'

for f in tests/test-enc17-mn-out.txt tests/test-enc17-mn-iso-out.txt; do
    if [ ! -s "$f" ]; then
        echo "FAIL: $f missing or empty"
        status=1
    fi
done
[ $status -ne 0 ] && exit 1

if [ "$(norm tests/test-enc17-mn-out.txt)" = "$expected_out" ]; then
    echo "PASS: test-enc17-mn (utf-8 output)"
else
    echo "FAIL: test-enc17-mn (utf-8 output):"
    norm tests/test-enc17-mn-out.txt | sed 's/^/  | /'
    status=1
fi

if [ "$(norm tests/test-enc17-mn-iso-out.txt)" = "$expected_iso" ]; then
    echo "PASS: test-enc17-mn (iso-8859-1 escapes)"
else
    echo "FAIL: test-enc17-mn (iso-8859-1 escapes):"
    norm tests/test-enc17-mn-iso-out.txt | sed 's/^/  | /'
    status=1
fi

exit $status
