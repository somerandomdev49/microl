## microl
> A pure, minimalistic programming language.

written in C.

### Examples:
```coffeescript
let output = 0
let i = 1
while i < 10 do
    let j = 0

    while j < 10 do
        output = output + j * i
        if j == 5 then break
        j = j + 1
    end
    i = i + 1
end
```

functions:

```coffeescript
let write = function(v)
    __write(tostring(v))

let sum = function(x)
    if x < 2 then x
    else x + sum(x - 1)

write(tostring(sum(3)))

```

this does the same thing:
```coffeescript
let write = function(v)
    __write(tostring(v))

write(tostring((function(x) if x < 2 then x else x + @(x - 1))(3)))
```

modules:
```coffeescript
import io
io.write(1)
```
io.microl:
```coffeescript
let export io = function(val) __write(tostring(val))
```
(beware that currently all modules should be located in `./stdlib/`)

### Usage:
```sh

$ microl run example.microl
# This will run example.microl

```
The value printed is the value of the `output` variable.

### Source

* `token.h` - tokens and a `token_list_t` which is a linked list of tokens.
* `lexer.h` - defines `lex` that will generate a token list from a FILE.
* `ast.h` - defines all of the nodes.
* `parser.h` - all of the parser functions.
* `ctx.h` - defines obj_t and ctx_t
* `intr.h` - an interpreter (temporary)
* `main.c` - the main file. (cli + running)

### TODO:
* [ ] make modules a separate object that can hold a whole context!.
* [ ] add parser support for strings.
* [ ] add parser support for nil.
* [ ] unify object allocation functions.
* [ ] change `get != ... then error` to `peek != ... then error else del`
* [ ] make this a compiler to custom bytecode
* [ ] optimise
* [x] ~~parse operators other than `+` and `*` (simple to fix, but im lazy).~~
* [x] ~~add if and loops~~
* [x] ~~change from `double` to some kind of `obj_t` for the interpreter.~~
* [x] ~~add functions.~~
* [x] add stdlib - *in progress*
* [x] ~~add modules!~~
* [x] ~~make object values separate. a simple number takes up a lot of memory!~~

### Long-Term goals:
* Compile to WASM
* Compile to x86
* LLVM for C?
* Other VMs.
