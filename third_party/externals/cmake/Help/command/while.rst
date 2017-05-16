while
-----

Evaluate a group of commands while a condition is true

::

  while(condition)
    COMMAND1(ARGS ...)
    COMMAND2(ARGS ...)
    ...
  endwhile(condition)

All commands between while and the matching endwhile are recorded
without being invoked.  Once the endwhile is evaluated, the recorded
list of commands is invoked as long as the condition is true.  The
condition is evaluated using the same logic as the if command.
