# Copilot instructions

MicroEmacs is a cross-platform editor implemented in C and its own ME macro
language. Keep changes focused, preserve platform behavior, and follow the
project guidance in `AGENTS.md` when more detail is needed.

## Code and macros

- Follow the surrounding file's style. C uses four-space indentation, braces
  for control flow, and existing naming conventions; avoid unnecessary
  comments.
- Keep `DEFVAR` entries in `src/evar.def` alphabetically sorted. The runtime
  uses binary search, so misordering breaks variable lookup.
- Follow existing ME macro conventions in `.emf` files, including `$` global
  variables, `#l0`-`#l9` local registers, and `!if`/`!while` control forms.
- For documentation, edit source files, not generated output. In `doc/`, edit
  `front.smd`, `history.smd`, `faq.smd`, or `reference.smd`, never
  `me-full.smd`.

## Build and test

- Linux: `make -f makefiles/unixgcc.gmk mec` (or `mew`, `mecb`, `mewb`).
- MSYS2: use `makefiles/winmingwgcc.gmk`; its default `mecb` builds both the
  mintty/MSYS2 and native Windows variants. `mewb` builds the native Windows
  variant. Pass `BDIST` explicitly when only one distribution is wanted.
- On other platforms, use the matching makefile in `makefiles/`; do not route
  MSYS2 through `unixgcc.gmk`.
- Run the smallest relevant test. The cross-platform editor regression suite
  can be run with `MEPATH=jasspa/macros <editor> @tests/test-basics`, then
  verify `tests/test-output.txt` contains `TEST:all-tests=complete`.
- Shell installers should pass `bash -n`; test platform selection without
  performing real downloads or installation.

## Repository workflow

- Preserve unrelated worktree changes. Do not commit unless the user
  explicitly approves; before a commit, present the staged changes and proposed
  message.
- For release changes, keep the version in `src/evers.h` aligned with release
  tags and update installer assets only when the release process requires it.
- The project targets Linux, macOS, FreeBSD, Windows, Cygwin, and MSYS2; check
  platform-specific build rules before changing shared code.
