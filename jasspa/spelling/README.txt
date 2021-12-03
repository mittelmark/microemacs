> -!- document -!-
> This is part of the JASSPA MicroEmacs macro files
> Copyright (C) 2004-2009 JASSPA (www.jasspa.com)
> See the file me.emf for copying and conditions.
>

Place any spelling  dictionaries  in this directory. The spelling
dictionaries  can be downloaded from  http://www.jasspa.com.  The
spelling dictionaries supported at the time of writing are:

ls_dede - German spell rules, base and extended dictionaries.
ls_engb - British spell rules, base and extended dictionaries.
ls_enus - American  spell rules, base and extended  dictionaries.
ls_eses - Spanish spell rules and dictionary.
ls_fifi - Finnish spell rules and dictionary.
ls_frfr - French spell rules and dictionary.
ls_itit - Italian spell rules and dictionary.
ls_plpl - Polish spell rules and dictionary.
ls_ptpt - Portuguese spell rules and dictionary.
ls_ruye - Russian (YE) spell rules and dictionary.
ls_ruyo - Russian (YO) spell rules and dictionary.

The files  contained  within a  spelling  archive  use the naming
convention:

    lsr<Language><Country>.emf  Spelling rules.
    lsdm<Language><Country>.emf Base dictionary.
    lsdx<Language><Country>.emf Extended dictionary.

Some languages do not require an extended  dictionary. If you are
short of space  then you can delete the  extended  dictionary  to
leave  just the base  dictionary,  your word  repertoire  will be
limited.

Usage
-----
JASSPA's  MicroEmacs  supports  an  automatic  spell  mode,  this
performs  background  spell  checking  and  highlights  erroneous
words. This mode may be enabled by:-

      Help -> User Setup -> Startup -> Auto Spell 
      
Spelling   errors  are  hilighted  in  most  color   schemes.  To
correct  then move the mouse over the word and right  select, the
Auto  spell  entry  should   display  in  a  pop-up,  select  the
replacement word to correct.

You can spell the whole buffer using:

    esc-x spell-buffer
    esc-x auto-spell-buffer            

Multiple  dictionaries may be supported on the same platform. The
active dictionary may be changed from the dialog of spell buffer.

Installation
------------
    Download the spelling  dictionary  and  decompress  into this
    directory. 
        
    For a zip archive:-
    
        cd /path_to/jasspa/spelling
        unzip /path_to_archive/le_enus.zip
    
    For tar+gzip archive (gtar on some platforms):-
    
        cd /path_to/jasspa/spelling
        tar zxvf /path_to_archive/le_enus.tar.gz
