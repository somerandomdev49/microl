## microl
> A pure, minimalistic programming language.

written in C.

### Example:
```js
let a = 12 * (4 + 3)
a
```

this will print `84`

### Usage:
```sh

$ microl run example.microl
# This will run example.microl

```

### Source

currently, only a simple interpreter that supports only numbers (doubles) is implemented.

* `token.h` - tokens and a `token_list_t` which is a linked list of tokens.
* `lexer.h` - defines `lex` that will generate a token list from a FILE.
* `ast.h` - defines all of the nodes.
* `parser.h` - all of the parser functions.
* `intr.h` - an interpreter (temporary)
* `main.c` - the main file. (cli + running)

### TODO:
* parse operators other than `+` and `*` (simple to fix, but im lazy).
* add if and loops
* change from `double` to some kind of `obj_t` for the interpreter.
* add strings.
