---
title: Tutorial about the Jasspa MicroEmacs EMF macro language
author: Detlef Groth, Caputh-Schwielowsee, Germany
date: 2026-10-29 14:30
abstract: >
    This is a short introduction to the MicroEmacs macro language.
---

## Introduction

This tutorial introduces the [Jasspa MicroEmacs](http://www.jasspa.com) macro
language. It covers variables, data structures, control flow, and functions,
then walks through examples of useful extensions.

Runnable examples use executable shell fences: `tmdoc` writes each snippet to
an EMF file, runs it with `mecb`, and includes the output beneath the code.
The helper macro in `tutorial.emf` is loaded from this directory. Interactive
examples and snippets that depend on the current editor state or user-specific
configuration remain ordinary EMF code blocks; try those by selecting them
and running `execute-region`, or by saving them in a macro file and loading
that file.

## Hello World

In the editor, you can execute:

```emf
ml-write "Hello World!"
```

This displays the message in the editor's command line at the bottom of the
screen, as shown here:

![](../images/hello-world.png)

To run the command, press `Esc-x`, enter `ml-write`, press Enter, and then
enter `Hello World!`. Alternatively, type the code in the editor, select the
line and a following blank line, then run `execute-region` with `Esc-x`.

## Executing ME scripts

This standalone example shows the executable-fence syntax:

```{.shell cmd="mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
-1 ml-write "Hello ME-World!"
exit-emacs
```

## Variables

For more information about variables, see the [Variables help
page](https://www.dgroth.de/me2009/me/m4fil018.htm).

The main variable types are:

- Register variables begin with `#` and provide fast access to values.
- System and editor variables begin with `$`; system variables hold
  configuration and state, while editor variables describe the current
  editor, buffer, or window.
- Command variables usually begin with `.` and are used within macros.
- Global macro variables usually begin with `%`.
- Buffer variables usually begin with `:` and hold information about the
  current buffer.

Variables are usually assigned with `set-variable`. Here is an example:

```emf
; make it easier to print information
define-macro print
    -1 ml-write @1
!emacro

print "Hello Print World!"
set-variable %test "This is a test!"
set-variable #g1 "This is a global register variable!"
print %test
print #g1
; some editor variables
print $buffer-bname
print $progname
print $version
```

Each executable example runs in its own process. The next example therefore
cannot use the `print` macro unless it loads the macro first.

```emf
print "test"
```

Your user startup file is usually located in your JASSPA home directory. The
following variables can help you find your user paths:

```emf
-1 ml-write $user-path
-1 ml-write $user-name
```

Use `$user-path` to locate your macro directory, then save the `print` macro
in a file such as `tutorial.emf`. Load it at the start of each script:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print "Hello Tutorial!"
exit-emacs
```

Now, back to variables.

### Register variables

Global register variables begin with `#g` followed by a number from 1 to 9.
They provide fast access to values that are visible across macro calls:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #g1 "Value of #g1 is a string!"
set-variable #g2 "Another value!"
define-macro test-global
    print #g1
!emacro 
test-global
print #g2
exit-emacs
```

Local registers use the `#l` prefix, followed by a number from 1 to 9.

Here is an example:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
define-macro test-local
    set-variable #l1 "Local variable string!"
    print #l1
!emacro 
test-local
set-variable %test &spr "#l1 is '%s'" #l1
print %test
exit-emacs
```

As you can see, register variables default to an empty string.

There are also register variables with the prefix `#p`. These are visible in
the calling (parent) macro. Here is an example:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
define-macro child-mac
    ; set a local variable in the caller
    set-variable #p1 "Hello parent!"
!emacro
define-macro parent-mac
    child-mac
    print #l1
!emacro
parent-mac
exit-emacs
```

This can be helpful in some situations, although it can also make the code a
little harder to understand.

### System variables


As noted above, system variables begin with `$` and hold information about
MicroEmacs and its environment. To display them, run `list-variables` from the
editor with `Esc-x`, or use the default key combination `C-h v`. This example
displays a few variables and then inspects the local `tutorial.emf` buffer:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print &sprintf "variable: $auto-time    = %s" $auto-time  
print &sprintf "variable: $buffer-bname = %s" $buffer-bname
print &sprintf "variable: $not-existing = %s" $not-existing
; open the local helper file in a buffer
find-file "tutorial.emf"
print &sprintf "variable: $buffer-bname = %s" $buffer-bname
; $buffer-fname contains an installation-specific absolute path
; count the lines in this file
end-of-buffer ; jump to the end
print &sprintf "variable: $window-aline = %s" $window-aline
exit-emacs
```

If you access a variable that does not exist, MicroEmacs returns the string
`ERROR`. You can use this value to check whether a variable exists:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable %real-var "hello"
set-variable #l2 %real-var
set-variable #l3 %dummy-var
; check whether the variable value is ERROR (&seq means string equality)
!if &seq #l2 "ERROR"
    print "variable %real-var does not exist!"
!else
    print "variable %real-var exists!"
!endif
!if &seq #l3 "ERROR"
    print "variable %dummy-var does not exist!"
!else
    print "variable %dummy-var exists!"
!endif
exit-emacs
```


## Data structures


MicroEmacs provides the following data structures:

- scalars
- lists

A scalar is a single value. By default, MicroEmacs has one composite data
structure: the list. ME recognizes a list when a variable contains
pipe-delimited values. Here is an example:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l1 "|this|is|a|list|"
print &lget #l1 1
print &lget #l1 2
exit-emacs
```

As you can see, `&lget` retrieves an element from a list. The following
functions are also available:

- `&ldel list idx` - delete an element from a list
- `&lfind list value` - find an element's position in a list
- `&linsert list index value` - insert an element at a specified position
- `&lset list index value` - set an element at a specified position

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l1 "|this|is|a|list|"
set-variable #l1 &linsert #l1 1 "start"
print &lget #l1 1
print &lget #l1 2
exit-emacs
```

Accessing and modifying lists can be relatively slow, so avoid storing large
amounts of data in them.

For illustration, let's create another data structure: a dictionary that
supports key-value access. Functions beginning with `&` are implemented in C.
To add similar functionality at the macro level, we will define two macros
that store dictionary data in a hidden buffer named `:*dict*DICTNAME:`.

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
define-macro dict
   set-variable #l1 &spr ":*dict*%s:" @1
   set-variable #l2 @2
   !force set-variable #l3 @3
   !if $status
     !if &not &exi &ind #l1
       set-position "\x82"
       find-buffer &mid #l1 1 -1
       buffer-mode "hide"
       !force goto-position "\x82"
     !endif
     set-variable &ind &cat #l1 #l2 &set .value #l3
   !else
     set-variable .value &ind &cat #l1 #l2
   !endif
!emacro
define-macro dict-get
    set-variable @3 &ind &spr ":*dict*%s:%s" @1 @2
!emacro

; we can now set dictionary values
dict "aa" "Cys" "Cystein"
dict "aa" "Ala" "Alanin"
; and we can retrieve these values later
dict-get "aa" "Cys" #g0
print #g0
dict-get "aa" "Ala" #g0
print #g0
exit-emacs
```

The third argument names the variable in which to store each retrieved value.
Steven Phillips kindly provided this dictionary implementation as an example.
As you can see, MicroEmacs is flexible and can be extended with custom macros.
We will discuss this topic in more detail later.

## Operators and Functions

Operators and functions are implemented in C. They can be grouped by the kind
of data they handle:

- boolean data
- numerical data
- string data

To see which functions are available, consult the Macro Language Glossary.
Open it from the `Help > General Help` menu, then browse to `Macro-Dev`.

### Numerical functions

Here are some examples of numerical functions:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 -12
set-variable #l1 &abs #l0   ; absolute value
print #l0
print #l1
print &add #l1 1            ; add 1
print &div #l1 4            ; divide by four
set-variable #l2 &inc #l1 1 ; increment variable
print #l2
exit-emacs
```

### String functions

Here are a few examples of string functions:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 "Hello Murks"
print &rep #l0 "Murks" "World!"      ; replace a string
print &cat &cat "Hello " "World" "!" ; concatenate a string
print &rig "Hello World!" 6          ; right characters
print &lef "Hello World!" 6          ; left characters
print &mid "Hello World!" 6 8        ; middle characters
print &slo "Hello World!"            ; lowercase
print &sup "Hello World!"            ; uppercase
print &len "Hello World!"            ; length of string
print &trb " Hello World!  "         ; trim string - remove whitespace
exit-emacs
```

You can combine these functions in different ways:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 &rep &trb &cat " Hello " "Murks    " "Murks" "World!"
print #l0
exit-emacs
```

In the example above, `&cat` first concatenates `" Hello "` and
`"Murks    "`. Then `&trb` removes whitespace from both ends, and `&rep`
replaces `"Murks"` with `"World!"`. This is equivalent to:

`rep(trim(cat(" Hello ", "Murks    ")), "Murks", "World!")`

### UTF-8 strings

MicroEmacs supports UTF-8 natively and keeps the encoding per buffer. It
auto-detects file encodings; use `mec -E utf-8 file.txt` to force a buffer to
UTF-8 when needed. See [UTF-8 Encoding](../doc/utf8-encoding.md) for details.

The traditional string functions such as `&len` and `&mid` count bytes. The
`&u` variants count valid UTF-8 sequences as characters; their positions are
1-based. These functions count encoded characters, not user-perceived
graphemes, so a base letter followed by a combining mark may count as two.

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 "αβγ"
print &len #l0          ; 6 bytes in UTF-8
print &ulen #l0         ; 3 characters
print &uleft #l0 2      ; αβ
print &umid #l0 2 1     ; β
exit-emacs
```

### Operators

Let's start with numerical operators:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print &equal 3 3   ; equal ? 1 true
print &equal 3 4   ;         0 false
print &greater 4 3 ; greater 4 > 3
print &greater 3 4 ; greater 3 > 4
print &less    3 4 ; less: 3 < 4

exit-emacs
```

Now a few string operators and functions:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print &seq "Hello" "World!"     ; should return 0 / false
print &seq "Hello" "Hello"      ; should return 1 / true
print &seq "HELLO" &sup "Hello" ; should return 1 / true
print &isequal "HELLO" "HeLLo"  ; case-insensitive comparison
print &isin "HE" "Hello"        ; case-insensitive check
print &sin  "HE" "Hello"        ; case-sensitive check
exit-emacs
```

There are also string functions and operators that use regular expressions:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print &xse "Hello World!" "H.+!"   ; 1 true
print &xse "file.cpp" ".+\\.cp?p?"
print &xse "file.c"   ".+\\.cp?p?"
print &xse "file.h"   ".+\\.cp?p?" ; 0 false
exit-emacs
```

These examples illustrate how to manipulate numbers and strings with the
MicroEmacs macro language.

## Control flow

MicroEmacs also provides the usual control structures:

- `!if`, `!elif`, `!else`, `!endif`
- `!while` with `!done`
- `!repeat` with `!done` or `!until`
- `!break` and `!continue`

First, let's look at how to read the current date and time. We will use this
later in an example that greets the user:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
print $time
print &cat "Year:  " &mid $time 0 4
print &cat "Month: " &mid $time 7 2 
print &sprintf "Month: %02d" &mid $time 7 2 ; leading zero
print &cat "Day:   " &mid $time 9 2  
print &cat "Wday:  " &mid $time 11 1 ; 0=Sun
; let's create a list and extract the three-letter day abbreviation
set-variable %weekdays "|NULL|Sun|Mon|Tue|Wed|Thu|Fri|Sat|"
print &cat "Wday:  " &lget %weekdays &mid $time 11 1
print &sprintf "Hour:   %02d" &mid $time 12 2 
print &sprintf  "Min:    %02d" &mid $time 14 2 
exit-emacs
```

Let's check the current hour and greet the user accordingly:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 &mid $time 12 2  ; the current hour
print &sprintf "Current hour is: %02d" #l0
!if &gre #l0 4 &and &less #l0 11  ; from 5 to 10
    print "Good Morning!"
!elif &gre #l0 10 &and &less #l0 15
    print "Good Day!"
!elif &gre #l0 14 &and &less #l0 18    
    print "Good Afternoon!"
!elif &gre #l0 17 &and &less #l0 23    
    print "Good Evening!"
!else
    print "Good Night!"
!endif
print &lget "|01|02|03|04|" 2
exit-emacs
```

An `if` statement starts with `!if`; additional conditions use `!elif`, and an
optional fallback uses `!else`. Every conditional block ends with `!endif`.
The MicroEmacs editor can indent these blocks automatically.

Instead of these compound conditions, we can use `&lfind`, which returns 0
when an element is not in the list. The following example uses it to implement
the same greeting logic. Because 0 is false, the result can be used directly
as an `!if` condition.

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
; the current hour, with a leading zero in the morning
set-variable  #l0  &sprintf  "%02d" &mid $time 12 2  
print #l0
print &lfind "|05|06|07|08|09|10|" #l0 ; checking &lfind
!if &lfind "|05|06|07|08|09|10|" #l0 
    print "Good Morning!"
!elif &lfind "|11|12|13|14|" #l0
    print "Good Day!"
!elif &lfind "|15|16|17|" #l0
    print "Good Afternoon!"
!elif &lfind "|18|19|20|21|22|" #l0
    print "Good Evening!"
!else
    print "Good Night!"
!endif
exit-emacs
```

In some cases, using `&lfind` can make the code easier to read.

Let's continue with loops. ME provides `!while` and `!repeat`, but no
C-style `for` loop. A counting loop can be written with `!while`:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0  ; initialization
!while &less #l0 8  ; condition 
    set-variable #l0 &inc #l0 1 ; increment
    print #l0
!done
exit-emacs
```

Use `!continue` to skip the rest of a loop iteration. The following variation
prints only even numbers by using the `&mod` operator.

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0
!while &less #l0 8 
    set-variable #l0 &inc #l0 1 
    !if &mod #l0 2 ; skip odd numbers
        !continue
    !endif  
    print #l0
!done
exit-emacs
```

The `!repeat` directive has no condition at its start, so use `!break` to end
it explicitly, or `!until` to test a condition at the end of each iteration.
Here is an example using `!break`:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0
!repeat
    set-variable #l0 &inc #l0 1
    print #l0
    !if &gre #l0 7
        !break
    !endif    
!done
exit-emacs
```

Alternatively, `!until` checks a condition at the end of each `!repeat`
iteration:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0
!repeat
    set-variable #l0 &inc #l0 1
    print #l0
!until &gre #l0 7
exit-emacs
```

Current versions support three levels of `!while` and `!repeat` nesting,
including combinations of the two loop types. The following examples each use
two levels and count six iterations:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0  ; outer loop
set-variable #l2 0  ; total iterations
!while &less #l0 3
    set-variable #l0 &inc #l0 1
    set-variable #l1 0  ; inner loop
    !while &less #l1 2
        set-variable #l1 &inc #l1 1
        set-variable #l2 &inc #l2 1
    !done
!done
print #l2 ; 6
exit-emacs
```

The same iteration count can be expressed with nested `!repeat` loops:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
set-variable #l0 0  ; outer loop
set-variable #l2 0  ; total iterations
!repeat
    set-variable #l0 &inc #l0 1
    set-variable #l1 0  ; inner loop
    !repeat
        set-variable #l1 &inc #l1 1
        set-variable #l2 &inc #l2 1
    !until &gre #l1 1
!until &gre #l0 2
print #l2 ; 6
exit-emacs
```

`!goto` and `!tgoto` are also available for jumps to labels; see the
MicroEmacs help pages for their details.

## Macros as functions


After covering the basics of MicroEmacs macro programming, we can create
macros that act like functions. This section introduces:

- `define-macro` - define a macro/function
- `define-help` - define help for a macro/function

Macros in MicroEmacs cannot return values. However, you can pass variable
names as arguments and modify those variables inside the macro. Here is an
example that squares a value and stores the result in the specified variable:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
define-macro square-value
   set-variable @2 &mult @1 @1
!emacro
square-value 5 #l0
print #l0
exit-emacs
```

The first argument to a macro is `@1`, the second is `@2`, and so on. Although
macros cannot return values, you can pass variable names and store results in
those variables.

Macro behavior can be modified by preceding a call with a numeric argument.
For example, `-1 ml-write "message string"` sends the message to the terminal
rather than to the editor. Here is a macro that checks for a numeric argument:

```{.shell cmd="env MEPATH=. mecb -p @./%b > %b.txt" chunk.ext=emf ext=txt}
execute-file "tutorial.emf"
define-macro check-num
    !if @?
        ; numeric argument given
        print &sprintf "Given: %d" @#
    !else
        print "No numeric argument given"
    !endif
!emacro
check-num
100 check-num
-5 check-num
exit-emacs
```

For more information, consult the MicroEmacs help page for `define-macro`.

## Working with files and buffers

Let's finish with a macro that extracts this document's headings and displays
them with their line numbers. We will then adapt it to collect the headings in
a buffer named `toc.txt`.

```emf
find-file emf-tutorial.md
beginning-of-buffer
-1 ml-write $buffer-bname
; let's now search for lines starting with two hashes
!force  search-forward "^##"
!while $status
    forward-char
    -1 ml-write &sprintf "line: %03d %s" $window-line @wl 
    !force search-forward "^##"
!done
```

Let's extend the example to insert the headings into a buffer named `toc.txt`.

```emf
find-file "emf-tutorial.md"
find-file "toc.txt"
beginning-of-buffer
set-mark
end-of-buffer
exchange-point-and-mark
kill-region
find-buffer "emf-tutorial.md"
beginning-of-buffer
; let's now search for lines starting with two hashes
!force search-forward "^##"
!while $status
    forward-char
    -1 ml-write &sprintf "line: %03d %s" $window-line @wl 
    set-variable #l0 &sprintf "line: %03d %s\n" $window-line @wl 
    find-buffer "toc.txt"
    insert-string #l0
    find-buffer "emf-tutorial.md"
    !force search-forward "^##"
!done
```

## Developing your own macros

After learning a few basics, you may want to create your own macros.

You can add macros to your user startup file, usually named `USERNAME.emf`
and located in your MicroEmacs home directory. Let's add a simple macro that
deletes every line beginning with specified text. This is useful when you
need to clean up text in a particular format.

Here is an example file:

```
some text
delete me because I am not required
some text with other lines
delete me but with some other text
some text within other lines
```

Let's create a macro that deletes every line in the current buffer beginning
with `"delete me"`. First, add a simple message to the macro. Open your
`USERNAME.emf` file and add the following code:


```emf
define-macro remove-delete-me
    1000 ml-write "Macro is here"
!emacro
```

Select the three macro lines and run `execute-region` (`Esc-x`, then
`execute-region`). The `remove-delete-me` command is now available. Run it by
pressing `Esc-x` and entering `remove-delete-me`. You can also use tab
completion; entering `remo` and pressing Tab should be sufficient. The
message then appears in the editor.

You can modify the code, select it again, and run `execute-region` to update
the macro. Let's change it and try again:

```emf
define-macro remove-delete-me
    1000 ml-write "Macro is here again!!"
!emacro
```

You can extend the macro step by step this way. These examples are written
directly in Markdown, but the finished macro should be saved in an EMF file.
Placing it in `USERNAME.emf` ensures that it is loaded each time MicroEmacs
starts.

Here is the completed macro:

```emf
define-macro remove-delete-me
    ; stepwise debugging
    set-variable $debug 2
    set-alpha-mark "p"
    !force  search-forward "^delete me"
    !while $status
        kill-line
        !force search-forward "^delete.me"
    !done
    goto-alpha-mark "p"    
    1000 ml-write "Macro was executed!!"
    set-variable $debug 0
!emacro
```

Here is an example of text to which we can apply the macro:

```
hello 1
delete me 1
hello 2
delete me 1
```

We set a debugging variable near the top to step through the macro. This helps
identify problems. In this example, we will see that the macro must move to
the beginning of each line before deleting it:

```emf
define-macro remove-delete-me
    set-alpha-mark "p"
    !force  search-forward "^delete me"
    !while $status
        beginning-of-line
        kill-line ; cleans the line
        kill-line ; the now-empty line
        !force search-forward "^delete.me"
    !done
    goto-alpha-mark "p"    
!emacro
```

Placing the cursor above the text to be manipulated allows us to remove these
lines.

```
<CURSOR>
hello 1
delete me 1
hello 2
delete me 2
```

After executing `remove-delete-me`, the buffer contains:

```
<CURSOR>
hello 1
hello 2
```

This is a simple example, but it shows how to start writing your own macros.
Consult the embedded MicroEmacs help pages (`Esc-?`) for more information.

MicroEmacs also includes a macro recorder: press `Ctrl-x (` to start
recording, `Ctrl-x )` to stop, and `Ctrl-x e` to execute the last recorded
macro.

## Formal Grammar Reference

For a formal BNF (Backus-Naur Form) grammar of the EMF macro language, see
[doc/emf-bnf.md](../doc/emf-bnf.md). The grammar specifies the complete syntax,
including token types, expressions, directives, and block structure.

Key grammar rules:
- Programs are sequences of lines (blank, comment, command, directive, or label)
- Every line is self-contained (no line continuation with `\`)
- Prefix notation: functions appear before arguments (`&add 1 2`)
- All values are strings; numeric conversion happens automatically
- Comments start with `;` and extend to end of line
- Labels start with `*` at beginning of line

## Document generation

Build the tutorial HTML from this Markdown source with `make` in the `docs`
directory. `tmdoc` executes the runnable examples with `mecb`; `mndoc` then
produces the HTML with the styles in `tmdoc.css`. The remaining ordinary EMF
code blocks are intentionally not executed.

```
cd docs
make
```

## TODO

Possible topics for future tutorial extensions:

- Macro execution and control
  - Explain error handling with `$status`, `!force`, and conditional checks.
  - Expand the discussion of macro arguments, variable scope, and `define-help`.
- Editor interaction and automation
  - Show how to prompt users, build dialogs, and define key bindings.
  - Develop a complete file-and-buffer editing example, including saving results.
- Testing and debugging
  - Demonstrate a repeatable workflow for testing macros with `mecb`.
  - Cover common debugging techniques and how to interpret macro errors.
- Future language features
  - Add list iteration examples if a `foreach` construct is introduced.
  - Document key-value lists if that data structure is added to the language.
