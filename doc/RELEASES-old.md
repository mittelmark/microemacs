---
title: Older Release of MicroEmacs 09
author: Detlef Groth, University of Potsdam
date: 2025-08-24 19:56
---


> Older release notes and legacy topics moved from the main [README.md](../README.md).

## Download Prebuild MicroEmacs Executables (v09.12.26b4)

This release provides the following new features in comparison to v09.12.26b3:

- fix for excessive clipboard tool calling if in therminal version clipboard is enabled
- fix for creation of debugging file
- new theme Nord
- new platform: Windows Msys2 terminal platform supported with filepaths like /c/ or /home etc
- new platform: support for RedHat distros with Kernel 7

It further contains the following new features in comparison the v09.12.25 (2025) release.

- support for Mouse in terminal on Windows  and Unix (except for VTE based terminals lile ROXterm)
- support for Clipboard in terminal versions on Unix
- bugfix for crash on terminal resize on Windows
- bugfix on Unix for cut and paste (beta2 and beta3 fix, requiring xclip (X11), wl-clipboard (Wayland), pbpaste (macOS)
- support for Ubuntu/Debian with Kernel 7.0 aarch64 and intel
- terminal version: support for 16 colors in the terminal
- terminal version: automatic detection and use of colors, no need for TERM=xterm anymore
- graphical version: support for clipboard on Wayland and X11 (activate in user-setup, system), on Wayland wl-clipboard package is required, on X11 xclip gives full support for primary and clipboard
- new platforms: FreeBSD 15, MacOS 26 (intel and apple chips), MacOS 27 (only apple chip)
- new programming/diagram languages: PlantUML, Haxe transpiler, Fusion transpiler, Groovy, Scala, Typescript
- terminal and clipboard support we programmed with aid of AI tool opencode with the Big Pickle model

> [!NOTE] 
> Please   note  that  new   developments   take  place mainly in  the
> [MicroEmacs 26](https://github.com/bjasspa/jasspa)  branch.  So new  users  might try this
> release first. This ME26 branch should contain most of the things described above
> but as well  provided  more modern fonts support (TTF and OTF) using the libxft library,
> https  support terminal  but currently does n not provide older Linux 64 bit,  Linux 32 bit and
> FreeBSD builds (yet). 

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-ubuntu-18-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-ubuntu-18-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-ubuntu-18-microemacs-091226b4-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-fedora-28-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-fedora-28-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-i686-fedora-28-microemacs-091226b4-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-27 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-4-x86_64-almalinux-8-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-x86_64-almalinux-8-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-4-x86_64-almalinux-8-microemacs-091226b4-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 28-34  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-almalinux-9-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-almalinux-9-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-almalinux-9-microemacs-091226b4-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 35-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-almalinux-10-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-almalinux-10-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-almalinux-10-microemacs-091226b4-mecwb.zip) |
|             | Fedora 43-46    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-fedora-43-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-fedora-43-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-fedora-43-microemacs-091226b4-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-manjaro-0-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-manjaro-0-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-manjaro-0-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-18-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-18-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-18-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-20-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-20-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-5-x86_64-ubuntu-20-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-22-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-22-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-22-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-24-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-24-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-x86_64-ubuntu-24-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 26         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-ubuntu-26-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-ubuntu-26-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-x86_64-ubuntu-26-microemacs-091226b4-mecwb.zip) |
| Linux aarch64 | Ubuntu 22       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-22-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-22-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-22-microemacs-091226b4-mecwb.zip) |
| (Raspberry Pi)| Ubuntu 24       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-24-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-24-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-6-aarch64-ubuntu-24-microemacs-091226b4-mecwb.zip) |
|             | Ubuntu 26       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-aarch64-ubuntu-26-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-aarch64-ubuntu-26-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/linux-7-aarch64-ubuntu-26-microemacs-091226b4-mecwb.zip) |
| MacOS       | MacOS 14/15 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-x86_64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-x86_64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-x86_64-microemacs-091226b4-mecwb.zip) |
|             | MacOS 26 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-x86_64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-x86_64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-16-x86_64-microemacs-091226b4-mecwb.zip) |
|             | MacOS 14 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-14-arm64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-14-arm64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-14-arm64-microemacs-091226b4-mecwb.zip) |
|             | MacOS 15 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-arm64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-arm64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-15-arm64-microemacs-091226b4-mecwb.zip) |
|             | MacOS 26 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-26-arm64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-26-arm64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-26-arm64-microemacs-091226b4-mecwb.zip) |
|             | MacOS 27 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-27-arm64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-27-arm64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/macos-27-arm64-microemacs-091226b4-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mecwb.zip) |
|             | FreeBSD 15 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/freebsd-14-amd64-microemacs-091226b4-mecwb.zip) |
| Windows*    | Windows 98-10/11 mingw32   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-mingw32-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-mingw32-microemacs-091226b4-mewb.zip) | - |
| (intel, arm) | Windows 98-10/11 mingw64   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-mingw64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-mingw64-microemacs-091226b4-mewb.zip) | - |
|             | Windows 10/11 ucrt64   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-ucrt64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-ucrt64-microemacs-091226b4-mewb.zip) | - |
|             | Windows 10/11 msys2    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-msysunix-ucrt64-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/windows-mingw-ucrtr64-microemacs-091226b4-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.3-i686-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.3-i686-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.3-i686-microemacs-091226b4-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.6-microemacs-091226b4-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.6-microemacs-091226b4-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta4/cygwin-3.6-x86_64-microemacs-091226b4-mecwb.zip) |

## Download Prebuild MicroEmacs Executables (v09.12.26b3)

This release provides the following new features in comparison to v09.12.25:

- support for Mouse in terminal on Windows  and Unix (except for VTE based terminals lile ROXterm)
- support for Clipboard in terminal versions on Unix
- bugfix for crash on terminal resize on Windows
- bugfix on Unix for cut and paste (beta2 and beta3 fix, requiring xclip (X11), wl-clipboard (Wayland), pbpaste (macOS)
- support for Ubuntu/Debian with Kernel 7.0 aarch64 and intel
- terminal version: support for 16 colors in the terminal
- terminal version: automatic detection and use of colors, no need for TERM=xterm anymore
- graphical version: support for clipboard on Wayland and X11 (activate in user-setup, system), on Wayland wl-clipboard package is required, on X11 xclip gives full support for primary and clipboard
- new platforms: FreeBSD 15, MacOS 26 (intel and apple chips), MacOS 27 (only apple chip)
- new programming/diagram languages: PlantUML, Haxe transpiler, Fusion transpiler, Groovy, Scala, Typescript
- terminal and clipboard support we programmed with aid of AI tool opencode with the Big Pickle model

> [!NOTE] 
> Please   note  that  new   developments   take  place mainly in  the
> [MicroEmacs 26](https://github.com/bjasspa/jasspa)  branch.  So new  users  might try this
> release first. This ME26 branch should contain most of the things described above
> but as well  provided  more modern fonts support (TTF and OTF) using the libxft library,
> https  support terminal  but currently does n not provide older Linux 64 bit,  Linux 32 bit and
> FreeBSD builds (yet). 

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-ubuntu-18-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-ubuntu-18-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-ubuntu-18-microemacs-091226b3-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-fedora-28-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-fedora-28-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-i686-fedora-28-microemacs-091226b3-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-29 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-4-x86_64-almalinux-8-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-x86_64-almalinux-8-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-4-x86_64-almalinux-8-microemacs-091226b3-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 30-36  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-almalinux-9-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-almalinux-9-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-almalinux-9-microemacs-091226b3-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 27-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-almalinux-10-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-almalinux-10-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-almalinux-10-microemacs-091226b3-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-manjaro-0-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-manjaro-0-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-manjaro-0-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-18-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-18-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-18-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-20-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-20-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-5-x86_64-ubuntu-20-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-22-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-22-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-22-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-24-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-24-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-x86_64-ubuntu-24-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 26         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-x86_64-ubuntu-26-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-x86_64-ubuntu-26-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-x86_64-ubuntu-26-microemacs-091226b3-mecwb.zip) |
| Linux aarch64 | Ubuntu 22       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-22-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-22-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-22-microemacs-091226b3-mecwb.zip) |
| (Raspberry Pi)| Ubuntu 24       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-24-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-24-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-6-aarch64-ubuntu-24-microemacs-091226b3-mecwb.zip) |
|             | Ubuntu 26       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-aarch64-ubuntu-26-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-aarch64-ubuntu-26-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/linux-7-aarch64-ubuntu-26-microemacs-091226b3-mecwb.zip) |
| MacOS       | MacOS 14/15 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-x86_64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-x86_64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-x86_64-microemacs-091226b3-mecwb.zip) |
|             | MacOS 26 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-x86_64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-x86_64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-16-x86_64-microemacs-091226b3-mecwb.zip) |
|             | MacOS 14 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-14-arm64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-14-arm64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-14-arm64-microemacs-091226b3-mecwb.zip) |
|             | MacOS 15 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-arm64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-arm64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-15-arm64-microemacs-091226b3-mecwb.zip) |
|             | MacOS 26 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-26-arm64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-26-arm64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-26-arm64-microemacs-091226b3-mecwb.zip) |
|             | MacOS 27 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-27-arm64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-27-arm64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/macos-27-arm64-microemacs-091226b3-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mecwb.zip) |
|             | FreeBSD 15 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/freebsd-14-amd64-microemacs-091226b3-mecwb.zip) |
| Windows     | Windows 98-10/11 mingw32   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-MINGW32-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-MINGW32-microemacs-091226b3-mewb.zip) | - |
| (intel, arm) | Windows 98-10/11 mingw64   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-MINGW64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-MINGW64-microemacs-091226b3-mewb.zip) | - |
|             | Windows 10/11 ucrt64   | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-UCRT64-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/windows-mingw-UCRT64-microemacs-091226b3-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.3-i686-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.3-i686-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.3-i686-microemacs-091226b3-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.6-microemacs-091226b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.6-microemacs-091226b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta3/cygwin-3.6-x86_64-microemacs-091226b3-mecwb.zip) |

__Installation:__

Installation  of these  executables  is easy.  Make  them  executable  on Unix
platforms and move them to a folder  belonging to your PATH variable.  Windows
users should just copy them as well to such a folder.

Just  download an  executable  for your  platform  which matches as closely as
possible your operatig system. For instance for Fedora 39, you download the binaries for Fedora 38.
On Unix systems you make the file  executable  (chmod 755 filename) and rename
it for  instance  to me, then  copy it to a  folder  belonging  to your  PATH.
Therafter you can run the me executable.  The first thing you have to do is to
select the right  keyboard  configuration  after  starting your first session.
Choose the menu entry "Tools -> User Setup" and then  "Keyboard"  the Start-Up
tab.

[Ubuntu](https://ubuntu.com/)      builds      should     be     usable     on
[Debian](https://www.debian.org/)   and  derived   distros,   such  as  [Linux
Mint](https://www.linuxmint.com)     or    [MX     Linux](https://mxlinux.org)
compatible.     [AlmaLinux](https://almalinux.org)     builds     should    be
[CentOS](https://www.centos.org)                                           and
[RHEL](https://www.redhat.com/en/technologies/linux-platforms/enterprise-linux)
compatible.  [Fedora](https://www.fedora.org) builds can be probably only used
on Fedora without problems.

Build  for  other   platforms   might  be  provided  if  requested  using  the
[issues](https://github.com/mittelmark/microemacs/issues)    link    on   this
repository. 

### Msys2 Windows Terminal

> [!NOTE]
> Since version 26b4 a true Msys2 terminal version is available, 
> so the fix outlined below is not required anymore.


I usually  recommend  the   [Msys2](https://www.msys2.org)   environment  for
developers if they have to use the Windows  operating  system. As the provided
Windows build is a native  Windows build, the console  version of Me09 must be
started via the cmd  Terminal on Windows. You should use in this case a bas function
like this code below which should be added to your _.bashrc_.

```bash
### add this to your .bashrc
### we assume that you copied the windows executables
### to the bin folder in your msys HOME
function me {
    if [ $1 == "-n" ]; then
        ## running terminal version
        shift 1
        cmd //C `cygpath -wa ~/bin/mec-windows.exe` "${@}"
        
    else
        `cygpath -wa ~/bin/mew-windows.exe` "${@}" &
    fi
}
```

__Fonts:__

Download more programmers fonts: [TTF-Files](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip) -  [see here on how to install them](../docs/README-standalone.md#Fonts):

__Dictionaries:__

The executables linked  above  come with an embedded American  dictionary.  To use other
dictionaries  download the dictionary  files for your language from the relase
page: 
[https://github.com/mittelmark/microemacs/releases](https://github.com/mittelmark/microemacs/releases/tag/v0.9.0):
and place these files  in your  personal  user folder  `~/.jasspa` on Linux for
instance.  Then use "Tools -> User Setup -> Language  settings"  to switch the
dictionary.

You  should  download  the  files  for you  language  and place  them into the
ME config folder, usually _~/.jasspa_, in your home directory. On Windows check user-setup where the
ME user folder is.

## Download Prebuild MicroEmacs Executables (v09.12.26b2)


This release provides the following new features in comparison to v09.12.25:

- bugfix on Unix for cut and paste (beta2 fix)
- support for Ubuntu/Debian with Kernel 7.0 (beta2 addition)
- terminal version: support for 16 colors in the terminal
- terminal version: automatic detection and use of colors, no need for TERM=xterm anymore
- graphical version: support for clipboard on Wayland and X11 (activate in user-setup, system), on Wayland wl-clipboard package is required, on X11 xclip gives full support for primary and clipboard
- new platforms: FreeBSD 15, MacOS 26 (intel and apple chips)
- new programming/diagram languages: PlantUML, Haxe transpiler, Fusion transpiler, Groovy, Scala, Typescript
- terminal and clipboard support we programmed with aid of AI tool opencode with the Big Pickle model

> [!NOTE] 
> Please   note  that  new   developments   take  place mainly in  the
> [MicroEmacs 26](https://github.com/bjasspa/jasspa)  branch.  So new  users  might try this
> release first. This ME26 branch should contain most of the things described above
> but as well  provided  more modern fonts support (TTF and OTF) using the libxft library,
> https  support and mouse  support in the  terminal  but
> currently does not provide Cygwin build, older Linux 64 bit,  Linux 32 bit and
> FreeBSD builds (yet). 

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-ubuntu-18-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-ubuntu-18-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-ubuntu-18-microemacs-091226b2-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-fedora-28-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-fedora-28-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-i686-fedora-28-microemacs-091226b2-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-29 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-4-x86_64-almalinux-8-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-x86_64-almalinux-8-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-4-x86_64-almalinux-8-microemacs-091226b2-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 30-36  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-almalinux-9-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-almalinux-9-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-almalinux-9-microemacs-091226b2-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 27-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-almalinux-10-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-almalinux-10-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-almalinux-10-microemacs-091226b2-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-manjaro-0-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-manjaro-0-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-manjaro-0-microemacs-091226b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-18-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-18-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-18-microemacs-091226b2-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-20-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-20-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-5-x86_64-ubuntu-20-microemacs-091226b2-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-22-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-22-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-22-microemacs-091226b2-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-24-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-24-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-x86_64-ubuntu-24-microemacs-091226b2-mecwb.zip) |
|             | Ubuntu 26         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-7-x86_64-ubuntu-24-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-7-x86_64-ubuntu-24-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-7-x86_64-ubuntu-24-microemacs-091226b2-mecwb.zip) |
| Linux aarch64 | Ubuntu 22       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-22-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-22-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-22-microemacs-091226b2-mecwb.zip) |
| (Raspberry Pi)| Ubuntu 24       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-24-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-24-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/linux-6-aarch64-ubuntu-24-microemacs-091226b2-mecwb.zip) |
| MacOS       | MacOS 14/15 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-x86_64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-x86_64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-x86_64-microemacs-091226b2-mecwb.zip) |
|             | MacOS 26 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-x86_64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-x86_64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-16-x86_64-microemacs-091226b2-mecwb.zip) |
|             | MacOS 14 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-14-arm64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-14-arm64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-14-arm64-microemacs-091226b2-mecwb.zip) |
|             | MacOS 15 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-arm64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-arm64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-arm64-microemacs-091226b2-mecwb.zip) |
|             | MacOS 26 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-arm64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-15-arm64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/macos-16-arm64-microemacs-091226b2-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mecwb.zip) |
|             | FreeBSD 15 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/freebsd-14-amd64-microemacs-091226b2-mecwb.zip) |
| Windows     | Windows 10/11 (intel, arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/windows-10-intel-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/windows-10-intel-microemacs-091226b2-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.3-i686-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.3-i686-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.3-i686-microemacs-091226b2-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.6-microemacs-091226b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.6-microemacs-091226b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta2/cygwin-3.6-x86_64-microemacs-091226b2-mecwb.zip) |



## Download Prebuild MicroEmacs Executables (v09.12.26b1)


| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-29 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-4-x86_64-almalinux-8-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-x86_64-almalinux-8-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-4-x86_64-almalinux-8-microemacs-091226b1-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 30-36  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 27-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mecwb.zip) |
| Linux aarch64 | Ubuntu 22       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mecwb.zip) |
| (Raspberry Pi)| Ubuntu 24       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mecwb.zip) |
| MacOS       | MacOS 14/15 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 26 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-16-x86_64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 14 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 15 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 26 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-16-arm64-microemacs-091226b1-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecwb.zip) |
|             | FreeBSD 15 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecwb.zip) |
| Windows     | Windows 10/11 (intel, arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/windows-10-intel-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/windows-10-intel-microemacs-091226b1-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-x86_64-microemacs-091226b1-mecwb.zip) |


## Download Prebuild MicroEmacs Executables (v09.12.25)

This release provides the following new features:

- new programming languages supported: Vala, Kotlin, C#, Nim, Swift, Haskell
- new markup languages supported: AsciiDoc, Typst, Quarkdown, Tcl manual pages
- new encodings: ISO-8859-16 - Southeast Europe, CP1250, CP1251, CP1253, CP1254
- new platform: MacOS-15 intel
- improvements for Bash, Python, R, installer and font-path handling in installer
- fix: Crash on very, very long lines (thanks to Steven Phillips)
- fix: For for keys on dead key keyboard layouts

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-ubuntu-18-microemacs-091226b1-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-i686-fedora-28-microemacs-091226b1-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-29 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-4-x86_64-almalinux-8-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-x86_64-almalinux-8-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-4-x86_64-almalinux-8-microemacs-091226b1-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 30-36  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-almalinux-9-microemacs-091226b1-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 27-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-almalinux-10-microemacs-091226b1-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-manjaro-0-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-18-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-5-x86_64-ubuntu-20-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-22-microemacs-091226b1-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-x86_64-ubuntu-24-microemacs-091226b1-mecwb.zip) |
| Linux aarch64 | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-22-microemacs-091226b1-mecwb.zip) |
| (Raspberry Pi)|   Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/linux-6-aarch64-ubuntu-24-microemacs-091226b1-mecwb.zip) |
| MacOS       | MacOS 14/15 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 26 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-x86_64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-26-x86_64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 14 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-14-arm64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 15 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecwb.zip) |
|             | MacOS 26 (arm64, M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-15-arm64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/macos-26-arm64-microemacs-091226b1-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecwb.zip) |
|             | FreeBSD 15 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-15-amd64-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/freebsd-14-amd64-microemacs-091226b1-mecwb.zip) |
| Windows     | Windows 10/11 (intel, arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/windows-10-intel-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/windows-10-intel-microemacs-091226b1-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.3-i686-microemacs-091226b1-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-microemacs-091226b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-microemacs-091226b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.26.beta1/cygwin-3.6-x86_64-microemacs-091226b1-mecwb.zip) |

__Installation:__

Installation  of these  executables  is easy.  Make  them  executable  on Unix
platforms and move them to a folder  belonging to your PATH variable.  Windows
users should just copy them as well to such a folder.

Just  download an  executable  for your  platform  which matches as closely as
possible your operatig system. For instance for Fedora 39, you download the binaries for Fedora 38.
On Unix systems you make the file  executable  (chmod 755 filename) and rename
it for  instance  to me, then  copy it to a  folder  belonging  to your  PATH.
Therafter you can run the me executable.  The first thing you have to do is to
select the right  keyboard  configuration  after  starting your first session.
Choose the menu entry "Tools -> User Setup" and then  "Keyboard"  the Start-Up
tab.

[Ubuntu](https://ubuntu.com/)      builds      should     be     usable     on
[Debian](https://www.debian.org/)   and  derived   distros,   such  as  [Linux
Mint](https://www.linuxmint.com)     or    [MX     Linux](https://mxlinux.org)
compatible.     [AlmaLinux](https://almalinux.org)     builds     should    be
[CentOS](https://www.centos.org)                                           and
[RHEL](https://www.redhat.com/en/technologies/linux-platforms/enterprise-linux)
compatible.  [Fedora](https://www.fedora.org) builds can be probably only used
on Fedora without problems.

Build  for  other   platforms   might  be  provided  if  requested  using  the
[issues](https://github.com/mittelmark/microemacs/issues)    link    on   this
repository. 

### Msys2 Windows Terminal

I usually  recommend  the   [Msys2](https://www.msys2.org)   environment  for
developers if they have to use the Windows  operating  system. As the provided
Windows build is a native  Windows build, the console  version of Me09 must be
started via the cmd  Terminal on Windows. You should use in this case a bas function
like this code below which should be added to your _.bashrc_.

```bash
### add this to your .bashrc
### we assume that you copied the windows executables
### to the bin folder in your msys HOME
function me {
    if [ $1 == "-n" ]; then
        ## running terminal version
        shift 1
        cmd //C `cygpath -wa ~/bin/mec-windows.exe` "${@}"
        
    else
        `cygpath -wa ~/bin/mew-windows.exe` "${@}" &
    fi
}
```

__Fonts:__

For X11 systems on Linux and FreeBSD Use the font installer script to install a few more monsospaces TrueType fonts.

See the section <a href "#x11fonts"> X11 fonts</a> for more details.

__Dictionaries:__

The executables linked  above  come with an embedded American  dictionary.  To use other
dictionaries  download the dictionary  files for your language from the release
page: 
[https://github.com/mittelmark/microemacs/releases](https://github.com/mittelmark/microemacs/releases/tag/v0.9.0):
and place these files  in your  personal  user folder  `~/.jasspa` on Linux for
instance.  Then use "Tools -> User Setup -> Language  settings"  to switch the
dictionary.

You  should  download  the  files  for you  language  and place  them into the
ME config folder, usually _~/.jasspa_, in your home directory. On Windows check user-setup where the
ME user folder is. Alernatively there is as well and installer for these dictionaries, see the section <a href="#spelling">Spelling Dictionaries</a> for more details.


## Download Prebuild MicroEmacs Executables (v09.12.25.beta2)

This is an updated  release  providing a new  architecture:  Aarch64 for Linux
as well as a lot of new programming  languages, extended support for existing  languages and
documentation  extensions. The main new features and changes in comparison to the
2025 beta1 Release are:

- support for Linux-Aarch64
- support for Cygwin-3.3 32bit and Cygwin 3.5 and 3.6 64bit 
- support for editing  C3, Dart, Octave, Julia, Rust, V, Zig files
- improved support and documentation for C/C++. Fortran, Ada, Euphoria, Shell,
  Python, R and others 
- new command file-exec, file-format and file-lint
- starting shortcut hilights in abbreviations to simplify use of snippets
- 
| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux i686  | Ubuntu 18 / Antix 23 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-ubuntu-18-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-ubuntu-18-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-ubuntu-18-microemacs-091225b2-mecwb.zip) |
|             | Fedora 28 (32bit) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-fedora-28-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-fedora-28-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-i686-fedora-28-microemacs-091225b2-mecwb.zip) |
| Linux x86_64 | RHEL 8 / AlmaLinux 8 / Fedora 22-29 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-4-x86_64-almalinux-8-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-x86_64-almalinux-8-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-4-x86_64-almalinux-microemacs-091225b2-mecwb.zip) |
|             | RHEL 9 / AlmaLinux 9 / Fedora 30-36  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-almalinux-9-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-almalinux-9-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-almalinux-9-microemacs-091225b2-mecwb.zip) |
|             | RHEL 10 / AlmaLinux 10 / Fedora 27-42 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-almalinux-10-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-almalinux-10-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-almalinux-10-microemacs-091225b2-mecwb.zip) |
|             | Arch / Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-manjaro-0-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-manjaro-0-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-manjaro-0-microemacs-091225b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-18-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-18-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-18-microemacs-091225b2-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-20-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-20-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-5-x86_64-ubuntu-20-microemacs-091225b2-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-22-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-22-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-22-microemacs-091225b2-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-24-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-24-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-x86_64-ubuntu-24-microemacs-091225b2-mecwb.zip) |
| Linux aarch64 | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-22-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-22-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-22-microemacs-091225b2-mecwb.zip) |
|             |   Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-24-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-24-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/linux-6-aarch64-ubuntu-24-microemacs-091225b2-mecwb.zip) |
| MacOS       | MacOS 13 (intel64)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-13-x86_64-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-13-x86_64-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-13-x86_64-microemacs-091225b2-mecwb.zip) |
|             | MacOS 14 (arm64)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-14-apple-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-14-apple-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-14-apple-microemacs-091225b2-mecwb.zip) |
|             | MacOS 15 (arm64,M1..M5)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-15-arm64-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-15-arm64-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/macos-15-arm64-microemacs-091225b2-mecwb.zip) |
| FreeBSD     | FreeBSD 14 (x86_x64) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/freebsd-14-amd64-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/freebsd-14-amd64-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/freebsd-14-amd64-microemacs-091225b2-mecwb.zip) |
| Windows     | Windows 10/11 (intel32,intel64, arm)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/windows-10-intel-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/windows-10-intel-microemacs-091225b2-mewb.zip) | - |
|             | Windows Cygwin 3.3-i686 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.3-i686-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.3-i686-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.3-i686-microemacs-091225b2-mecwb.zip) |
|             | Windows Cygwin 3.5-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.5-x86_64-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.5-x86_64-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.5-x86_64-microemacs-091225b2-mecwb.zip) |
|             | Windows Cygwin 3.6-x86_64 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.6-microemacs-091225b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.6-microemacs-091225b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta2/cygwin-3.6-x86_64-microemacs-091225b2-mecwb.zip) |


## Download Prebuild MicroEmacs Executables (v09.12.25.beta1)

Release v09.12.25.beta1 (2025-04-11):

New in comparison to v09.12.24

- new functions tcl-exec, tcl-lint, tcl-format, go-format, go-lint, go-exec
- support for the Go programming language
- improvements in editing R-code, Tcl, and Python code as well
  improved handling of Markdown documents

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-debian-12-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-debian-12-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-debian-12-microemacs-091225b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-ubuntu-18-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-ubuntu-18-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux32-5-ubuntu-18-microemacs-091225b1-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/Jasspa_MicroEmacs_091225b1_x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-4-almalinux-8-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-4-almalinux-8-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-4-almalinux-8-microemacs-091225b1-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-almalinux-5-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-almalinux-9-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-almalinux-9-microemacs-091225b1-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-fedora-30-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-fedora-30-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-fedora-30-microemacs-091225b1-mecwb.zip) |
|             | Fedora 40         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-fedora-40-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-fedora-40-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-fedora-40-microemacs-091225b1-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-manjaro-0-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-manjaro-0-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-manjaro-0-microemacs-091225b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-18-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-18-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-18-microemacs-091225b1-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-20-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-20-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-5-ubuntu-20-microemacs-091225b1-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-22-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-22-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-22-microemacs-091225b1-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-24-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-24-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/linux-6-ubuntu-24-microemacs-091225b1-mecwb.zip) |
| MacOS       | MacOS 12 (intel)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-12-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-12-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-12-microemacs-091225b1-mecwb.zip) |
|             | MacOS 13 (intel)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-13-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-13-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-13-microemacs-091225b1-mecwb.zip) |
|             | MacOS 14 (arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-14-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-14-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-14-microemacs-091225b1-mecwb.zip) |
|             | MacOS 15 (arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-15-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-15-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/macos-15-microemacs-091225b1-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/freebsd-14-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/freebsd-14-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/freebsd-14-microemacs-091225b1-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/windows-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/windows-microemacs-091225b1-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.5-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.5-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.5-microemacs-091225b1-mecwb.zip) |
|             | Windows Cygwin 3.6 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.6-microemacs-091225b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.6-microemacs-091225b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.25.beta1/cygwin-3.5-microemacs-091225b1-mecwb.zip) |


## Download Prebuild MicroEmacs Executables (v09.12.24)

Release v09.12.24 (2024-12-28):

New in comparison to v09.12.24-beta3

- new functions r-exec, r-lint, r-format, py-doc, py-format, py-exec, py-lint
- improvements in editing R-code and Python code
- abbrev-list, item-list and folding for shell scripts
- folding and item-list for emf files as well supports now define-help
- limited conversion support for UTF files, using charset-ut8-to-iso and charset-iso-to-utf8, requiring iconv installed

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-debian-12-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-debian-12-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-debian-12-microemacs-091224-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-ubuntu-18-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-ubuntu-18-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux32-5-ubuntu-18-microemacs-091224-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/Jasspa_MicroEmacs_091224_x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-4-almalinux-8-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-4-almalinux-8-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-4-almalinux-8-microemacs-091224-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-almalinux-5-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-almalinux-9-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-almalinux-9-microemacs-091224-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-fedora-30-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-fedora-30-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-fedora-30-microemacs-091224-mecwb.zip) |
|             | Fedora 40         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-fedora-40-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-fedora-40-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-fedora-40-microemacs-091224-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-manjaro-0-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-manjaro-0-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-manjaro-0-microemacs-091224-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-18-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-18-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-18-microemacs-091224-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-20-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-20-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-5-ubuntu-20-microemacs-091224-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-22-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-22-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-22-microemacs-091224-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-24-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-24-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/linux-6-ubuntu-24-microemacs-091224-mecwb.zip) |
| MacOS       | MacOS 12 (intel)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-12-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-12-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-12-microemacs-091224-mecwb.zip) |
|             | MacOS 13 (intel)  | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-13-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-13-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-13-microemacs-091224-mecwb.zip) |
|             | MacOS 14 (arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-14-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-14-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-14-microemacs-091224-mecwb.zip) |
|             | MacOS 15 (arm)    | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-15-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-15-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/macos-15-microemacs-091224-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/freebsd-14-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/freebsd-14-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/freebsd-14-microemacs-091224-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/windows-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/windows-microemacs-091224-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/cygwin-3.5-microemacs-091224-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/cygwin-3.5-microemacs-091224-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24/cygwin-3.5-microemacs-091224-mecwb.zip) |

## Download Prebuild MicroEmacs Executables (v09.12.24-beta3)

Release v09.12.24b3 (beta-3) (2024-11-09):

New in comparison to v09.12.24-beta2

- new favorite entries possible for help pages for instance for MicroEmacs help or R-doc etc
- spell install now from within MicroEmacs using the spell-install macro
- improvements in editing R-code
- various fixes jeany.emf key bindings and documentation

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-debian-12-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-debian-12-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-debian-12-microemacs-091224b3-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-ubuntu-18-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-ubuntu-18-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux32-5-ubuntu-18-microemacs-091224b3-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/Jasspa_MicroEmacs_091224b3_x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-4-almalinux-8-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-4-almalinux-8-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-4-almalinux-8-microemacs-091224b3-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-almalinux-5-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-almalinux-9-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-almalinux-9-microemacs-091224b3-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-fedora-30-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-fedora-30-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-fedora-30-microemacs-091224b3-mecwb.zip) |
|             | Fedora 40         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-fedora-40-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-fedora-40-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-fedora-40-microemacs-091224b3-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-manjarolinux-0-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-manjarolinux-0-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-manjarolinux-0-microemacs-091224b3-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-18-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-18-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-18-microemacs-091224b3-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-20-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-20-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-5-ubuntu-20-microemacs-091224b3-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-22-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-22-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-22-microemacs-091224b3-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-24-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-24-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/linux-6-ubuntu-24-microemacs-091224b3-mecwb.zip) |
| MacOS       | MacOS 12          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-12-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-12-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-12-microemacs-091224b3-mecwb.zip) |
|             | MacOS 13          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-13-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-13-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-13-microemacs-091224b3-mecwb.zip) |
|             | MacOS 14          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-14-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-14-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/macos-14-microemacs-091224b3-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/freebsd-14-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/freebsd-14-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/freebsd-14-microemacs-091224b3-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/windows-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/windows-microemacs-091224b3-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/cygwin-3.5-microemacs-091224b3-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/cygwin-3.5-microemacs-091224b3-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta3/cygwin-3.5-microemacs-091224b3-mecwb.zip) |


## Download Prebuild MicroEmacs Executables (v09.12.24-beta2)

Release v09.12.24b2 (beta-2) (2024-09-28):

New in comparison to v09.12.24-beta1

- new  macro  commands  rdoc and  pydoc for  browsing  inside ME R and
  Python documentation
- writing macro help using Markdown syntax
- more editor schemes Ayu Dark, Artur, Solarized Light and Dark, Tango
  Light and Dark
- new macro  xommand  xrdb-scheme  for  loading  more  editor  schemes
  directly via xrdb files
- Unicode  support  for  terminal  version  using  luit and abduco see
  [below](#luit)

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-debian-12-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux32-5-ubuntu-18-microemacs-091224b2-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/Jasspa_MicroEmacs_091224b2_x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-4-almalinux-8-microemacs-091224b2-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-5-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-9-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-almalinux-9-microemacs-091224b2-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-fedora-30-microemacs-091224b2-mecwb.zip) |
|             | Fedora 40         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-40-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-40-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-fedora-40-microemacs-091224b2-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-manjarolinux-0-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-18-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-5-ubuntu-20-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-22-microemacs-091224b2-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/linux-6-ubuntu-24-microemacs-091224b2-mecwb.zip) |
| MacOS       | MacOS 12          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-12-microemacs-091224b2-mecwb.zip) |
|             | MacOS 13          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-13-microemacs-091224b2-mecwb.zip) |
|             | MacOS 14          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/macos-14-microemacs-091224b2-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/freebsd-14-microemacs-091224b2-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/windows-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/windows-microemacs-091224b2-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta2/cygwin-3.5-microemacs-091224b2-mecwb.zip) |


## Download Prebuild MicroEmacs Executables (v09.12.24-beta1)

Release v09.12.24b1 (beta-1) (2024-08-23):

New in comparison to v09.12.23

- adding git commands, git-add, git-commit, git-grep and others
- adding xfontsel font selection for MacOS and Linux change-font-xfontsel
- embedding libz on Windows for easier installation
- improved and bug fixed internal documentation
- new function `&llen` - for list length
- new directive `!iif` for single line if's (ported from ME 24 from Steven Phillips)
- support for TTF-files 
- support for ISO 8859-1..15 and Windows-CP1252 encodings with Euro symbol etc

| OS          | Platform          | mecb (terminal) | mewb (GUI)    | mecwb (terminal+GUI)       |
|:-----------:|:-----------------:|:---------------:|:-------------:|:--------------------------:|
| Linux 32bit | Ubuntu 16         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-16-microemacs-091224b1-mecwb.zip) |
|             | AntiX (Debian 12) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux32-5-debian-12-microemacs-091224b1-mecwb.zip) |
| Linux 64bit | AppImage          | - | - | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/Jasspa_MicroEmacs_09-x86_64.AppImage) |
|             | AlmaLinux 8       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-almalinux-8-microemacs-091224b1-mecwb.zip) |
|             | AlmaLinux 9       | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-almalinux-9-microemacs-091224b1-mecwb.zip) |
|             | Fedora 30         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-fedora-30-microemacs-091224b1-mecwb.zip) |
|             | Fedora 39         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-fedora-39-microemacs-091224b1-mecwb.zip) |
|             | Manjaro Linux     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-manjarolinux-0-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 18         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-4-ubuntu-18-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 20         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-5-ubuntu-20-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 22         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-22-microemacs-091224b1-mecwb.zip) |
|             | Ubuntu 24         | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/linux-6-ubuntu-24-microemacs-091224b1-mecwb.zip) |
| MacOS       | MacOS 12          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-12-microemacs-091224b1-mecwb.zip) |
|             | MacOS 13          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-13-microemacs-091224b1-mecwb.zip) |
|             | MacOS 14          | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/macos-14-microemacs-091224b1-mecwb.zip) |
| FreeBSD     | FreeBSD 14        | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/freebsd-14-microemacs-091224b1-mecwb.zip) |
| Windows     | Windows 10/11     | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/windows-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/windows-microemacs-091224b1-mewb.zip) | - |
|             | Windows Cygwin 3.5 | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mecb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mewb.zip) | [x](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/cygwin-3.5-microemacs-091224b1-mecwb.zip) |

Download more programmers fonts: [TTF-Files](https://github.com/mittelmark/microemacs/releases/download/v09.12.24.beta1/ttf-fonts.zip) -  [see here on how to install them](../docs/README-standalone.md#Fonts):

## Download Prebuild MicroEmacs Executables (v09.12.23)

Release Date: 2024-03-29 (v09.12.23):

   
| OS      | Platform    | (X)-Windows Binary | Terminal Binary |
|:-------:|:------------|:-------------------|:----------------|
| Linux   | AppImage    | [AppImage](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage)| Run AppImage with -n option |
|         | Ubuntu 16 (32bit)| [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu16-32bit-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu16-32bit-mecb.zip) |
|         | Antix  23 (32bit)| [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-antix23-32bit-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-antix23-32bit-mecb.zip) |
|         | Ubuntu 18   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu18-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu18-mecb.zip) |
|         | Ubuntu 20   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu20-mecb.zip) |
|         | Ubuntu 22   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-ubuntu22-mecb.zip) |
|         | Ubuntu 24   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-ubuntu24-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-ubuntu24-mecb.zip) |
|         | Fedora 30   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora30-mecb.zip) |
|         | Fedora 38   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora38-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-fedora38-mecb.zip) |
|         | Fedora 40   | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-fedora40-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09-091223-linux-fedora40-mecb.zip) |
|         | AlmaLinux 8.9 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma89-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma89-mecb.zip) |
|         | AlmaLinux 9.4 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma9_4-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_linux-alma9_4-mecb.zip) |
| macOS   | macOS 11    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-11-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_Linux-macos-11-mecb.zip) |
|         | macOS 12    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-12-mecb.zip) |
|         | macOS 14    | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_macos-14-arm-mecb.zip) |
| Windows | Win32/Win64 | [mewb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mewb.zip)| [mecb.zip](https://github.com/mittelmark/microemacs/releases/download/v09.12.23/MicroEmacs09_091223_windows-32-mecb.zip) |

## Download Prebuild MicroEmacs Executables (v09.12.21)

Release-Date: 2023-10-31 (v09.12.21):

* Linux 64bit
    * [Jasspa_MicroEmacs-x86_64.AppImage](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/Jasspa_MicroEmacs-x86_64.AppImage) -
        Linux  AppImage  which should work on Debian,  Ubuntu, Fedora and probably others 64bit Linuxes as well
    * [Linux-Fedora-38-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-fedora-38-2023-11-02.bin)
    * [Linux-Ubuntu-18-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-ubuntu-18-2023-11-02.bin)
    * [Linux-Ubuntu-20-x86_64](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-ubuntu-20-2023-11-02.bin)
* MacOS 11 64bit (should work on MacOS 12 and 13 as well)
    * [MacOS-Darwin-20-x86_64 -  console only](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mec-macos-12-2023-11-02.bin)
    * [MacOS-Darwin-20-x86_64 - console and XQuartz version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mecw-macos-12-2023-11-02.bin)
* Windows 32 and 64bit
    * Install  using the scoop  package  manager on Windows  like this  `scoop install -a 32bit https://raw.githubusercontent.com/mittelmark/microemacs/master/user/scoop-jme.json`
    * Downloading the single files:
        * [Windows window version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mew-windows-2023-11-02.exe)
        * [Windows console version](https://github.com/mittelmark/microemacs/releases/download/v09.12.21/mec-windows-2023-11-02.exe)
* Older builds for other OS are provided here [http://www.jasspa.com/](http://www.jasspa.com/)


<a name="luit">Luit and Abduco for using ME on Unicode terminals</a>
## Luit and Abduco

To use the  full  ISO-8859-XX  or  Windows-CP125X  character  sets on  Unicode
terminals you can run ME 2009 using the [Luit](https://invisible-island.net/luit/)
and [Abduco](https://github.com/martanne/abduco) terminal utilities.

Luit as a wrapper  application  translates  Unicode and ISO characters between
the Terminal  emulator and  MicroEmacs  and Abduco  allows you to suspend both
applications  together.  Instead of Abduco other supend tools like [Dtach](https://github.com/crigler/dtach) could
be used. Here a simple wrapper script which allows allows all Western ISO-8859-15 characters (like the Euro symbol) to be
used inside MicroEmacs:

```bash
#!/bin/sh
### file: mecu 
### Description: wrapper to run MicroEmacs with extended character settings
###              on UTF-8 enabled terminals
###  
### Tools required:
###   abduco: session management and detacher
###           https://www.brain-dump.org/projects/abduco/   
###   luit:   filter between non-utf-8 applications and utf-8 terminals
###           https://invisible-island.net/luit/
### Installation:
###           fedora: sudo dnf install abduco luit  
###           debian: sudo apt install abduco luit

### session name creation for the current tty 
tty=$(tty | grep -Eo '[0-9]+')
## already running? list abduco sessions
res=$(abduco -l | grep mecb$tty)

### running session, if no create an new one
### otherwise attach to the old one
### (press in ME Ctrl-l to update screen if neccesary)

if [[ "$res" == "" ]] ; then 
    ### need a new one 
   TERM=xterm abduco -A -e ^z mecb$tty luit -encoding ISO-8859-15 mecb "$@"
else
    ### attach to the old one
    abduco -a -e ^z mec$tty 
fi
```    

Change  the  filename  mecb to the  name  you  give  you  MicroEmacs  terminal
instance.  Name the bash script above `mec`, Make it executable  and move to a
folder  belonging to your PATH  variable.  With this little shell script using
two small tools, you can run MicroEmacs  nicely as well on all UTF-8 terminals
with a more  extended  character  set.  Obviously  you can as well  change the
encoding.  Here is a list of all ISO  encodings  with a short  description  of
their usable letters - [https://en.wikipedia.org/wiki/ISO/IEC_8859](https://en.wikipedia.org/wiki/ISO/IEC_8859).

## Luit aliases and X11 fonts (older versions)

For proper rendering of non  ASCII  characters for version before 0926b6 o UTF8 terminals you might
use the __luit__ tool on a Unix terminal with UTF-8 support it is usually done by performing an
alias in your `.bashrc` or your `.zshrc` like this:

```
alias mec="luit -encoding ISO8859-1 mecb"
alias mew="mewb"
```

If you like to have Windows CP1252 character support you can as well use the following `mec` alias:

```
alias mec="luit -encoding CP1252 mecb"
```

<a name="x11fonts"> </a>
**Better Font Support on X11**

For versions prior v091226b6 the  number of fonts  which  MicroEmacs  could use per  default was quite
restricted as it was using only the old XLFD-X11 font system. For a long story look here: 
[https://www.linuxdoc.org/HOWTO/XWindow-User-HOWTO-7.html](https://www.linuxdoc.org/HOWTO/XWindow-User-HOWTO-7.html)

There is as well a little  installer  script for the X11 version of MicroEmacs
which links your already  installed  existing  monospaced  TrueType fonts into
your personal folder `~/.config/share/fonts` and then indexes them there using
the mkfontscale  tool. Further the script downloads a few more free fonts like
"Source  Code Pro",  "Dejavu Sans Mono" and "Ubuntu Mono" into this folder and
the indexes these fonts as well.

You can run this script like this;

```
bash -c "$(curl -fsSL https://github.com/mittelmark/microemacs/releases/latest/download/install-fonts.sh)"
```

Therafter you might update the fontpath like this;

```
xset +fp ~/.local/share/fonts
xset fp rehash
```

To add the  fontpath  at every time you start your  system  automatically  you
might  need  to  add  these  two  lines  as  an   autostart   entry  for  your
desktop/window manager.

Here an example on what you can add to your .bashrc to add that font path after you logged in:

```
xset q | grep -A 2 Font | grep -q /.local/share/fonts || xset +fp ~/.local/share/fonts && xset fp rehash
```

> [!TIP]
> Unfortunately  some recent Linux systems like Red Hat  Enterprise 10 or Alma Linux 10 do not
> allow anymore to install these old X11 tools like `xset`. Workaround: you have in this case
> install the fonts with sudo privileges in the existing X11 font directories. On a
> Alma  Linux 10 system  for  instance I  copied  as sudo the  indexed  fonts  (using
> mkfontscale) into the directory /etc/X11/fontpath.d/fonts and they are then available as well
> for other older X11 applications.
