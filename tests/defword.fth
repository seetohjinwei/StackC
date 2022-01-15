-- duplicates top 2 elements
defword dup2 -- int, int -> int, int, int, int
  over over
enddef

2 3 dup2 . . . .

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

10 fib .
29 fib .

defword sum3 + + enddef
1 2 3 sum3 .
3 3 3 sum3 .

defword ++ 1 + enddef
65 ++ .
