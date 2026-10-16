#!/usr/bin/env python3
"""Mouse modifier-click regression test.

Drives the console build (mec) over a pty with SGR click events that carry
the Shift/Control/Alt modifier bits and checks that each reaches the
matching binding.

A terminal encodes the modifiers in the SGR button number
(4=Shift, 8=Alt, 16=Control). Before the fix those bits were not stripped
before the button was mapped, so e.g. Shift+left (4) or Ctrl+left (16)
matched no button and the whole event was silently discarded.

The test:
  1. Creates a throw-away work directory and launches mec with
     tests/mouse-mod.emf, which binds a distinct macro to each of
     mouse-pick-1 / S-mouse-pick-1 / C-mouse-pick-1 / A-mouse-pick-1 and
     f5 to dump the recorded tokens.
  2. Sends a plain click, then a Shift, Control and Alt click.
  3. Sends f5 to write the recorded tokens and exit.
  4. Checks the recorded tokens are exactly plain;shift;ctrl;alt; .

Run from the repository root:

    python3 tests/mouse-mod-test.py [path-to-mec]
"""
import os, sys, pty, time, select, struct, fcntl, termios, subprocess, threading
import tempfile, shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS, COLS = 24, 80

BUTTON, SHIFT, ALT, CTRL = 0, 4, 8, 16

EXPECTED = "plain;shift;ctrl;alt;"


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


def key(fd, btn, col, row):
    """Send a full click (press + release) with the given button code."""
    os.write(fd, sgr(btn, col, row, "M").encode())
    time.sleep(0.15)
    os.write(fd, sgr(btn, col, row, "m").encode())
    time.sleep(0.15)


def read_out(path):
    if not os.path.exists(path):
        return None
    with open(path) as f:
        return f.read().strip()


def _kill(proc):
    try:
        os.killpg(os.getpgid(proc.pid), 15)
        proc.wait(timeout=5)
    except Exception:
        pass


def run(mec, buttons):
    """Launch mec, send the given clicks, return the recorded token string."""
    work = tempfile.mkdtemp(prefix="me-mod-work-")
    home = tempfile.mkdtemp(prefix="me-mod-")
    out_path = os.path.join(work, "mouse-mod-out.txt")

    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    env = dict(os.environ)
    env.update({
        "TERM": "xterm-256color",
        "TERM_PROGRAM": "xterm",
        "MENAME": "mouse-mod-" + os.path.basename(home),
        "MEPATH": os.path.join(ROOT, "jasspa", "macros"),
        "HOME": home,
        "ME_MOUSE_WORK": work,
    })
    proc = subprocess.Popen([mec, "@tests/mouse-mod"], stdin=slave,
                            stdout=slave, stderr=slave, env=env, cwd=ROOT,
                            preexec_fn=os.setsid)
    os.close(slave)

    stop = threading.Event()
    sink = []
    threading.Thread(target=drain, args=(master, sink, stop), daemon=True).start()

    try:
        time.sleep(0.8)                        # let ME finish start-up
        for i, btn in enumerate(buttons):
            key(master, btn, 5 + i, 5 + i)
        time.sleep(0.4)
        os.write(master, "\x1b[15~".encode())   # f5 - dump + exit
        time.sleep(0.6)
        return read_out(out_path)
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

    got = run(mec, [BUTTON, SHIFT, CTRL, ALT])
    if got is None:
        print("ERROR: no output produced")
        return 2
    print("clicks delivered: %r" % got)

    if got != EXPECTED:
        print("FAIL: expected %r, got %r" % (EXPECTED, got))
        return 1
    print("PASS: plain/Shift/Ctrl/Alt clicks all delivered")
    return 0


if __name__ == "__main__":
    sys.exit(main())
