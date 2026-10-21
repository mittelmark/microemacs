#!/usr/bin/env python3
"""Mouse window-resize regression test (ticket 21).

Drives the console build (mec) over a pty with SGR mouse events and checks
that dragging the mode line between two stacked windows resizes them.

Why a pty: the drag is only meaningful while ME is in its input loop, and
the SGR mouse sequences have to arrive on the terminal file descriptor.

The test:
  1. Creates a throw-away work directory and launches mec with
     tests/mouse-resize.emf, which creates two stacked windows and binds
     f5 (dump geometry) / f6 (exit). The work directory is handed to the
     macro in $ME_MOUSE_WORK; the macro uses absolute paths built from it
     (ME resolves relative names against the current buffer's directory,
     so successive relative names would be doubled).
  2. Reads the first dump of the current (top) window.
  3. Sends a left-button press on that window's mode line, several motion
     events (button held) moving down, then the release.
  4. Sends f5 to dump again and f6 to exit.
  5. Passes if the window depth grew by at least 2 rows.

Run from the repository root:

    python3 tests/mouse-resize-test.py [path-to-mec]

Use --no-motion for a control run (press/release without motion), which must
NOT resize anything.
"""
import os, sys, pty, time, select, struct, fcntl, termios, subprocess, re
import tempfile, shutil, threading

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS, COLS = 24, 80


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


def send(fd, s):
    os.write(fd, s.encode())


def sgr(btn, col, row, final="M"):
    # SGR mouse report: ESC [ < btn ; col ; row M   (1-based coordinates)
    return f"\x1b[<{btn};{col};{row}{final}"


def parse_dumps(path):
    out = []
    if os.path.exists(path):
        with open(path) as f:
            for line in f:
                line = line.strip()
                if line.startswith("mode-line="):
                    out.append(dict(re.findall(r"(\S+?)=([-\d]+)", line)))
    return out


def main():
    no_motion = "--no-motion" in sys.argv
    mec = find_binary()
    if not os.path.exists(mec):
        print(f"ERROR: mec binary not found: {mec}")
        return 2

    # Throw-away work directory: keeps the repository clean, avoids stale
    # auto-save "name#" files and sidesteps ME's relative-name resolution.
    work = tempfile.mkdtemp(prefix="me-mouse-work-")
    out_path = os.path.join(work, "mouse-resize-out.txt")

    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    home = tempfile.mkdtemp(prefix="me-mouse-")
    env = dict(os.environ)
    env.update({
        "TERM": "xterm-256color",
        "TERM_PROGRAM": "xterm",
        "MENAME": "mouse-resize-" + os.path.basename(home),
        "MEPATH": os.path.join(ROOT, "jasspa", "macros"),
        "HOME": home,
        "ME_MOUSE_WORK": work,
    })
    proc = subprocess.Popen([mec, "@tests/mouse-resize"], stdin=slave,
                            stdout=slave, stderr=slave, env=env, cwd=ROOT,
                            preexec_fn=os.setsid)
    os.close(slave)

    stop = threading.Event()
    sink = []
    threading.Thread(target=drain, args=(master, sink, stop), daemon=True).start()

    def cleanup():
        stop.set()
        _kill(proc)
        shutil.rmtree(home, ignore_errors=True)
        shutil.rmtree(work, ignore_errors=True)

    dumps = []
    for _ in range(100):
        time.sleep(0.1)
        dumps = parse_dumps(out_path)
        if dumps:
            break
    if not dumps:
        print("ERROR: no initial dump produced")
        cleanup()
        return 2

    first = dumps[0]
    mode_line = int(first["mode-line"])
    depth = int(first["depth"])
    print("initial dump: mode-line=%d depth=%d" % (mode_line, depth))

    col = 5
    press_row = mode_line + 1          # 0-based -> 1-based
    send(master, sgr(0, col, press_row, "M"))               # press left
    time.sleep(0.15)
    if not no_motion:
        for i in range(1, 5):
            send(master, sgr(32, col, press_row + i, "M"))  # motion, held
            time.sleep(0.08)
    release_row = press_row + (0 if no_motion else 4)
    send(master, sgr(0, col, release_row, "m"))             # release
    time.sleep(0.3)

    send(master, "\x1b[15~")   # f5 - dump
    time.sleep(0.4)
    send(master, "\x1b[17~")   # f6 - exit
    time.sleep(0.4)

    dumps = parse_dumps(out_path)
    cleanup()

    if len(dumps) < 2:
        print("FAIL: second dump missing (f5 not received?)")
        return 2
    second = dumps[-1]
    delta = int(second["depth"]) - depth
    print("after drag:   mode-line=%s depth=%s" % (second["mode-line"], second["depth"]))

    if no_motion:
        if delta == 0:
            print("PASS: control (no motion) did not resize")
            return 0
        print("FAIL: control resized without motion")
        return 1
    if delta >= 2:
        print("PASS: window resized by mouse drag (depth %+d)" % delta)
        return 0
    print("FAIL: window did not resize (depth %+d)" % delta)
    return 1


def _kill(proc):
    try:
        os.killpg(os.getpgid(proc.pid), 15)
        proc.wait(timeout=5)
    except Exception:
        pass


if __name__ == "__main__":
    sys.exit(main())
