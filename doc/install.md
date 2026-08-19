# Installation and Update

This document describes the one-time installation process and the self-contained update mechanism for JASSPA MicroEmacs 09 on Unix-like systems (Linux, macOS, FreeBSD, Cygwin, MSYS2) and Windows.

## Quick Install

### Unix-like Systems (Linux, macOS, FreeBSD, Cygwin, MSYS2)

```bash
bash -c "$(curl -fsSL https://github.com/mittelmark/microemacs/releases/latest/download/install.sh)"
source ~/.bashrc   # or re-open your terminal
mecb -V            # verify installation
```

The command above downloads and runs the latest `install.sh` from GitHub.
It detects your operating system, downloads the matching pre-built binaries,
and places them in `~/.local/bin/`.

### Windows (PowerShell)

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
Invoke-RestMethod -Uri https://github.com/mittelmark/microemacs/releases/latest/download/install-windows.ps1 | Invoke-Expression
mecb -V            # verify installation
```

The script downloads and installs `mecb.exe` and `mewb.exe` to
`$env:LOCALAPPDATA\Microsoft\WindowsApps`, creates a Start Menu shortcut,
and writes `mecb-update.ps1` for future updates.

After installation, the `mecb-update` command is available for all future updates
(see [Updating](#updating)).

---

## What Gets Installed

### Unix-like Systems

| Command          | Description                                    |
|------------------|------------------------------------------------|
| `mecb`           | Console (terminal) version of MicroEmacs       |
| `mewb`           | GUI (X11 / Wayland / Windows) version          |
| `mecu`           | Wrapper to run `mecb` with UTF-8 + abduco      |
| `mecb-update`    | Self-contained update checker and installer    |

All files are installed into `~/.local/bin/`, which is automatically added to
your PATH on first install.

### Windows

| File               | Description                                    |
|--------------------|------------------------------------------------|
| `mecb.exe`         | Console (terminal) version of MicroEmacs       |
| `mewb.exe`         | GUI (X11 / Wayland / Windows) version          |
| `mecb-update.ps1`  | Self-contained PowerShell update checker       |

All files are installed into `$env:LOCALAPPDATA\Microsoft\WindowsApps`, and a
Start Menu shortcut for `mewb.exe` is created.

---

## How `install.sh` Works

`install.sh` performs these steps in order:

### 1. Detect System

The script reads `uname -o`, `uname -m`, and `uname -r` to determine:

- Operating system (Linux, Darwin, Cygwin, Msys, FreeBSD)
- Architecture (x86_64, i686, arm64)
- Kernel version

This determines which pre-built binary package to download from the GitHub
release assets.

### 2. Determine Latest Release Version

```bash
BASEURL=$(curl https://github.com/mittelmark/microemacs/releases/latest ...)
VERSION=$(echo $BASEURL | sed ...)
```

The script follows the GitHub `latest` redirect and extracts the tag
(e.g. `v09.12.26.beta3` → `091226b3`). This is the **target** version.

### 3. Check Existing Installation (`check_installed`)

Before downloading anything, `check_installed` runs:

1. Finds `mecb` in `$PATH` using `which mecb`.
2. If none found, proceeds with full install.
3. If found, runs `mecb -V` and parses the version string
   (e.g. `2009/12/26b3`).
4. Converts both the installed and latest versions to integers for comparison.
   The encoding is `YYMMDDbN` → `YY*1000000 + MM*10000 + DD*100 + N`.
5. If the installed version is **greater than or equal to** the latest release,
   the script exits with code 0 — no download, no overwrite.
6. Otherwise, it proceeds with the install.

This means running `install.sh` repeatedly is safe and idempotent:
it only downloads when a newer version is available.

### 4. Download and Extract Binaries

For each platform, two ZIP archives are fetched from GitHub releases:

```
https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/
    linux-6-x86_64-almalinux-10-microemacs-091226b3-mecb.zip
    linux-6-x86_64-almalinux-10-microemacs-091226b3-mewb.zip
```

The scripts extract only the binary files directly into `~/.local/bin/`
using `unzip -p`, without recreating the directory structure inside the ZIP.

### 5. Update PATH (if needed)

If `~/.local/bin` is not already in `$PATH`, the script appends the
appropriate export line to `~/.bashrc` or `~/.zshrc`, depending on the
user's login shell.

### 6. Install X11 Fonts (Linux / FreeBSD / Cygwin)

On graphical systems, the script installs font support for the **GUI version (`mewb`)** so that TrueType glyphs render correctly. The terminal version (`mecb`) does not require these fonts. If `xset` is unavailable, a fallback using `sudo` is suggested.

---

## The `mecu` Wrapper Script

`mecu` is a Bash/Zsh wrapper script that runs `mecb` with extended character
support via `luit` and session management via `abduco`. It is installed to
`~/.local/bin/mecu` on Unix-like systems.

### Requirements

- `luit` — filters between non-UTF-8 applications and UTF-8 terminals
- `abduco` — session management and detacher
  ([https://www.brain-dump.org/projects/abduco/](https://www.brain-dump.org/projects/abduco/))

Install on Debian/Ubuntu:

```bash
sudo apt install luit abduco
```

Install on Fedora/RHEL:

```bash
sudo dnf install luit abduco
```

### Usage

```bash
mecu                   # start mecb in a detached abduco session with luit
mecu -t dracula        # start with a color theme
mecu -e ISO8859-1      # specify encoding explicitly
mecu --help            # show options
```

### Options

| Option | Description                                          |
|--------|------------------------------------------------------|
| `-t`   | Theme name (`artur`, `ayu-dark`, `dracula`, `espresso`, `iterm`, or custom) |
| `-e`   | Encoding (default: `ISO8859-15`, use `luit -list` for all) |
| `-h`   | Show help and available options                      |

### How It Works

1. Parses `-t` (theme) and `-e` (encoding) options.
2. Applies terminal color theme if specified.
3. Checks if an abduco session named `mec<tty>` already exists.
   - **No session**: Creates one with `abduco -A -e ^z mec<tty> luit -encoding <enc> mecb "$@"`.
   - **Session exists**: Attaches to it with `abduco -a -e ^z mec<tty>`.
4. Press `Ctrl+z` to detach from the session without exiting `mecb`.

This allows running `mecb` in a persistent terminal session that survives
disconnections and supports UTF-8 content via `luit`.

`mecb-update` is a standalone, self-contained script written to
`~/.local/bin/mecb-update` during the first install. It contains all the
logic from `install.sh` needed for future updates, without requiring an
external download.

### Workflow

1. **Fetch latest release info** from GitHub (same as `install.sh`).
2. **Check installed version** via `mecb -V`.
3. **Skip** if installed version ≥ latest.
4. **Download** only `mecb` and `mewb` (no PATH setup, no fonts).
5. **Print** the new version numbers for confirmation.

### Usage

```bash
mecb-update              # check and update if needed
mecb-update              # run again — reports "Nothing to do" if current
```

Because `mecb-update` is self-contained, it works even after the system
has been rebooted or the terminal session has changed, as long as
`~/.local/bin` remains in your PATH.

---

## The `mecb-update.ps1` Script (Windows)

`mecb-update.ps1` is a standalone, self-contained PowerShell script written to
`%LOCALAPPDATA%\Microsoft\WindowsApps\mecb-update.ps1` during the first install.
It contains all the logic needed for future updates, without requiring an
external download.

### Workflow

1. **Fetch latest release info** from GitHub (same as `install-windows.ps1`).
2. **Check installed version** via `mecb.exe -V`.
3. **Skip** if installed version ≥ latest.
4. **Download** only `mecb.exe` and `mewb.exe` (no PATH setup, no shortcuts).
5. **Print** the new version numbers for confirmation.

### Usage

```powershell
# From PowerShell or cmd.exe
mecb-update.ps1              # check and update if needed
mecb-update.ps1              # run again — reports "Nothing to do" if current
```

Or from the Start Menu, search for "PowerShell" and run:

```powershell
& "$env:LOCALAPPDATA\Microsoft\WindowsApps\mecb-update.ps1"
```

Because `mecb-update.ps1` is self-contained, it works even after the system
has been rebooted, as long as `mecb.exe` and `mewb.exe` remain in
`%LOCALAPPDATA\Microsoft\WindowsApps`.

### Execution Policy

If you receive an execution policy error, set it for the current user:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

This allows locally created scripts (like `mecb-update.ps1`) to run while
still requiring downloaded scripts to be signed.

---

## Platform Support

The script supports the following platforms. Build filenames follow the pattern:

```
<os>-<kernel>-<arch>-<distro>-microemacs-<version>-mecb.zip
<os>-<kernel>-<arch>-<distro>-microemacs-<version>-mewb.zip
```

| Platform   | Detection                          | Example Build Name                              |
|------------|------------------------------------|--------------------------------------------------|
| Linux      | `uname -o` = `Linux`               | `linux-6-x86_64-ubuntu-22-microemacs-091226b4-mecb` |
| macOS      | `uname -o` = `Darwin`              | `macos-26-arm64-microemacs-091226b4-mecb`          |
| FreeBSD    | `uname -o` = `FreeBSD`             | `freebsd-14-x86_64-microemacs-091226b4-mecb`       |
| Cygwin     | `uname -s` contains `CYGWIN`       | `cygwin-3.6-x86_64-microemacs-091226b4-mecb`       |
| MSYS2      | `uname -o` = `Msys`                | `windows-msysunix-ucrt64-microemacs-091226b4-mecb` |

### Linux Distro Details

| Kernel | Arch       | Distro                     | Build Suffix                         |
|--------|------------|----------------------------|--------------------------------------|
| 4      | x86_64     | AlmaLinux 8                | `linux-4-x86_64-almalinux-8-mecb`    |
| 5      | i686       | Fedora 28                  | `linux-5-i686-fedora-28-mecb`        |
| 5      | x86_64     | AlmaLinux 9                | `linux-5-x86_64-almalinux-9-mecb`    |
| 5      | x86_64     | Ubuntu 20                  | `linux-5-x86_64-ubuntu-20-mecb`      |
| 5      | i686       | Ubuntu 18                  | `linux-5-i686-ubuntu-18-mecb`        |
| 6      | x86_64     | AlmaLinux 10               | `linux-6-x86_64-almalinux-10-mecb`   |
| 6      | x86_64     | Ubuntu 22                  | `linux-6-x86_64-ubuntu-22-mecb`      |
| 6      | x86_64     | Manjaro / Arch             | `linux-6-x86_64-manjaro-0-mecb`      |
| 7      | x86_64     | Fedora 43                  | `linux-7-x86_64-fedora-43-mecb`      |
| 7      | x86_64     | Ubuntu 26                  | `linux-7-x86_64-ubuntu-26-mecb`      |

### macOS Details

| Kernel | Arch       | Build Suffix                               |
|--------|------------|--------------------------------------------|
| 23     | x86_64     | `macos-15-x86_64-microemacs-<ver>-mecb`    |
| 23     | arm64      | `macos-14-arm64-microemacs-<ver>-mecb`     |
| 24+    | any        | Auto-selected based on kernel version      |

---

## Version Number Encoding

Versions are encoded as `YYMMDDbN` and compared numerically:

```
091226b3 → 09*1000000 + 12*10000 + 26*100 + 3   = 9,122,603
091226b4 → 09*1000000 + 12*10000 + 26*100 + 4   = 9,122,604
091226   → 09*1000000 + 12*10000 + 26*100 + 100 = 9,122,700   (final release)
```

A version without a `b` suffix (final / non-beta release) gets an internal
beta value of **100**. This guarantees that a final release ranks **higher**
than any beta of the same date:

| Installed | Latest  | Decision       |
|-----------|---------|----------------|
| `091226`  | `091226b4` | skip — already newer |
| `091226b3`| `091226b4` | install — beta 3 < beta 4 |
| `091226`  | `091226b3` | skip — final > beta |

Cross-date comparisons also work correctly:

```
091301 (final, next month) = 9,130,200 > 091226b99 = 9,122,699
091227 (final, next day)   = 9,122,800 > 091226b99 = 9,122,699
```

The `mecb -V` output format is:

```
MicroEmacs 09 - Date 2009/12/26b4 - linux
```

The date portion (`2009/12/26b4`) is parsed by the version check; the rest
is informational.

---

## Troubleshooting

### `mecb-update` not found

Ensure `~/.local/bin` is in your PATH:

```bash
echo $PATH | grep -q ~/.local/bin || echo "Not in PATH"
```

If missing, add to your shell config:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

### Version check reports wrong version

The script parses `mecb -V` output. If the binary is broken or returns
unexpected output, the check falls through to a full re-install. You can
force a reinstall by removing the existing binary first:

```bash
rm ~/.local/bin/mecb ~/.local/bin/mewb
bash -c "$(curl -fsSL https://github.com/mittelmark/microemacs/releases/latest/download/install.sh)"
```

### Old binary blockes new version

If you previously installed from an older release, the version check will
correctly detect the gap and proceed with the download. In the rare case
that the old binary refuses to run (e.g. architecture mismatch), remove it
and re-run the install.

### Missing `unzip` or `curl`

Both tools are required. Install them before running the script:

```bash
# Debian/Ubuntu
sudo apt install unzip curl

# Fedora
sudo dnf install unzip curl

# macOS
brew install unzip curl
```
