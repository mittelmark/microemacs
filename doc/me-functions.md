# ME Macro Language — Function Cheat Sheet

Quick reference for commonly used functions. All functions abbreviate to 3 letters.
Full docs in `doc/me.smd` (search for function name).

## String Operators

| Function | Args | Description |
|----------|------|-------------|
| `&cat` | str1 str2 | Concatenate: `str1str2` |
| `&lef` | str len | Leftmost `len` chars. Negative `len` = all but last `0-len` chars |
| `&rig` | str index | Rightmost chars from `index`. 0 = full string. Negative = last `0-index` chars |
| `&mid` | str index len | Substring from `index` of length `len`. Negative `index` = from end |
| `&len` | str | String length (number of characters) |
| `&slo` | str | Convert to lowercase |
| `&sup` | str | Convert to uppercase |
| `&trb` | str | Trim whitespace from both sides |
| `&trl` | str | Trim whitespace from left |
| `&trr` | str | Trim whitespace from right |

**Index note:** String indices are **1-based**. `&lef "abc" 2` → `"ab"`.

## List Functions

Lists are pipe-delimited: `"|item1|item2|item3|"`. The first character is the delimiter.

| Function | Args | Description |
|----------|------|-------------|
| `&lle` | list | Number of items in list |
| `&lge` | list index | Get item at `index` (**1-based**, 0 = empty) |
| `&lin` | list index value | Insert `value` at `index`. 0 = beginning, -1 = end, -2/-3 = alphabetical |
| `&lde` | list index | Delete item at `index` |
| `&lfi` | list value | Find value, returns index (0 if not found) |
| `&lse` | list index value | Set (replace) item at `index` |

**Important:** `&lget` index is **1-based** — index 0 returns empty string!

```me
set-variable %list "|"              ; empty list
set-variable %list &lin %list -1 "A"   ; "|A|"
set-variable %list &lin %list -1 "B"   ; "|A|B|"
&lle %list                           ; → 2
&lge %list 1                         ; → "A"
&lge %list 2                         ; → "B"
&lfi %list "B"                       ; → 2
```

## Arithmetic

| Function | Args | Description |
|----------|------|-------------|
| `&add` | n1 n2 | n1 + n2 |
| `&sub` | n1 n2 | n1 - n2 |
| `&mul` | n1 n2 | n1 * n2 |
| `&div` | n1 n2 | Integer division: n1 / n2 |
| `&mod` | n1 n2 | Modulus (remainder): n1 % n2 |
| `&neg` | n | Negate: -n |
| `&abs` | n | Absolute value |
| `&inc` | var n | Pre-increment var by n, returns new value |
| `&dec` | var n | Pre-decrement var by n, returns new value |
| `&pinc` | var n | Post-increment, returns old value |
| `&pdec` | var n | Post-decrement, returns old value |

Expression evaluation is **prefix** (no brackets): `(2*3)+4` = `&add &mul 2 3 4`.

## Comparison Operators

### Numeric

| Function | Abbrev | Description |
|----------|--------|-------------|
| `&equal` | `&equ` | num1 == num2 |
| `&great` | `&gre` | num1 > num2 |
| `&less` | `&les` | num1 < num2 |

**No `&ges` or `&lss`!** For >= use `!if &not &les`, for <= use `!if &not &gre`.

### String

| Function | Abbrev | Description |
|----------|--------|-------------|
| `&sequal` | `&seq` | str1 == str2 (case sensitive) |
| `&isequal` | `&iseq` | str1 == str2 (case insensitive) |
| `&sless` | `&sle` | str1 < str2 (alphabetical) |
| `&sgreat` | `&sgr` | str1 > str2 (alphabetical) |
| `&xsequal` | `&xse` | str1 matches regex (case sensitive) |
| `&xisequal` | `&xis` | str1 matches regex (case insensitive) |

### Logical

| Function | Description |
|----------|-------------|
| `&and` | Both true |
| `&or` | Either true |
| `&not` | Negate |

**No short-circuit evaluation** — all arguments are always evaluated.

## Bitwise Operators

| Function | Description |
|----------|-------------|
| `&band` | Bitwise AND: `num1 & num2` |
| `&bor` | Bitwise OR: `num1 \| num2` |
| `&bxor` | Bitwise XOR: `num1 ^ num2` |
| `&bnot` | Bitwise NOT: `~num` |

## Search Functions

| Function | Args | Description |
|----------|------|-------------|
| `&sin` | str1 str2 | Position of first occurrence of str1 in str2 (1-based, 0 = not found) |
| `&isi` | str1 str2 | Same as `&sin` but case insensitive |
| `&ris` | str1 str2 | Position of **last** occurrence |
| `&ris` | str1 str2 | Same but case insensitive |

**Index note:** Return value is **1-based** (position + 1). `0` means not found.

## Conditional Expression

```
&cond log expr1 expr2
```

If `log` is non-zero, return `expr1`, else return `expr2`. Usable inside other expressions:

```me
set-variable %max &cond &gre %a %b %a %b   ; max(a,b)
```

## Variable Naming

| Prefix | Scope | Example |
|--------|-------|---------|
| `$` | Global | `$system`, `$buffer-fname` |
| `#l0`-`#l9` | Local (per macro) | `#l0`, `#l1` |
| `%` | Temporary (current exec) | `%temp` |
| `.ns.name` | Buffer-local | `.mdview.history-pos` |

Buffer-local variables MUST use `.namespace.varname` format (dot after namespace).

## Control Flow

```me
!if condition
    ...
!else
    ...
!endif

!while condition
    ...
!done

!repeat
    ...
!until condition

!force              ; Ignore errors in next command
!return             ; Return from current macro
!abort              ; Abort current operation
!goto label         ; Jump to label
```
