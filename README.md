# StackC

StackC is a stack-oriented programming language that is inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), another stack-oriented programming language. This language is built entirely for fun!

The interpreter and testing framework is written in C.

If anyone stumbles upon this and has any feedback/bug reports, feel free to open an issue regarding it. I would appreciate it, thank you!

## Quick Setup

Minimally, you only need to download `stackc.c` and compile it with a C compiler. Then, you can run the executable with your program name as the argument.

The program name must have an extension of ".stc".

```shell
gcc -o stackc stackc.c
./stackc <your_program>.stc
```

## Documentation

Included below are brief explanations and examples (and equivalent programs in python). There are more examples in `tests` folder.

Feel free to check out the source code for more details.

## Words

Words must be separated with either a ` ` (space character) or `\n` (new line). You can have multiple spaces or new line characters (even mixed around) successively, they are treated as a single separator.

```stackc
1    2 + .
```

```stackc
1
2 +

.
```

The above two programs are equivalent (aside from readability).

FYI: Words are hashed by their first character, so that searches are a bit more performant.

## Comments

Comments are declared by `//`, every character after `//` is ignored by the interpreter.

```stackc
// This is a comment.
// Anything after `//` is ignored.
// . . . . .
// Does not cause stack underflow because it is not ran!
65 emit // prints an `A` character
```

```python
# This is a comment.
# Anything after `#` is ignored.
# print("NOT PRINTED!")
# Does not cause stack underflow because it is not ran!
print(chr(65)) # prints an `A` character
```

## Types

StackC uses a simple inferred typing system to allow operations to understand what type of object they are computing should they care about it.

The types currently supported are integers, characters, strings.

The type code for each object is pushed onto the stack after its value.

Type casting words are available in the standard library.

### Integers

Type Code: 0

Pushes the integer onto the stack. Integers are 32-bit integers. Negative integers are supported too.

`1 2 3 100 -2` pushes `1`,`2`,`3`,`1000`,`-2` onto the stack in order.

### Booleans

Type Code: 0 (pseudo-type based on integer)

Booleans are a pseudo-type. They are typed as integers but simply treated differently.

`0` is considered `false`.

`1` (or any non-zero integer) is considered `true`. However, `1` is preferred.

The standard library defines `true` and `false` as constants mapped to `1` and `0` respectively for convenience.

### Characters

Type Code: 1

`'a' 'A' 'B'` pushes `'a'`,`'A'`,`'B'` onto the stack in order.

#### Mathematical Operations

Used as in Reverse Polish Notation, e.g. `1 2 +` (RPN) === `1 + 2` (usual infix notation).

The follow operations pop 2 elements off the stack and push the result back on it.

| Word | Description |
| --- | --- |
| `+` | adds the two elements |
| `-` | subtracts the two elements |
| `*` | multiplies the two elements |
| `/` | floor divides the two elements |
| `%` | remainder of the two elements |

```stackc
9 3 - .  // prints 6
5 6 * .  // prints 30
50 7 / . // prints 7
50 7 % . // prints 1
```

```python
print(9 - 3)     # prints 6
print(5 * 6)     # prints 30
print(50 // 7)   # prints 7
print(50 % 7)    # prints 1
```

##### Add (+)

Adding 2 integers will result in an integer. If either of the 2 objects are characters, the result will be a character. Cannot add 2 characters.

```stackc
 1   2  +  //  3
'a'  1  +  // 'b'
 2  'c' +  // 'a'
'a' 'b' +  // Error: char char + not supported.
```

#### Comparison Operations

The following operations pop 2 elements off the stack and push 0 (if false) and 1 (if true) back on it.

| Word | Description |
| --- | --- |
| `=` | checks for equality |
| `!=` | checks for not equals |
| `>=` | greater than or equals |
| `<=` | lesser than or equals |
| `>` | greater than |
| `<` | lesser than |

```stackc
1 1 = .    // prints 1 (true)
0 1 = .    // prints 0 (false)
19 19 >= . // prints 1 (true)
20 19 >= . // prints 1 (true)
19 5 <= .  // prints 0 (false)
2 1 > .    // prints 1 (true)
5 5 < .    // prints 0 (false)
```

```python
print(1 == 1)    # prints True
print(0 == 1)    # prints False
print(19 >= 19)  # prints True
print(20 >= 19)  # prints True
print(19 <= 5)   # prints False
print(2 > 1)     # prints True
print(5 < 5)     # prints False
```

#### Printing to Standard Output

The following words will print into standard output. Will have an error if the operation causes stack underflow.

| Word | Description |
| --- | --- |
| `.` | pops from the top of the stack, removing it, printing the integer. |
| `emit` | similar to `.` but prints the ASCII equivalent instead. |
| `.s` | prints the size of the current stack. Intended to be used for debugging interpreter. |

```stackc
65 1 2 3
.s   // prints 4
.    // prints 3
.    // prints 2
.    // prints 1
emit // prints `A`
.s   // prints 0
```

```python
stack = [65, 1, 2, 3]
print(stack[-1])           # prints 3
print(len(stack))          # prints 4
print(stack.pop())         # prints 3
print(stack.pop())         # prints 2
print(stack.pop())         # prints 1
print(chr(stack.pop()))    # prints `A`
print(len(stack))          # prints 0
```

### Strings

Type code: 2

Strings are saved by characters in the stack.

`"ABC"` pushes 0, 67, 66, 65, 3 onto the stack.

The string size will be at the top of the stack, followed by `n` characters in ascii and then the terminating NULL character.

`print` pops a string off the stack and prints it to standard output.

Multi-line strings are not directly supported. However you can use `\n` for a new line.

```stackc
"Hello World\n" print

"This is\t tabbed!" print

"C" "B" "A" print print print
```

will print the following to the standard output

```txt
Hello World
This is    tabbed!ABC
```

| Escape Characters | Character |
| --- | --- |
| `\"` | Double quotes |
| `\'` | Single quotes |
| `\\` | Backslash |
| `\n` | New line |
| `\r` | Carriage return |
| `\t` | New tab |


## Stack Manipulation

| Word | Description |
| --- | --- |
| `drop` | pops the first element |
| `dup` | duplicates the first element |
| `over` | duplicates the second element and pushes it to the top |
| `rot` | rotates the first 3 elements, `1 2 3 rot` -> `2 3 1` |
| `swap` | swaps the first two elements |

## Control Flow

### If Statement

The words between `if` and `then` are always evaluated. When `then` is encountered, the first element is popped off the stack and evaluated as a boolean. If true, the block between `then` and the next `elseif` or `end` is evaluated. After which, the evaluator will jump to the `end` word. If false, the evaluator will jump to the next `elseif` block to evaluate it.

If you want to simulate a regular `else`, as in other languages, just use a `elseif 1 then` for it to always evaluate to true.

Nested if/else are also supported!

```stackc
if 0 then          // evaluates to false
  12 .
elseif 1 then      // evaluates to true
  if 2 1 - then    // evaluates to 1 which is true
    23 .           // 23 is printed
  elseif 1 then    // skipped to jump ahead to the `end` block
    34 .
  end
elseif 1 then
  45 .             // never evaluated as it is equivalent to placing 2 `else` after one another.
end
```

```python
if False:
  print(12)
else:
  if bool(2 - 1):
    print(23)          # 23 is printed
  else:
    print(34)
else:                  # not even valid python to have 2 else blocks
  print(45)            # never evaluated
```

### While Loop

The words between `while` and `then` are evaluated at the start of each loop. After which, an integer is popped off the stack and evaluated as a boolean. If this is false, the loop ends. If this is true, the words between `then` and `end` are evaluated. And this will repeat indefinitely (remember to update your control variable).

Nested loops are supported, go crazy (but maybe not *too* crazy)!

The following program is FizzBuzz from 1 to 100.

```stackc
1
while dup 100 <= then
  if dup 15 % 0 = then
    "FizzBuzz\n" print
  elseif dup 3 % 0 = then
    "Fizz\n" print
  elseif dup 5 % 0 = then
    "Buzz\n" print
  elseif 1 then
    dup . "\n" print
  end
  1 +
end
drop
```

```python
x = 1
while x <= 100:
  if x % 15 == 0:
    print("FizzBuzz")
  elif x % 3 == 0:
    print("Fizz")
  elif x % 5 == 0:
    print("Buzz")
  else:
    print(x)
  x += 1
```

## Defining Words

It is possible to define custom words, which is useful for repeated operations. It is also possible to define "constants" this way as well, however, at the moment, the interpreter does not optimise for this.

Rules:
1. Words cannot start with a number.
2. Words cannot contain `'` and `"`.

Do note that recursion is not supported. Weird things may happen (probably segfault). StackC *may* support recursion in the future, depending on how hard it is to implement.

`def <wordname> <word body> end`

After the definition of the custom word, every other occurrence of a word is effectively replaced by the word body.

Ideally, one adds in a "function signature" as a comment to denote how many elements the word will pop and push.

The following program computes the nth fibonaci number (this is what I came up with but there might be a better way). It is kinda tricky to have to juggle with 3 values in the stack.

```stackc
// duplicates top 2 elements
def dup2 // A, B -> A, B, A, B
  over over
end

// nth fibonacci number
def fib // int -> int
  1 -
  0 1 rot
  while dup 0 > then
    rot rot
    dup2 + rot drop
    rot 1 -
  end
  drop drop
end

29 fib .                // prints 317811
```

is equivalent to

```python
def fib(n):
  n -= 1
  a = 0
  b = 1
  while n > 0:
    a, b = b, a + b
    n -= 1
  print(a)

fib(29)                 # prints 317811
```

## Standard Library

Documentation for standard library available [here](stdlib.md).

## Tests

Some tests are available in `tests` folder, each `.stc` file is matched with a `.o` file which is the code and the expected result respectively. These tests are run automatically by `test`.

### Quick Usage of Tests

If you downloaded the `Makefile`, just run `make`.

Else you can run the tester manually, `gcc -o test test.c && ./test -d tests`

### Flags

Flags must be declared before the directory/files.

| Flag | Description |
| --- | --- |
| `d` | Runs all tests in specified directory after flags. Only one directory is allowed. |
| `u` | Creates (if it does not exist) and updates all `.o` files with the current `.stc` stdout. |
| `v` | Verbose output. Logs standard output of the evaluation and some debug information. |

Do not include `.stc` when denoting the program.


```shell
./test -d tests # runs all tests

./test -dv tests
./test -d -v tests # runs all tests with verbose output

./test -u tests/if # runs `tests/if.stc` and updates `tests/if.o` with the current output

./test tests/if tests/while # runs `tests/if.stc` and compares with `tests/if.o` and same with `while`
```

### Makefile Arguments

| Command | Description |
| --- | --- |
| no arguments | Runs all tests in `tests` directory. |
| `update` | Updates all expected files with current output. |
| `verbose` | Runs all tests in `tests` directory with verbose output. |
| `clean` | Cleans up `stackc` and `test` executables. |

## TODO

- simple type system (int, char, string, everything on the stack has a type)
- every object is type-value-...-values
- 'A' to register a character
- write tests/push.stc test for char pushing

- break statement to jump to the end
- import/include files so we can actually import and use the stdlib.stc!!!
- get nth element of the stack
- `stackc.c` debug flag to print stack after each operation
- Bitwise operations
- Rule 110 program
- Game of life
- brainfk interpreter
- read input?
- floats?
- recursion?

- have access to a second stack?
- string manipulation words

- array

- meta-evaluator (stackc being able to evaluate stackc)
- compile stackc programs into assembly -> executables
- re-write StackC compiler in StackC

stdlib:
- print in stdlib instead (its there but I'll remove it from primitive after include)
- reverseN
