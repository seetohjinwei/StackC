# Standard Library

This is the documentation for the standard library for StackC. It is manually updated so *might* be outdated. However, the source code is right there if you want to inspect it :D

## Printing Operations

| Word | Description |
| --- | --- |
| `,` | Pop but without removing the element from the stack. |
| `cr` | Prints a new line. |
| `print` | Pops and prints the string at the top of the stack. |

## Stack Operations

| Word | Description |
| --- | --- |
| `dup2` | A, B -> A, B, A, B |
| `rot3` | A, B, C -> C, A, B |

## Math Operations

Some common (or not so common) mathematical operations. StackC does not support floating point numbers so there aren't many operations to have here.

| Word | Description |
| --- | --- |
| `abs` | a -> \|a\| |
| `factorial` | n -> n! |
| `gcd` | a, b -> gcd(a, b) |
| `min` | a, b -> min(a, b) |
| `max` | a, b -> max(a, b) |
| `neg` | a -> -a |
| `pow` | a, b -> a^b |
| `sumN` | Pops `n` then sums up the next `n` integers of the stack. |

```
1 2 3 0 4 sumN .  -- prints 6    (= 1 + 2 + 3 + 0)
5 factorial .     -- prints 120  (= 5!)
4 5 pow .         -- prints 1024 (= 4^5)
```
