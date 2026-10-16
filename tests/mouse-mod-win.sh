#!/bin/sh
# Mouse-modifier regression test for the Windows console build.
#
# Drives mec.exe with one SGR click report per run - carrying the
# Shift/Alt/Control modifier bit (4/8/16) in the button number - and
# checks that each reaches the matching binding (tests/mouse-mod-win.emf).
# Before the fix the modifier bits were not stripped before the button
# was mapped, so a modifier-click matched no button and was discarded.
#
# The report is repeated ~1s apart rather than sent once up front: ME
# runs a TTahead()/TTinflush() during start-up which reads and discards
# input already waiting in the pipe.
#
# Usage: sh tests/mouse-mod-win.sh [path-to-mec.exe]
# Exits 0 when all four cases pass.
cd "$(dirname "$0")/.." || exit 1

MEC=${1:-src/.ucrt64win-release-mec/mec.exe}
export TERM=xterm-256color
export TERM_PROGRAM="Windows Terminal"
export ME_CONSOLE_PIPEMODE=1

WORK=mwtest-mod
rm -rf "$WORK" && mkdir -p "$WORK"
export ME_MOUSE_WORK="$WORK"

status=0

run_case() {
    want=$1
    esc=$2
    rm -f "$WORK/mouse-mod-win-out.txt"
    { for _i in 1 2 3 4; do sleep 1; printf "$esc" || break; done; sleep 2; } \
        | MEPATH=jasspa/macros timeout 30 "$MEC" @tests/mouse-mod-win > "$WORK/console-$want.out" 2>&1
    got=$(tr -d '\r' < "$WORK/mouse-mod-win-out.txt" 2>/dev/null)
    if [ "$got" = "$want" ]; then
        echo "PASS: $want click delivered"
    else
        echo "FAIL: $want click (got '$got')"
        head -c 200 "$WORK/console-$want.out" | cat -v
        status=1
    fi
}

# SGR button number: 0=left, 4=Shift+left, 8=Alt+left, 16=Ctrl+left
run_case plain '\033[<0;5;5M'
run_case shift '\033[<4;5;5M'
run_case alt   '\033[<8;5;5M'
run_case ctrl  '\033[<16;5;5M'

exit $status
