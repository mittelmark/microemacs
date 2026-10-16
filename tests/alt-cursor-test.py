#!/usr/bin/env python3
"""Terminal modified-cursor-key regression test.

Drives the console build (mec) over a pty with the xterm CSI 1;<modifier><final>
sequences that terminals send for Shift/Alt/Control plus arrow and Home/End,
and checks that each reaches the matching binding.

A terminal encodes the modifier as 1 + shift(1) + alt(2) + control(4), so
Alt-Left is CSI 1;3D and Ctrl-Right is CSI 1;5C. Termcap only carries the
unmodified sequences, so before the fix the translation table had no entry
for the modified forms: the prefix match failed and the raw "1;3D" text was
inserted instead of running the bound command (A-left is bound to
backward-word by meemacs.emf).

The test:
  1. Creates a throw-away work directory and launches mec with
     tests/alt-cursor.emf, which binds a distinct macro to each modified key
     and f5 to dump the recorded tokens.
  2. Sends Alt-Left/Right/Up/Down, Ctrl-Left/Right, Shift-Left,
     Alt-Home/End and the unmodified Left/Up sequences.
  3. Sends f5 to write the recorded tokens and exit.
  4. Checks the tokens are exactly the ones expected (a key that was not
     translated would leak "1;3<final>" as text instead and leave its token
     out of the log).

Run from the repository root:

    python3 tests/alt-cursor-test.py [path-to-mec]
"""
import os, sys, pty, time, select, struct, fcntl, termios, subprocess, threading
import tempfile, shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROWS, COLS = 24, 80

EXIT_F5 = "\x1b[15~"

# (input sequence, expected token). The modified forms are the xterm
# CSI 1;<modifier><final> encodings (modifier = 1 + shift + 2*alt + 4*ctrl);
# the last two entries are the unmodified termcap sequences, included to
# guard against a regression in the shared translation table.
SEQUENCES = [
    ("\x1b[1;3D", ";al"),   # Alt-Left
    ("\x1b[1;3C", ";ar"),   # Alt-Right
    ("\x1b[1;3A", ";au"),   # Alt-Up
    ("\x1b[1;3B", ";ad"),   # Alt-Down
    ("\x1b[1;5D", ";cl"),   # Ctrl-Left
    ("\x1b[1;5C", ";cr"),   # Ctrl-Right
    ("\x1b[1;2D", ";sl"),   # Shift-Left
    ("\x1b[1;3H", ";ah"),   # Alt-Home
    ("\x1b[1;3F", ";ae"),   # Alt-End
    ("\x1b[D",    ";pl"),   # plain Left
    ("\x1b[A",    ";pu"),   # plain Up
]

EXPECTED = "".join(tok for _, tok in SEQUENCES)


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


def run(mec):
    """Launch mec, send the CSI sequences, return (tokens, raw_buffer)."""
    work = tempfile.mkdtemp(prefix="me-alt-work-")
    home = tempfile.mkdtemp(prefix="me-alt-")
    out_path = os.path.join(work, "alt-cursor-out.txt")

    master, slave = pty.openpty()
    fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
    env = dict(os.environ)
    env.update({
        "TERM": "xterm-256color",
        "TERM_PROGRAM": "xterm",
        "MENAME": "alt-cursor-" + os.path.basename(home),
        "MEPATH": os.path.join(ROOT, "jasspa", "macros"),
        "HOME": home,
        "ME_MOUSE_WORK": work,
    })
    proc = subprocess.Popen([mec, "@tests/alt-cursor"], stdin=slave,
                            stdout=slave, stderr=slave, env=env, cwd=ROOT,
                            preexec_fn=os.setsid)
    os.close(slave)

    stop = threading.Event()
    sink = []
    threading.Thread(target=drain, args=(master, sink, stop), daemon=True).start()

    try:
        time.sleep(0.9)                        # let ME finish start-up
        for seq, _ in SEQUENCES:
            os.write(master, seq.encode())
            time.sleep(0.2)
        time.sleep(0.3)
        os.write(master, EXIT_F5.encode())     # f5 - dump + exit
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

    got = run(mec)
    if got is None:
        print("ERROR: no output produced")
        return 2
    print("tokens: %r" % got)

    if got != EXPECTED:
        print("FAIL: expected %r, got %r" % (EXPECTED, got))
        return 1
    print("PASS: all modified cursor keys delivered (no literal text)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
