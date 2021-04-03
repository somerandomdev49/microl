## microl
> A pure, minimalistic programming language.

written in C.

### Examples:
```lua
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

```lua
let fib = func(x) do
    if x < 2 then x
    else fib(x - 1) + fib(x - 2)
end

let output = fib(6)
```

this does the same thing:
```lua
let output = (func(x) if x < 2 then x else @(x - 1) + @(x - 2))(6)
```


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
* [x] ~~parse operators other than `+` and `*` (simple to fix, but im lazy).~~
* [x] ~~add if and loops~~
* [x] ~~change from `double` to some kind of `obj_t` for the interpreter.~~
* [x] ~~add functions.~~
* [ ] unify object allocation functions.
* [ ] make object values separate. a simple number takes up a lot of memory!
* [ ] add strings.
* [ ] add parser support for nil.
* [ ] change `get != ... then error` to `peek != ... then error else del`
* [ ] add stdlib - *in progress*
