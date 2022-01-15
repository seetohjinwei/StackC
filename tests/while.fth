-- FizzBuzz from 1 to 100
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

-- Simple 10 to 1 loop

10
while dup 0 > then
  ,
  1 -
end
drop

-- Nested while loops

5
while dup 0 > then
  3
  while dup 0 > then
    ,
    1 -
  end
  drop
  1 -
end
drop

-- "Hi" 5 times

5
while dup 0 > then
  ." Hi end
  1 -
end
drop