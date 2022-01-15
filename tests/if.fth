-- 43 is +, 45 is -

if 0 then
  45 emit
end

if 1 then
  43 emit
end

if 0 then
  45 emit
elseif 1 then
  43 emit
end

if 1 then
  43 emit
elseif 1 then
  45 emit
elseif 0 then
  45 emit
end

0
if dup 5 > then
  . ." is more than 5 end
elseif 1 then
  . ." is less than 5 end
end

10
if dup 5 > then
  . ." is more than 5 end
elseif 1 then
  . ." is less than 5 end
end

-- nested loops

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

if 1 then
  if 1 then
    if 1 then
      43 emit      -- printed
    end
  elseif 1 then
    45 emit
  end
  43 emit          -- printed
end
