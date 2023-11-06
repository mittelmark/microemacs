## Courier Prime X11-Fonts

This folder contains TTF fonts and their X11 equivalents to be used within the
Jasspa MicroEmacs. To use these fonts you can either 

A) use a folder for the Courier Prime fonts itself 
  1) copy the folder where this file is in into a folder in your home directory
  2) update your FONTPATH settings like this:
        xset +fp FOLDERNAME
        xset fp rehash

B) or add the bdf files from this folder to an folder containing as well other
   already other X11 files which you use, in this case
   1) copy all ttf and bdf files to this folder
   2) update the fonts.dir file with the following commands: 
      cd FOLDERNAME 
      mkfontscale . 
      mkfontdir . 
      xset +fp `pwd` # (if not in the path) 
      xset fp rehash

Then you can add the following line to your `username.emf` file in your folder ~/.jasspa:

```
change-font-type "courier prime-bold" "|10|14|17|19|22|25|28|33|" 22
```

If your then press `Esc x execute-buffer<RETURN><RETURN>`  the fonts should be
visible  within your  MicroEmacs  session. This needs to be done only for your
current  MicroEmacs  session.  All  new  sessions  will  load  the  last  line
automatically from your USERNAME.emf file.

To choose the fonts with medium weight use:

```
change-font-type "courier prime-medium" "|10|14|17|19|22|25|28|33|" 22
```
