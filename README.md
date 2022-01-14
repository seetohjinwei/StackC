# Forth-like Interpreter

This language is heavily inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), a stack-based programming language.

This link is useful for learning [Forth syntax](https://skilldrick.github.io/easyforth/). However, do note that I do not follow all of the syntax in standard Forth.

This interpreter is written in C. Testing framework is written in Python.

## Quick Setup

Minimally you only need to download `forth.c` and compile it with a C compiler. Then, you can run the executable with your program name as the argument.

```
gcc forth.c -o forth
./forth your_program
```

## Words

Words must be separated with either a ` ` (space character) or `\n` (new line). You can have multiple spaces or new line characters (even mixed around) successively, they are treated as a single separator.

```
1    2 + .
```

```
1
2 +

.
```

The above two programs are equivalent (aside from readability).

### Comments

Comments are declared by `--`, every character after `--` is ignored by the interpreter.

```
-- This is a comment.
-- Anything after `--` is ignored.
-- . . . . .
-- Does not cause stack underflow because it is not ran!
65 emit -- prints an `A` character
```

### Integer

Pushes the integer onto the stack. Integers are 32-bit integers.

`1 2 3 100` pushes `1`,`2`,`3`,`1000` onto the stack in order.

## Booleans

`0` is considered `false`.

`1` (or any non-zero integer) is considered `true`. However, `1` is preferred.

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

### Strings

String functionality is *extremely* limited and only intended to be used for printing short messages.

Any characters between `."` and `end` is evaluated as characters and printed to standard output. There is no way to save strings to the stack.

Do note that there are required space characters after and before `."` and `end` respectively. They will not be printed.

No many how many spaces or newlines there are between each word in the string, it is ignored and replaced with a single space character, *and* there is a extra space and newline character after every string. This is because I parse the file completely before interpreting it and this results in a loss of information about the original spacing between the words. But doing it this way allows me to check for potential errors before interpreting the file.

Also, if you are trying to print `end`, I am afraid that there is no way to escape the `end` keyword. You can probably just print `end.` or something I don't know!

Also also, strings are limited to 1000 characters.

```
." This is a single line string! end

." This is a

"multi-line"

string

end
```

will print the following to the standard output

```
This is a single line string! 
This is a "multi-line" string 
```

### Control Flow

#### `if` and `elseif` words

`if` will pop the first element off the stack and treat it as a boolean. The following example will hopefully explain it better than I can.

Nested if/else are also supported!

```
-- in this language
1 0      -- pushing `1` and `0` onto the stack       (stack: [1, 0])
if       -- 0 is popped off the stack and is false   (stack: [1])
  12 .   -- skipped
elseif   -- 1 is popped off the stack and is true    (stack: [])
  2 1 -  --                                          (stack: [1])
  if     -- 1 is popped and is true                  (stack: [])
    23 . -- 23 will be printed
  else
    34 .
  end
else     -- elseif will skip to end
  45 .   -- not evaluated
end
```

```python
# equivalent program in python
stack = []               # to simulate stack
stack.append(1)
stack.append(0)

if stack.pop():
  print(12)
elif stack.pop():
  stack.append(2 - 1)
  if stack.pop():
    print(23)             # 23 is printed
  else:
    print(34)
else:
  print(45)
```


### Stack Manipulation

| Word | Description |
| --- | --- |
| `dup` | duplicates the first element |
| `drop` | pops the first element |
| `swap` | swaps the first two elements |
| `over` | duplicates the second element and pushes it to the top |
| `rot` | rotates the first 3 elements, `1 2 3 rot . . .` -- prints `1 3 2` |

### Memory

There is a integer array (of size 100) for the program to utilise. All values in the array is initialised to be 0 when the program is ran. It is implemented with a C array so there is random memory access.

This is created for conveniences like having global variables or when creating loops or whatever else you find a use for!

If you think this violates the idea of a stack-based language too much, then feel free to just not use these. If these keywords are not found in the program, the array is never initialised in the interpreter.

| Word | Description |
| --- | --- |
| `mem A` | pops the first element off the stack and saves it to index `A` |
| `memr A` | pushes the value of index `A` onto the stack |

## Tests

Some tests are available in `tests` folder, each `.fth` file is matched with a `.o` file which is the code and the expected result respectively. These tests are run automatically by `test.py`, which must be executed in the root directory.

## TODO

- Power (exponent)
- Bitwise operations
- `while` and `for` (`do` loop) loop
- String printing `." --multiple ASCII Integers-- end`
- Better testing framework (especially for verifying errors)
- If error is detected, test.py should print to stdout as well (if not supposed to be).
- In `test.py` auto-update `.o`s, with some argument like `--force-update` or something
- Allow user-defined words
