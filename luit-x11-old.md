---
title: Luit and X11 fonts for older versions
author: Detlef Groth
date: 2026-09-28 11:07
---

**Luit**


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
