#!/usr/bin/env tclsh

### Author      : Detlef Groth, University of Potsdam, Germany, $YEAR$
### Created     : 2023-11-12
### Name        : Convert a MicroEmacs help file (ehf) to HTML files

proc usage {} {
    global argv0
    puts "Usage: $argv0 me.ehf INDEXFILE FOLDERNAME"
}

set HEADER {---
title: __TITLE__
author: Jasspa
date: 2023-11-14
include-before: |
    <A HREF="home.htm"><IMG SRC="../images/logo.png" BORDER="0" ALIGN="BOTTOM" ALT="[me]" width="24" height="24"></IMG></A> __TITLE__ 
    <br /><P><FONT SIZE="4">
    [Home](home.htm) 
    [Commands](amicr031.htm)
    [Variables](amicr033.htm)
    [Macro-Dev](amicr034.htm)
    [Glossary](amicr035.htm)
    </FONT></P>
    
    ---
---
}

set FOOTER {
---

*Copyright (c) 1998-2010 JASSPA, 2021-2023 Detlef Groth*<br />
*Last Modified: 2023/11/13*<br />
*Generated On: 2023/11/13*
}
proc links {} {
    uplevel 1 {
        while {[regexp {\033ls\033lm.+\033le} $line]} {
            regexp {\033ls\033lm(.+?)\033le} $line -> nlink
            set nlink [regsub {MicroEmacs - } $nlink ""]
            set nlink [regsub {&} $nlink {AMPERS}]
            if {[regexp {^[a-z].+\(5\)} $nlink]} {
                set nlink "\$$nlink"
            }
            if {![info exists index($nlink)]} {
                puts "No link in $cfile for nlink '$nlink'"
                exit
            }
            set url $index($nlink) ;#[zlib crc32 $nlink].htm
            set line [regsub {\033ls\033lm(.+?)\033le} $line "\[$nlink\](${url})"]
        }
        while {[regexp {\033ls.+\033lm.+\033le} $line]} {
            regexp {\033ls(.+?)\033lm(.+?)\033le} $line -> url nlink
            set nlink [regsub {AMPERS} $nlink {\&}]
            set nlink [regsub {MicroEmacs - } $nlink ""]
            set nlink [regsub {^faq} $nlink "ffaq"]
            set nlink [regsub {^FAQ([0-9])} $nlink "ffaq\\1"]
            if {[regexp {^[a-z].+\(5\)} $nlink]} {
                set nlink "\$$nlink"
            }
            if {[info exists index($url)]} {
                set url $index($url)
            } elseif {![info exists index($nlink)]} {
                puts "No link in $cfile for nlink '$nlink'"
                exit
            } else {
                set url $index($nlink) ;#[zlib crc32 $nlink].htm
            }
            set line [regsub {\033ls(.+?)\033lm(.+?)\033le} $line "\[$nlink\](${url})"]
        }
        set line [regsub {AMPERS} $line {\&}]
    }
}

proc process {filename indexfile dirname} {
    array set index [list]
    if [catch {open $indexfile r} infh] {
        puts stderr "Cannot open $filename: $infh"
        exit
    } else {
        while {[gets $infh line] >= 0} {
            if {[regexp {^(.+)\t(.+)} $line -> h t]} {
                set t [regsub {&} $t {AMPERS}]
                set index($t) $h
                set t [regsub {\([0-9mf]+\)} $t ""]
                set index($t) $h
            }
        }
        close $infh
    }
    if [catch {open $filename r} infh] {
        puts stderr "Cannot open $filename: $infh"
    } else {
        set link false
        set pages [list]
        set d [dict create]
        set crc null
        set out ""
        set txt false
        set p false
        set pre false
        set ul false
        set elines 0
        if {![file isdirectory $dirname]} {
            file mkdir $dirname
        }
        set x 0
        set cfile ""
        while {[gets $infh line] >= 0} {
            if {[regexp  {^! ([0-9mf]{1,2})\s*(.+?)\s*$} $line -> sec title]} {
                ## name check 1
                if {!$link} {
                    # string to upper?
                    set title [regsub {MicroEmacs - } $title ""]
                    set title [regsub {^faq} $title "ffaq"]
                    set title [regsub {^FAQs} $title "fFAQs"]
                    set title [regsub {^&} $title {AMPERS}]
                    if {![info exists index($title)]} {
                        puts "No link for here '$title'"
                        exit
                    }
                    #set crc [zlib crc32 $title]
                    set crc $index($title) 
                    set link true
                    lappend pages $title
                } 
            } elseif {[regexp  {^! \s*(.+?)\s*$} $line ->  title]} {
                ## name check 2
                if {!$link} {
                    # string to upper?
                    set title [regsub {MicroEmacs - } $title ""]
                    set title [regsub {^&} $title {AMPERS}]                    
                    if {![info exists index($title)]} {
                        puts "No link for '$title'"
                        exit
                    }
                    set crc $index($title)
                    #set crc [zlib crc32 $title]
                    set link true
                    lappend pages $title
                } 
            } elseif {$link && [regexp {^.cE.cE([^\033]+?)\s*.c} $line -> page]} {
                ## start help section
                set elines 0
                set link false
                set txt true
                if {$out ne ""} {
                    puts $out $::FOOTER
                    close $out
                }
                if {$page eq "NAME"} {
                    set tpage [lindex $pages 0]
                } else {
                    set tpage $page
                }
                set cfile [regsub {.htm$} [file join $dirname ${crc}] ".md"]
                set out [open $cfile w 0600]
                puts $out [regsub -all {__TITLE__} $::HEADER [regsub {@} [regsub {!} [regsub "%" $tpage "\\%"] "\\!"] "\\@"]]
                puts $out "## $page"
                set pages [list]
                continue
            } elseif {$txt && [regexp {^.cE.cE([^\033]+?)\s*\033} $line -> page]} {
                ## start help section
                set elines 0
                puts $out "## $page"
                continue
            } elseif {$txt && [regexp {^\033cE([^\033]+)\s*\033} $line -> page]} {
                # header in help section
                puts $out "## $page"
                continue
            } elseif {$txt && [regexp {^\033sB} $line]} {
                # MicroEmacs code in help section
                puts $out "```{.emf}"
                set pre true
                continue
            } elseif {$txt && $pre && [regexp {^\033sA} $line]} {
                # MicroEmacs code in help section
                puts $out "```\n"
                set pre false
                continue
                
            } 
            if {[regexp {^\s+[^\s]} $line]} {
                links
                set line [regsub {^\s{5,10}\[} $line "> - \["]
                set line [regsub {^\s{0,4}\[} $line "- \["]
            }
            if {$txt && !$pre && $elines > 2 && [regexp {^[\s]{10,}[^\s]} $line]} {
                set line [regsub -all {\033c[A-G]} $line ""]
                puts $out $line
                set elines 0
                set pre true
            } elseif {$txt && !$pre && [regexp {^[\s]{1,8}[^\s]} $line]} {
                set line [regsub {^   } $line ""]
                set line [regsub {^ {0,2}\* } $line "- "]
                set line [regsub {^ {2,10}\* } $line "    - "]
                set line [regsub -all {.cD(.+?).cA} $line "__\\1__"]
                set line [regsub -all {.cG(.+?).cA} $line "`\\1`"]                
                set line [regsub -all {\033cC(.*?)\033cA} $line "_\\1_"]                
                set line [regsub -all {\033c[A-G]} $line ""]                
                #set line [regsub -all {<} $line {&lt;} $line]
                #set line [regsub -all {>} $line {&gt;} $line]
                #set line [regsub -all {&} $line {AMPERS}]
                #set line [regsub {AMPERS} $line {&amp;}]
                #set line [regsub {^\s{0,12}\[} $line "- \["]
                puts $out "$line"
            } elseif {$txt && ![regexp {^!} $line]}  {
                set line [regsub -all {\033c[A-G]} $line ""]
                puts $out $line
            }
            if {[regexp {[^\s]+} $line]} {
                set elines 0
            } else {
                incr elines
            }
        }
        close $infh
        if {$out ne ""} {
            puts $out $::FOOTER
            close $out
        }
    }
}
proc help {} {
    global argv0
    puts "Convert MicroEmacs help file to HTML files"
    puts "Detlef Groth, 2023"
    usage
    
}
proc main {argv} {
    if {[llength $argv] < 3} {
        usage
    } elseif {[llength $argv] > 0 && [list -h --help] in $argv} {
        help
    } else {
        if {![file exists [lindex $argv 0]]} {
            puts "Error: File [lindex $argv 0] does not exists!"
        } elseif {![file exists [lindex $argv 1]]} {
            puts "Error: File [lindex $argv 1] does not exists!"
        } else {
            process [lindex $argv 0] [lindex $argv 1] [lindex $argv 2]
        }
    }
    
}
# cEcEMICROEMACS cEcA
if {[info exists argv0] && [info script] eq $argv0} {
    main $argv
}
    

