#!/usr/bin/env python3
"""Mouse wheel scrolling regression test (ticket 22).

Drives the console build (mec) over a pty with SGR wheel events and checks
that the current window scrolls vertically.

Why a pty: ME only parses SGR mouse reports while it is in its input loop,
and the sequences have to arrive on the terminal file descriptor.

A terminal reports a wheel tick as a single SGR press (button 64 = up,
65 = down) with NO release, unlike X-Windows which sends a press and a
release. The terminal decoder synthesises the release so that the existing
mouse.emf wheel translation ("mouse-pick-N mouse-drop-N" -> mouse-wup/
mouse-wdown) runs. This test guards that path: a wheel-down must move the
window's vertical scroll, a wheel-up must move it back, and no wheel event
must leave it untouched.

The test:
  1. Creates a throw-away work directory and launches mec with
     tests/mouse-wheel.emf, which loads a 200 line buffer and binds f5
     (dump geometry) / f6 (exit).
  2. Sends N SGR wheel-down events (and optionally M wheel-up events).
  3. Sends f5 to dump $window-y-scroll and f6 to exit.
  4. Checks the vertical scroll position.

Run from the repository root:

    python3 tests/mouse-wheel-test.py [path-to-mec]
"""
import os, sys, pty, time, select, struct, fcntl, termios, subprocess, re
import tempfile, shutil, threading

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS, COLS = 24, 80

WHEEL_UP, WHEEL_DOWN = 64, 65


def find_binary():
    for a in sys.argv[1:]:
        if not a.startswith("--"):
            return a
    for cand in ("src/.linuxgcc-release-mec/mec",
                 "src/.linuxgcc-debug-mec/mec",
                 "src/.linuxgcc-release-mecw/mecw"):
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


def parse_dump(path):
    """Return the last dumped geometry as a dict of ints."""
    if not os.path.exists(path):
        return None
    line = None
    with open(path) as f:
        for ln in f:
            ln = ln.strip()
            if ln.startswith("y-scroll="):
                line = ln
    if line is None:
        return None
    return {k: int(v) for k, v in re.findall(r"(\S+?)=([-\d]+)", line)}


def _kill(proc):
    try:
        os.killpg(os.getpgid(proc.pid), 15)
        proc.wait(timeout=5)
    except Exception:
        pass


def run(mec, down, up):
    """Launch mec, send wheel events, return the dumped scroll geometry."""
    work = tempfile.mkdtemp(prefix="me-wheel-work-")
    home = tempfile.mkdtemp(prefix="me-wheel-")
    out_path = os.path.join(work, "mouse-wheel-out.txt")

    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    env = dict(os.environ)
    env.update({
        "TERM": "xterm-256color",
        "TERM_PROGRAM": "xterm",
        "MENAME": "mouse-wheel-" + os.path.basename(home),
        "MEPATH": os.path.join(ROOT, "jasspa", "macros"),
        "HOME": home,
        "ME_MOUSE_WORK": work,
    })
    proc = subprocess.Popen([mec, "@tests/mouse-wheel"], stdin=slave,
                            stdout=slave, stderr=slave, env=env, cwd=ROOT,
                            preexec_fn=os.setsid)
    os.close(slave)

    stop = threading.Event()
    sink = []
    threading.Thread(target=drain, args=(master, sink, stop), daemon=True).start()

    try:
        # Let ME finish start-up (no initial dump keeps the src buffer
        # current, which is the buffer the wheel scrolls).
        time.sleep(0.8)
        for _ in range(down):
            os.write(master, sgr(WHEEL_DOWN, 5, 5).encode())
            time.sleep(0.15)
        for _ in range(up):
            os.write(master, sgr(WHEEL_UP, 5, 5).encode())
            time.sleep(0.15)
        time.sleep(0.4)
        os.write(master, "\x1b[15~".encode())      # f5 - dump
        time.sleep(0.4)
        os.write(master, "\x1b[17~".encode())      # f6 - exit
        time.sleep(0.4)
        return parse_dump(out_path)
    finally:
        stop.set()
        _kill(proc)
        shutil.rmtree(home, ignore_errors=True)
        shutil.rmtree(work, ignore_errors=True)


def main():
    mec = find_binary()
    if not os.path.exists(mec):
        print(f"ERROR: mec binary not found: {mec}")
        return 2

    down = run(mec, 8, 0)
    if down is None:
        print("ERROR: no dump produced")
        return 2
    print("8 wheel-down: %s" % down)

    both = run(mec, 8, 4)
    if both is None:
        print("ERROR: no dump produced")
        return 2
    print("8 wheel-down, 4 wheel-up: %s" % both)

    none = run(mec, 0, 0)
    if none is None:
        print("ERROR: no dump produced")
        return 2
    print("no wheel: %s" % none)

    y_down = down.get("y-scroll", 0)
    y_both = both.get("y-scroll", 0)
    y_none = none.get("y-scroll", 0)

    ok = True
    if y_down < 8:
        print("FAIL: wheel-down did not scroll (y-scroll=%d)" % y_down)
        ok = False
    if not (0 < y_both < y_down):
        print("FAIL: wheel-up did not scroll back (y-scroll=%d, down=%d)"
              % (y_both, y_down))
        ok = False
    if y_none != 0:
        print("FAIL: control scrolled without a wheel event (y-scroll=%d)"
              % y_none)
        ok = False

    if ok:
        print("PASS: mouse wheel scrolls down (y=%d) and back up (y=%d)"
              % (y_down, y_both))
        return 0
    return 1


if __name__ == "__main__":
    sys.exit(main())
