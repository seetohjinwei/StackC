# StackC

StackC is a stack-oriented programming language that is heavily inspired by [Forth](https://en.wikipedia.org/wiki/Forth_\(programming_language\)), another stack-oriented programming language.

The interpreter is written in C. Testing framework is written in Python.

## Quick Setup

Minimally you only need to download `stackc.c` and compile it with a C compiler. Then, you can run the executable with your program name as the argument. Or alternatively, you can just run a short script with a `-s` flag in front as shown. This short script would be identical to an actutal `.stc` file except it does not accept newline as spacing.

```
gcc stackc.c -o stackc
./stackc your_program
./stackc -s "if 0 then 45 emit elseif 1 then 43 emit end" -- prints `+` to stdout
```

## Documentation

Included below are brief explanations and examples (and equivalent programs in python). There are more examples in `tests` folder.

Feel free to check out the source code for more details.

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

```python
# This is a comment.
# Anything after `#` is ignored.
# print("NOT PRINTED!")
# Does not cause stack underflow because it is not ran!
print(chr(65)) # prints an `A` character
```

### Integer

Pushes the integer onto the stack. Integers are 32-bit integers. Negative integers are supported too.

`1 2 3 100 -2` pushes `1`,`2`,`3`,`1000`,`-2` onto the stack in order.

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

```python
print(1 + 2)     # prints 3
print(9 - 3)     # prints 6
print(5 * 6)     # prints 30
print(50 // 7)   # prints 7
print(50 % 7)    # prints 1
```

### Comparison Operations

The following operations pop 2 elements off the stack and push 0 (if false) and 1 (if true) back on it.

| Word | Description |
| --- | --- |
| `=` | checks for equality |
| `!=` | checks for not equals |
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

```python
print(1 == 1)    # prints True
print(0 == 1)    # prints False
print(19 >= 19)  # prints True
print(20 >= 19)  # prints True
print(19 <= 5)   # prints False
print(2 > 1)     # prints True
print(5 < 5)     # prints False
```

### Printing to Standard Output

The following words will print into standard output. Will have an error if the operation causes stack underflow.

| Word | Description |
| --- | --- |
| `,` | peeks at the top of the stack, without removing it, printing the integer. |
| `.` | pops from the top of the stack, removing it, printing the integer. |
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

String functionality is *extremely* limited and only intended to be used for printing short messages.

Any characters between `."` and `end` is evaluated as characters and printed to standard output. There is no way to save strings to the stack.

Do note that there are required space characters after and before `."` and `end` respectively. They will not be printed.

No many how many spaces or newlines there are between each word in the string, it is ignored and replaced with a single space character, *and* there is a extra space and newline character after every string. This is because I parse the file completely before interpreting it and this results in a loss of information about the original spacing between the words. But doing it this way allows me to check for potential errors before interpreting the file.

Also, if you are trying to print `end`, I am afraid that there is no way to escape the `end` keyword. You can probably just print `end.` or something, I don't know.

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

### Stack Manipulation

| Word | Description |
| --- | --- |
| `dup` | duplicates the first element |
| `drop` | pops the first element |
| `swap` | swaps the first two elements |
| `over` | duplicates the second element and pushes it to the top |
| `rot` | rotates the first 3 elements, `1 2 3 rot . . .` -- prints `1 3 2` |

### Control Flow

#### If Statement

The words between `if` and `then` are always evaluated. When `then` is encountered, the first element is popped off the stack and evaluated as a boolean. If true, the block between `then` and the next `elseif` or `end` is evaluated. After which, the evaluator will jump to the `end` word. If false, the evaluator will jump to the next `elseif` block to evaluate it.

If you want to simulate a regular `else`, as in other languages, just use a `elseif 1 then` for it to always evaluate to true.

Nested if/else are also supported!

```
if 0 then          -- evaluates to false
  12 .
elseif 1 then      -- evaluates to true
  if 2 1 - then    -- evaluates to 1 which is true
    23 .           -- 23 is printed
  elseif 1 then    -- skipped to jump ahead to the `end` block
    34 .
  end
elseif 1 then
  45 .             -- never evaluated as it is equivalent to placing 2 `else` after one another.
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

#### While Loop

The words between `while` and `then` are evaluated at the start of each loop. After which, an integer is popped off the stack and evaluated as a boolean. If this is false, the loop ends. If this is true, the words between `then` and `end` are evaluated. And this will repeat indefinitely (remember to update your control variable).

Nested loops are supported, go crazy (but maybe not *too* crazy)!

The following program is FizzBuzz from 1 to 100.

```
1
while dup 100 <= then
  if dup 15 % 0 = then
    ." FizzBuzz end
  elseif dup 3 % 0 = then
    ." Fizz end
  elseif dup 5 % 0 = then
    ." Buzz end
  elseif 1 then
    ,
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

### Memory

There is a integer array (of size 100) for the program to utilise. All values in the array is initialised to be 0 when the program is ran. It is implemented with a C array so there is random memory access.

This is created for conveniences like having global variables or when creating loops or whatever else you find a use for!

If you think this violates the idea of a stack-based language too much, then feel free to just not use these. If these keywords are not found in the program, the array is never initialised in the interpreter.

| Word | Description |
| --- | --- |
| `mem A` | pops the first element off the stack and saves it to index `A` |
| `memr A` | pushes the value of index `A` onto the stack |

### Defining Words

It is possible to define custom words, which is useful for repeated operations.

`defword <wordname> <word body> enddef`

After the definition of the custom word, every other occurrence of a word is effectively replaced by the word body.

Ideally, one adds in a "function signature" as a comment to denote how many elements the word will pop and push.

The following program computes the nth fibonaci number (this is what I came up with but there might be a better way). It is kinda tricky to have to juggle with 3 values in the stack.

```
-- duplicates top 2 elements
defword dup2 -- int, int -> int, int, int, int
  over over
enddef

-- nth fibonacci number
defword fib -- int -> int
  1 -
  0 1 rot
  while dup 0 > then
    rot rot
    dup2 + rot drop
    rot 1 -
  end
  drop drop
enddef

29 fib .                -- prints 317811
```

is equivalent to

```
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

## Tests

Some tests are available in `tests` folder, each `.stc` file is matched with a `.o` file which is the code and the expected result (stdout) respectively. These tests are run automatically by `test.py`, which must be executed in the root directory.

### Quick Usage of Tests

`./test.py` or `python3 test.py`

### Flags

| Flag | Description |
| --- | --- |
| `f` | Creates (if it does not exist) and updates all `.o` files with the current `.stc` stdout. |
| `v` | Verbose output. Displays stdout of the evaluation when there is a non-zero exit code. |

`./test.py -f` or `./test.py -fv`

## TODO

- `stackc.c` debug flag to print stack after each operation
- Power (exponent)
- Bitwise operations
- Rule 110 program
- declaration of constants and small arrays instead of `mem`?
- Allow user-defined words
- small standard library?
- read input?
