#!/usr/bin/env python3
"""Terminal middle-click PRIMARY-paste regression test (ticket 25).

Drives the console build (mec) over a pty with an SGR middle-button click
and checks that the paste reads the PRIMARY selection rather than the
CLIPBOARD one.

A terminal middle-click should paste the primary selection (the X11
convention). The console clipboard code flags the next external fetch as a
PRIMARY read when the middle button is released, and then runs the
clipboard tool with the primary selector. Before the fix a middle click
pasted the same CLIPBOARD content as C-y.

To stay hermetic the test installs a fake `xclip` on PATH that logs its
arguments and prints different content for the primary and clipboard
selections. It then presses C-y (an ordinary paste) and a middle click and
asserts:

  1. the middle click was invoked as `-selection primary -o`,
  2. C-y was still invoked as `-selection clipboard -o`, and
  3. both contents landed in the buffer.

Run from the repository root:

    python3 tests/mouse-mid-test.py [path-to-mec]
"""
import os, sys, pty, time, select, struct, fcntl, termios, subprocess, threading
import tempfile, shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS, COLS = 24, 80

EXIT_F5 = "\x1b[15~"

FAKE_XCLIP = """#!/bin/sh
# Fake xclip for tests/mouse-mid-test.py: log the arguments and answer the
# primary/clipboard selections with distinguishable content.
echo "xclip $*" >> "$ME_MOUSE_WORK/clip-calls.txt"
out=0
primary=0
for a in "$@"; do
    case "$a" in
        -o|--output)            out=1 ;;
        -i|--input)             out=0 ;;
        primary|--primary)      primary=1 ;;
        clipboard|--clipboard)  primary=0 ;;
    esac
done
if [ "$out" = 1 ]; then
    if [ "$primary" = 1 ]; then
        printf 'PRIMARY-CONTENT'
    else
        printf 'CLIPBOARD-CONTENT'
    fi
else
    cat > /dev/null
fi
"""


def find_binary():
    for a in sys.argv[1:]:
        if not a.startswith("--"):
            return a
    for cand in ("src/.linuxgcc-release-mec/mec",
                 "src/.linuxgcc-debug-mec/mec",
                 "src/.linuxgcc-release-mecw/mecw",
                 "src/.macosgcc-release-mec/mec"):
        p = os.path.join(ROOT, cand)
        if os.path.exists(p):
            return p
    return os.path.join(ROOT, "src/.linuxgcc-release-mec/mec")


def drain(fd, sink, stop):
    while not stop.is_set():
        r, _, _ = select.select([fd], [], [], 0.2)
        if fd in r:
            try:
                data = os.read(fd, 65536)
            except OSError:
                break
            if not data:
                break
            sink.append(data)


def sgr(btn, col, row, final="M"):
    # SGR mouse report: ESC [ < btn ; col ; row M   (1-based coordinates)
    return f"\x1b[<{btn};{col};{row}{final}"


def middle_click(fd, col, row):
    os.write(fd, sgr(1, col, row, "M").encode())   # button 1 = middle
    time.sleep(0.15)
    os.write(fd, sgr(1, col, row, "m").encode())
    time.sleep(0.15)


def read_out(path):
    if not os.path.exists(path):
        return None
    with open(path) as f:
        return f.read()


def _kill(proc):
    try:
        os.killpg(os.getpgid(proc.pid), 15)
        proc.wait(timeout=5)
    except Exception:
        pass


def run(mec):
    """Launch mec, yank with C-y then middle-click, return (buffer, calls)."""
    work = tempfile.mkdtemp(prefix="me-mid-work-")
    home = tempfile.mkdtemp(prefix="me-mid-")
    bindir = tempfile.mkdtemp(prefix="me-mid-bin-")

    fake = os.path.join(bindir, "xclip")
    with open(fake, "w") as f:
        f.write(FAKE_XCLIP)
    os.chmod(fake, 0o755)

    out_path = os.path.join(work, "mouse-mid-out.txt")
    calls_path = os.path.join(work, "clip-calls.txt")

    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    env = dict(os.environ)
    env.pop("XDG_SESSION_TYPE", None)
    env.pop("WAYLAND_DISPLAY", None)
    env.update({
        "TERM": "xterm-256color",
        "TERM_PROGRAM": "xterm",
        "DISPLAY": ":0",                 # pretend X11 so xclip is selected
        "PATH": bindir + os.pathsep + env.get("PATH", ""),
        "MENAME": "mouse-mid-" + os.path.basename(home),
        "MEPATH": os.path.join(ROOT, "jasspa", "macros"),
        "HOME": home,
        "ME_MOUSE_WORK": work,
    })
    proc = subprocess.Popen([mec, "@tests/mouse-mid"], stdin=slave,
                            stdout=slave, stderr=slave, env=env, cwd=ROOT,
                            preexec_fn=os.setsid)
    os.close(slave)

    stop = threading.Event()
    sink = []
    threading.Thread(target=drain, args=(master, sink, stop), daemon=True).start()

    try:
        time.sleep(0.9)                        # let ME finish start-up
        os.write(master, b"\x19")              # C-y - paste CLIPBOARD
        time.sleep(0.4)
        middle_click(master, 8, 5)
        time.sleep(0.5)                        # allow the clipboard fork
        os.write(master, EXIT_F5.encode())     # f5 - save + exit
        time.sleep(0.6)
        return read_out(out_path), read_out(calls_path)
    finally:
        stop.set()
        _kill(proc)
        shutil.rmtree(home, ignore_errors=True)
        shutil.rmtree(bindir, ignore_errors=True)
        shutil.rmtree(work, ignore_errors=True)


def main():
    mec = find_binary()
    if not os.path.exists(mec):
        print(f"ERROR: mec binary not found: {mec}")
        return 2

    buf, calls = run(mec)
    print("xclip calls: %r" % calls)
    print("buffer:      %r" % buf)

    if buf is None:
        print("ERROR: no output produced")
        return 2
    if calls is None or "-selection primary -o" not in calls:
        print("FAIL: expected a PRIMARY read, xclip calls were %r" % calls)
        return 1
    if "-selection clipboard -o" not in calls:
        print("FAIL: expected an ordinary C-y CLIPBOARD read, xclip calls "
              "were %r" % calls)
        return 1
    if "CLIPBOARD-CONTENT" not in buf:
        print("FAIL: CLIPBOARD content not pasted by C-y, buffer was %r" % buf)
        return 1
    if "PRIMARY-CONTENT" not in buf:
        print("FAIL: PRIMARY content not pasted, buffer was %r" % buf)
        return 1
    print("PASS: C-y pasted CLIPBOARD, middle-click pasted PRIMARY")
    return 0


if __name__ == "__main__":
    sys.exit(main())
