-- 43 is +, 45 is -
-- not nested

0 if
  45 emit
end

1 if
  43 emit
end

0 if
  45 emit
else
  43 emit
end

0 1 if
  43 emit
elseif
  45 emit
else
  45 emit
end

1 0 if
  45 emit
elseif
  43 emit
else
  45 emit
end

0 0 if
  45 emit
elseif
  45 emit
else
  43 emit
end

-- nested tests

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

1 if
  1 if
    43 emit
  else
    45 emit
  end
else
  45 emit
end

0 if
  45 emit
else
  1 if
    43 emit
  else
    45 emit
  end
end

1 1 1
if
  if
    if
      43 emit
    end
  else
    45 emit
  end
else
  45 emit
end

1 if
  1 0
  if
    45 emit
  elseif
    43 emit
  else
    45 emit
  end
else
  45 emit
end
