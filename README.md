# Forth-like Interpreter

This language is heavily inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), a stack-based programming language.

Useful for learning [Forth syntax](https://skilldrick.github.io/easyforth/)

This interpreter is written in C.

Words must be space-delimited.

Currently supported words:

| Word | Description |
| --- | --- |
| Integer | Pushes the integer onto the stack |
| `,` | Peeks at the first element and prints it |
| `.` | Pops the first element and prints it |
| `.s` | Prints the length of the stack |
| `+` | Adds the two first two elements |
| `-` | Subtracts the first element from the second |
| `*` | Multiplies the two first two elements |
| `/` | Divides the first element from the second |
| `dup` | Duplicates the first element |
| `drop` | Pops the first element |
| `swap` | Swaps the first two elements |
| `over` | Duplicates the second element and pushes it to the top |

To be implemented:

| Word | Description |
| --- | --- |
| `rot` | Rotates the first 3 elements, i.e. A B C -> B C A |
| `emit` | Similar to `.` but prints the ASCII equivalent instead |
|  |  |
| Defining Words | Defining user-declared words |

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
