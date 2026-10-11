# Pratt parser in C

A minimal Pratt parser (top-down operator precedence) written for my blog post
[How to Build a Pratt Parser](https://pablosabater.dev/posts/how-to-build-a-pratt-parser/). It reads one
expression per line from standard input and prints its tree as an
S-expression:

```
$ echo 'a + b * c - d' | ./pratt
(- (+ a (* b c)) d)
```

## Build

```sh
make
./pratt
```

Type an expression and press Enter.

## What it parses

| Syntax           | Example             | Output                 |
|------------------|---------------------|------------------------|
| `+` `-` `*`      | `a + b * c`         | `(+ a (* b c))`        |
| `^` (right assoc)| `a ^ b ^ c`         | `(^ a (^ b c))`        |
| prefix `!`       | `!a * b`            | `(* (! a) b)`          |
| parentheses      | `(a + b) * c`       | `(* (+ a b) c)`        |
| member access    | `a.b.c`             | `(. (. a b) c)`        |
| indexing         | `a[b]`              | `([ a b)`              |
| calls            | `f(a, b)`           | `(( f (, a b))`        |
| `if` / `else`    | `if x y else z`     | `(if x y z)`           |

Operands are words made of letters and digits (`x`, `42`, `foo`). Spaces are
optional between operators.

## Limitations

This is teaching code, kept short on purpose:

- No error handling: the input is assumed to be well-formed.
- Nodes are never freed.
- No unary `-` and no empty calls (`f()`).
- There is nothing between an `if` condition and its branch, so `if c (x)`
  parses the condition as the call `c(x)`.


Feel free to contribute. <3
