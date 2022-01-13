# Forth-like Interpreter

This language is heavily inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), a stack-based programming language.

This link is useful for learning [Forth syntax](https://skilldrick.github.io/easyforth/).

This interpreter is written in C. Testing framework is written in Python.

## Booleans

`0` is considered `false`.

`1` (or any non-zero integer) is considered `true`. However, `1` is preferred.

## Words

Words must be space-delimited.

### Integer

Pushes the integer onto the stack. Integers are 32-bit integers.

`1 2 3 100` pushes `1`,`2`,`3`,`1000` onto the stack in order.

### Printing to Standard Output

The following words will print into standard output. Will have an error if the operation causes stack underflow.

| Word | Description |
| --- | --- |
| `,` | peeks at the top of the stack, without removing it, printing the literal integer. |
| `.` | pops from the top of the stack, removing it, printing the integer literal. |
| `emit` | similar to `.` but prints the ASCII equivalent instead. |
| `.s` | prints the size of the current stack. |
| `cr` | prints a new line |

```
65 1 2 3
,    -- prints 3
.s   -- prints 4
.    -- prints 3
.    -- prints 2
.    -- prints 1
emit -- prints `A`
.s   -- prints 0
```

### Mathematical Operations

Used as in Reverse Polish Notation, e.g. `1 2 +` (RPN) === `1 + 2` (usual infix notation).

The follow operations pop 2 elements off the stack and push the result back on it.

| Word | Description |
| --- | --- |
| `+` | adds the two elements |
| `-` | subtracts the two elements |
| `*` | multiplies the two elements |
| `/` | floor divides the two elements |
| `%` | remainder of the two elements |

```
1 2 + .  -- prints 3
9 3 - .  -- prints 6
5 6 * .  -- prints 30
50 7 / . -- prints 7
50 7 % . -- prints 1
```

### Comparison Operations

The following operations pop 2 elements off the stack and push 0 (if false) and 1 (if true) back on it.

| Word | Description |
| --- | --- |
| `=` | checks for equality |
| `>=` | greater than or equals |
| `<=` | lesser than or equals |
| `>` | greater than |
| `<` | lesser than |

```
1 1 = .    -- prints 1 (true)
0 1 = .    -- prints 0 (false)
19 19 >= . -- prints 1 (true)
20 19 >= . -- prints 1 (true)
19 5 <= .  -- prints 0 (false)
2 1 > .    -- prints 1 (true)
5 5 < .    -- prints 0 (false)
```

### Stack Manipulation

| Word | Description |
| --- | --- |
| `dup` | duplicates the first element |
| `drop` | pops the first element |
| `swap` | swaps the first two elements |
| `over` | duplicates the second element and pushes it to the top |
| `rot` | rotates the first 3 elements, `1 2 3 rot . . .` -- prints `1 3 2` |

## Tests

Some tests are available in `tests` folder, each `.fth` file is matched with a `.o` file which is the code and the expected result respectively. These tests are run automatically by `test.py`, which must be executed in the root directory.

## TODO
- `if` statements
- `while` and `for` (`do` loop) loop
- String printing `." --multiple ASCII Integers-- end`
- Add support for commments
- Better testing framework (especially for verifying errors)
- Allow user-defined words
