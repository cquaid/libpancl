h1. Language Features
Just a short overview while the formal specification is being written.

h2. Tokens

h3. Newline
```
Newline = "\r\n"
        | "\r"
        | "\n"
        ;
```

h3. Comment
```
Comment = '#' .* Newline
        ;
```
Comments start with `#` and continue until the end of the line.  A comment
may not end with a backslash `\\` as comments are single line and thus cannot
continue to the next line. Comments may appear almost anywhere.

h3. Raw Identifier
```
RawIdent = [a-zA-Z0-9_:+-]+
```
Raw Identifiers are bare-words that may be used in limited context.  While Raw
Identifiers may look like an integer, floating point number, or boolean,
they may be evaluated differently depending on the context.

The rule-of-thumb is: if the the context allows an integer, floating point, or
boolean, then those types are used instead of raw identifer.

h3. String
```
String = '"' .* '"'
       | "'" .* "'"
       | String '"' .* '"'
       | String "'" .* "'"
       ;
```
A string is a series of adjacent quoted values. Values in double-quotes may
evaluate escape sequences:
| Escape Sequence | Meaning |
| --- | --- |
| `\\a` | BEL (0x07) |
| `\\b` | BS  (0x08) |
| `\\f` | Form Feed (0x0C) |
| `\\n` | Line Feed (0x0A) |
| `\\r` | Carriage Return (0x0D) |
| `\\t` | Horizontal Tab (0x09) |
| `\\v` | Vertical Tab (0x0B) |
| `\\`  | Backslash (0x5C) |
| `\\'` | Single Quote (0x27) |
| `\\"` | Double Quote (0x22) |
| `\\ooo` | Octal value, 1-3 digits (o = octal digit) |
| `\\xhh | Single-byte hex value (h = hex digit) |
| `\\uhhhh | two-byte unicode character (h = hex digit) |
| `\\Uhhhhhhhh | four-byte unicode character (h = hex digit) |

Newlines may also be escaped.  If so, the newline is consumed and the string
parsing continues as if it never existed.

If a literal newline is found in a string, it is converted into a single Line
Feed (LF) character.

A single-quoted string does not evaluate escape sequences.

Adjacend strings (``"foo" 'bar'``) are combined into one token ("foobar").

h3. Integers
```
BinaryInt = [+-]?0[bB][01]+
       ;
DecInt = [+-]?[1-9][0-9]*
       ;
HexInt = [+-]?0[xX][a-fA-F0-9]+
       ;
OctalInt = [+-]?0[oO][0-7]+
         ;

Integer = BinaryInt
        | DecInt
        | HexInt
        | OctalInt
        | [+-]?0
        ;
```
Note that `0` may not prefix a decimal integer. Only `+0`, `-0`, and `0` are
valid.  However, hex, octal, and binary integers may contain multiple leading
zeros after their specific prefixes (e.g. `0x00000a`, `0b00`, `0o067`).

h3. Floating Point Values
```
SignAndNumber = [+-]?[0-9]+
            ;
Exponent = [eE] SignAndNumber
         ;

Float = SignAndNumber '.' Exponent?
      | SignAndNumber '.' [0-9]+ Exponent?
      | [+-]? '.' [0-9]+ Exponent?
      | [+-]?NaN
      | [+-]?Inf
      ;
```

h3. Boolean Values
```
Bool = 'true'
     | 'false'
     ;
```

h3. Whitespace
```
Whitespace = [ \t]
```

h2. High-Level Constructs

h3. Array
```
ArrayMemberList = RVALUE
                | ArrayMemberList ',' RVALUE
                ;

Array = '[' ']'
      | '[' ArrayMemberList ']'
      | '[' ArrayMemberList ',' ']'
      ;
```
Arrays may only contain members of a single type and may include an optional
trailing comma.

h3. Tuple
```
TupleMemberList = RVALUE
                | TupleMemberList ',' RVALUE
                ;

Tuple = '(' ')'
      | '(' TupleMemberList ')'
      | '(' TupleMemberList ',' ')'
      ;
```
Tuples may contain members any type (i.e. each member may be a different type
if desired), and may include an optional trailing comma.

h3. Custom Type
```
CustomType = RawIdent Tuple
           ;
```
Custom Types are a way to specify a unique type that isn't part of PanCL.
They are simply an unquoted name followed by a Tuple.  The name must be a
Raw Identifier that does not look like an integer, float, or bool.

Custom Type names that begin with `::` are reserved for use by the standard.

h3. Inline Table
```
InlineTableList = Assignment
                | InlineTableList ',' Assignment
                ;

InlineTable = '{' '}'
            | '{' InlineTableList '}'
            | '{' InlineTableList ',' '}'
            ;
```
An inlline table is a collection of name-value pairs. These come in the form
of assignment expressions and may include an optional trailing comma.  The
names in the inline table are not required to be unique.

h3. RVALUE
```
RVALUE = Array
       | Bool
       | CustomType
       | Float
       | Integer
       | InlineTable
       | String
       | Tuple
       ;
```
An RVALUE is anything legal to the right of an equal sign in an assignment.

h3. Assignment
```
Assignment = String '=' RVALUE
           | RawIdent '=' RVALUE
           ;
```
Assignment statements are one of the two allowable constructs available at
the top-level of a PanCL file.

h3. Table Header
```
TableHeader = '[' String ']'
            | '[' RawIdent ']'
            ;
```
A Table Header may only appear at the top-level of a PanCL file.  A Table
thus is a collection of name-value pairs.  The names in a Table do not have
to be unique.

Table Headers must be on a line by themselves (save for comments)

h2. Parsing Entry Point
```
TopLevelConstruct = TableHeader
                  | Assignment
                  ;

EntryPoint = TopLevelConstruct
           | EntryPoint TopLevelConstruct
           ;
```
The entry point for parsing a PanCL file may only contain Table Headers,
Assignments, comments and whitespace.

h2. Other Information
1. Lines ending with a backslash (`\\`) followed immediately by a newline are
continued onto the next line.  Otherwise, parsing ends at the newline unless
parsing an Array, Tuple, Inline Table, or String.
1. Optional Integer Types
Since the default integer type is a signed 32-bit value, the language defines
optional alternate-width integer designations:
| Custom Type Name |Description |
| --- | --- |
| ::Integer | Signed 32-bit |
| ::Int8 | Signed 8-bit |
| ::Uint8 | Unsigned 8-bit |
| ::Int16 | Signed 16-bit |
| ::Uint16 | Unsigned 16-bit |
| ::Int32 | Signed 32-bit |
| ::Uint32 | Unsigned 32-bit |
| ::Int64 | Signed 64-bit |
| ::Uint64 | Unsigned 64-bit |
All Optional integer types accept a tuple with one or two parameters:
    1. A *string* value representing the integer
    1. An optional *integer* value representing the base.  If `0` is given,
       the number may be decimal, octal, binary, or hexadecimal (with
       appropriate prefixes).

