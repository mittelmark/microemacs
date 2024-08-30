## Courier Prime  X11-Fonts

This folder contains TTF fonts to be used within the Jasspa MicroEmacs. To use
these fonts you can either: 

A) use a folder for the Courier Prime fonts itself 
  1) copy the folder where this file is in into a folder in your home directory
  2) update your FONTPATH settings like this:
        xset +fp FOLDERNAME
        xset fp rehash

B) or add the ttf files from this folder to an folder containing as well
   already other X11 files which you already use for fonts, in this case
   1) copy all ttf files to this folder
      cp *.ttf FOLDERNAME/
   2) update the fonts.dir file with the following commands: 
      cd FOLDERNAME 
      mkfontscale . 
      mkfontdir . 
      xset +fp `pwd` # (if not in the path) 
      xset fp rehash

The font path adaptation commands at the end should be performed automatically
after your system starts.

Finally check if the fonts are properly  installed using a font selection  tool
for X11 called `xfontsel`.
