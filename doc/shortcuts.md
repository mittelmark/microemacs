# Keyboard shortcuts

MicroEmacs provides its default key bindings and optional GNU Emacs, CUA
(Windows Style), and NEdit v5 emulations. Select an emulation in the Emulation
control in `user-setup`. The table lists the main bindings supplied by each
mode; user settings and terminal or window-system key handling can change
them. CUA and NEdit are partial emulations, not complete reproductions of
their source editors.

`C-` means Control, `A-` means Alt, and `S-` means Shift. `Esc` followed by a
key means press Escape, then that key. A dash means there is no dedicated
binding in that mode.

| Action | MicroEmacs | GNU Emacs | CUA | NEdit v5 |
|---|---|---|---|---|
| Beginning of line | `C-a` | `C-a` | `Home` | `Home` |
| End of line | `C-e` | `C-e` | `End` | `End` |
| Beginning of buffer | `Esc <` | `Esc <` | `C-Home` | `C-Home` |
| End of buffer | `Esc >` | `Esc >` | `C-End` | `C-End` |
| Open file | `C-x C-f` | `C-x C-f` | `C-o` | `C-o` |
| Save buffer | `C-x C-s` | `C-x C-s` | `C-s` | `C-s` |
| Save as | `C-x C-w` | `C-x C-w` | `F12` | — |
| Exit | `C-x C-c` | `C-x C-c` | `C-q` | `C-q` |
| Cut | `C-w` | `C-w` | `C-x` | `C-x` |
| Copy | `Esc w` | `Esc w` | `C-c` | `C-c` |
| Paste | `C-y` | `C-y` | `C-v` | `C-v` |
| Undo | `C-/` | `C-/` | `C-z` | `C-z` |
| Redo | — | — | `C-y` | `A-C-z` |
| Select all | — | — | `C-a` | `C-/` |
| Search forward / backward | `C-s` / `C-r` | `C-s` / `C-r` | `C-f` / `A-C-f` | `C-f` / `C-r` |
| Go to line | `Esc g` | `Esc g` | `C-g` | `C-l` |
| Occur | `S-F6` | `S-F6` | `F6` | — |
| Toggle toolbar | `S-F11` | `S-F11` | `C-t t` | — |
| Toggle whitespace display | `F11` or `C-F11` | `F11` or `C-F11` | `C-t s` | — |
| Help | `C-h ?` | `C-h ?` | `C-t ?` | `C-h` |

The NEdit `C-f` and `C-r` bindings open its search dialog. Function-key
bindings may be intercepted by a terminal or window manager; for the active
key map, use the `describe-bindings` command.
