# Forth-like Interpreter

This language is heavily inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), a stack-based programming language.

This link is useful for learning [Forth syntax].(https://skilldrick.github.io/easyforth/)

This interpreter is written in C.

### Booleans

`0` is considered `false`.

`1` (or any non-zero integer) is considered `true`. However, `1` is preferred.

### Words

Words must be space-delimited.

Currently supported words:

| Word | Description |
| --- | --- |
| Integer | Pushes the integer onto the stack |
| `,` | Peeks at the first element and prints it |
| `.` | Pops the first element and prints it |
| `emit` | Similar to `.` but prints the ASCII equivalent instead |
| `.s` | Prints the length of the stack |
| `+` | Adds the two first two elements |
| `-` | Subtracts the first element from the second |
| `*` | Multiplies the two first two elements |
| `/` | Floor divides the first element from the second |
| `=` | Pops the first two elements and pushes 0 or 1 depending on not equals or equals respectively |
| `dup` | Duplicates the first element |
| `drop` | Pops the first element |
| `swap` | Swaps the first two elements |
| `over` | Duplicates the second element and pushes it to the top |
| `rot` | Rotates the first 3 elements, i.e. A B C -> B C A |
| `cr` | Prints a new line |

To be implemented:

| Word | Description |
| --- | --- |
|  |  |

Some tests are available in `tests` folder, each `.fth` file is matched with a `.o` file which is the code and the expected result respectively. These tests are run automatically by `test.py`, which must be executed in the root directory.

To do:
- Parse all tokens into an deque (doubly-linked list) to process
- Remainder operation
- Inequality GTE, LTE, GT, LT operations
- `if` statements
- `while` and `for` (`do` loop) loop
- String printing `." --multiple ASCII Integers-- end`
- Add support for commments
- Allow user-defined words

### Example Program
Code:
```
10 2 5 + * .
10 8 - ,
.s drop .s
10 dup + .
1 2 swap . .
1 2 over . . .
33 119 111 87 emit emit emit emit
```

Output
```
70 2 1 0 20 1 2 1 2 1 Wow!
```
