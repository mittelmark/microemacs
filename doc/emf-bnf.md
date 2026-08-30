---
title: Jasspa MicroEmacs EMF Macro Language - Formal BNF Grammar
author: Detlef Groth, Caputh-Schwielowsee, Germany, MiMo 2.5
date: 2026-08-29
abstract: >
    This document provides a formal BNF (Backus-Naur Form) grammar specification 
    for the JASSPA MicroEmacs macro language (EMF). It is derived from the 
    documentation in `doc/me.smd` and serves as a reference for understanding 
    the language structure.
---

## Overview

MicroEmacs  macro  language  uses  **prefix  notation**  (Polish  notation)  -
operators appear before their arguments. Every line is self-contained (no line
continuation). The first character of a token determines its type.

## Token Types

```
;  -> Comment
$  -> System or environment variable
%  -> User variable
:  -> Buffer variable
.  -> Command variable
#  -> Register variable
@  -> Macro variable
!  -> Directive
&  -> Function
"  -> String literal
0-9 -> Number
*  -> Goto label
```

## BNF Grammar

### Programs and Files

```bnf
<program>         ::= <line>*
<line>            ::= <comment-line>
                    | <blank-line>
                    | <command-line>
                    | <directive-line>
                    | <label-line>

<blank-line>      ::= WS*
<comment-line>    ::= WS* ';' <comment-text> NEWLINE
<label-line>      ::= '*' <identifier> NEWLINE
```

### Comments

```bnf
<comment-text>    ::= <any-char-except-newline>*
```

### Identifiers

```bnf
<identifier>      ::= <alpha> <alphanumeric>*
<alphanumeric>    ::= <alpha> | <digit>
<alpha>           ::= 'a'..'z' | 'A'..'Z' | '_' | '-'
<digit>           ::= '0'..'9'
```

### Numbers

```bnf
<number>          ::= <decimal-number>
                    | <hex-number>
                    | <octal-number>

<decimal-number>  ::= <digit>+
<hex-number>      ::= '0' 'x' <hex-digit>+
<octal-number>    ::= '0' <octal-digit>+
<hex-digit>       ::= <digit> | 'a'..'f' | 'A'..'F'
<octal-digit>     ::= '0'..'7'
```

### Strings

```bnf
<string>          ::= '"' <quoted-chars> '"'
<quoted-chars>    ::= <quoted-char>*
<quoted-char>     ::= <escape-seq>
                    | <any-char-except-quote-backslash>

<escape-seq>      ::= '\\'    ; literal backslash
                    | '\t'    ; TAB
                    | '\n'    ; NEWLINE
                    | '\x' <hex-digit> <hex-digit>  ; hex char

<unquoted-string> ::= <non-special-char>+
<non-special-char>::= <alphanumeric> | <special-chars>
<special-chars>   ::= '.' | ',' | ':' | ';' | '+' | '-' | '='
                    | '(' | ')' | '[' | ']' | '{' | '}' | '/'
```

### Variables

```bnf
<variable>        ::= <system-var>
                    | <user-var>
                    | <buffer-var>
                    | <cmd-var>
                    | <register-var>
                    | <macro-var>
                    | <indexed-var>

<system-var>      ::= '$' <identifier>
<user-var>        ::= '%' <identifier>
<buffer-var>      ::= ':' <identifier>
                    | ':' <identifier> ':' <buffer-name>
<cmd-var>         ::= '.' <identifier>
                    | '.' <identifier> '.' <identifier>

<register-var>    ::= '#' 'l' <digit>    ; local (0-9)
                    | '#' 'g' <digit>    ; global (0-9)
                    | '#' 'p' <digit>    ; parent (0-9)

<macro-var>       ::= '@' <macro-var-char>
<macro-var-char>  ::= '0'..'9' | '#' | '?' | 'p'

<indexed-var>     ::= <variable> '[' <expression> ']'
```

### Expressions

```bnf
<expression>      ::= <string>
                    | <number>
                    | <variable>
                    | <function-call>
                    | <expression> <binop> <expression>

<function-call>   ::= '&' <identifier> <arg-list>
<arg-list>        ::= <expression>*

<binop>           ::= <arithmetic-op>
                    | <comparison-op>
                    | <logical-op>
                    | <string-op>
                    | <bitwise-op>
                    | <list-op>

<arithmetic-op>   ::= '+' | '-' | '*' | '/' | '%'  ; add/sub/mul/div/mod
<comparison-op>   ::= '=' | '<' | '>' | '<=' | '>=' ; equal/less/great/etc
<string-op>       ::= '=' | '<' | '>' | '~'          ; seq/sless/sgreat/sin
<logical-op>      ::= '&' | '|' | '!'
<bitwise-op>      ::= '&' | '|' | '^' | '~'          ; band/bor/bxor/bnot
<list-op>         ::= 'find' | 'get' | 'set' | 'del' | 'insert'
```

### Commands

```bnf
<command-line>    ::= WS* <command> <arg-list> NEWLINE

<command>         ::= <identifier>       ; built-in command name

<arg-list>        ::= <arg>*
<arg>             ::= <expression>
```

### Directives

```bnf
<directive-line>  ::= WS* <directive> NEWLINE

<directive>       ::= <define-macro>
                    | <emacro>
                    | <if-directive>
                    | <while-directive>
                    | <repeat-directive>
                    | <goto-directive>
                    | <force-directive>
                    | <return-directive>
                    | <abort-directive>
                    | <continue-directive>
                    | <done-directive>
                    | <nmacro>

<define-macro>    ::= 'define-macro' <macro-name>
<macro-name>      ::= <identifier>
<emacro>          ::= '!emacro'

<if-directive>    ::= '!if' <expression>
<elif-directive>  ::= '!elif' <expression>
<else-directive>  ::= '!else'
<endif-directive> ::= '!endif'

<while-directive> ::= '!while' <expression>
<done-directive>  ::= '!done'
<repeat-directive>::= '!repeat'
<until-directive> ::= '!until' <expression>
<break-directive> ::= '!break'

<return-directive>::= '!return'
<abort-directive> ::= '!abort'
<force-directive> ::= '!force' <command> <arg-list>
<continue-directive>::= '!continue'
<bell-directive>  ::= '!bell'
<ehelp-directive> ::= '!ehelp'
<nmacro>          ::= '!nmacro'

<goto-directive>  ::= '!goto' <label>
<tgoto-directive> ::= '!tgoto' <label> <expression>
<jump-directive>  ::= '!jump' <number>
<tjump-directive> ::= '!tjump' <number>
<label>           ::= '*' <identifier>
```

### Conditionals (Block Structure)

```bnf
<if-block>        ::= <if-directive> <body>
                    | <if-directive> <body> <else-block>

<else-block>      ::= <else-directive> <body>

<elif-block>      ::= <elif-directive> <body>

<body>            ::= <line>*
```

### Loops

```bnf
<while-loop>      ::= <while-directive> <body> <done-directive>
<repeat-loop>     ::= <repeat-directive> <body> <until-directive>
```

### Macro Definition

```bnf
<macro-def>       ::= <define-macro> <body> <emacro>

<body>            ::= <line>*
```

## Examples

### Simple Macro

```emf
define-macro hello
    -1 ml-write "Hello World!"
!emacro
```

BNF derivation:

```
<macro-def>
  <define-macro>  -> "define-macro" "hello"
  <body>
    <line> -> " " "-1" " " "ml-write" " " '"Hello World!"' NEWLINE
  <emacro> -> "!emacro"
```

### Conditional Block

```emf
!if &gre #l0 5
    -1 ml-write "greater"
!elif &less #l0 5
    -1 ml-write "less"
!else
    -1 ml-write "equal"
!endif
```

### While Loop

```emf
set-variable #l0 0
!while &less #l0 10
    set-variable #l0 &add #l0 1
!done
```

### Nested Function Calls

```emf
ml-write &cat "Count: " &sprintf "%02d" #l0
```

This is parsed as:

```
ml-write <expression>
<expression> = &cat <arg1> <arg2>
  <arg1> = '"Count: "'
  <arg2> = &sprintf '"%02d"' #l0
```

## Operator Precedence (Highest to Lowest)

1. Parentheses `()`
2. Function calls `&func args`
3. Unary operators `!`, `-`
4. Multiplicative `*`, `/`, `%`
5. Additive `+`, `-`
6. Bitwise `&`, `|`, `^`
7. Comparison `<`, `>`, `=`, `<=`, `>=`
8. Logical `&`, `|`, `!`
9. Assignment (via `set-variable`)

## Important Notes

1. **No line continuation** - Every line must be complete
2. **Prefix notation** - Functions appear before arguments: `&add 1 2` not `1 &add 2`
3. **All values are strings** - Numbers are converted as needed
4. **Backslash escaping** - `\` must be doubled in strings: `\\`
5. **Comments** - Start with `;` to end of line
6. **Labels** - Start with `*` at beginning of line
7. **Block structure** - Use indentation for readability (not required by parser)

## See Also

- [Writing MicroEmacs Macros](macroWriting(4))
- [Macro Language Overview](macroOverview(4))
- [Macro Language Glossary](MicroEmacs - Macro Language Glossary)
- [Macro Debugging Crash Course](macroDebugging(4))
