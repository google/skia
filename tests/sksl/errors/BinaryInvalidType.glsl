### Compilation failed:

error: 2: type mismatch: '*' cannot operate on '<INVALID>', 'int'
    float x = functionLeft * 2;
              ^^^^^^^^^^^^^^^^
error: 6: type mismatch: '*' cannot operate on 'int', '<INVALID>'
    float x = 2 * functionRight;
              ^^^^^^^^^^^^^^^^^
error: 10: type mismatch: '*' cannot operate on '<INVALID>', '<INVALID>'
    float x = functionBoth * functionBoth;
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: type mismatch: '*' cannot operate on 'S', 'int'
    float x = s * 2;
              ^^^^^
error: 22: type mismatch: '*' cannot operate on 'int', 'S'
    float x = 2 * s;
              ^^^^^
error: 26: type mismatch: '*' cannot operate on 'S', 'S'
    float x = s * s;
              ^^^^^
error: 32: type mismatch: '*' cannot operate on 'shader', 'int'
    float x = shad * 2;
              ^^^^^^^^
error: 36: type mismatch: '*' cannot operate on 'int', 'shader'
    float x = 2 * shad;
              ^^^^^^^^
error: 40: type mismatch: '*' cannot operate on 'shader', 'shader'
    float x = shad * shad;
              ^^^^^^^^^^^
error: 46: type mismatch: '*' cannot operate on 'int[1]', 'int'
    float x = array * 2;
              ^^^^^^^^^
error: 50: type mismatch: '*' cannot operate on 'int', 'int[1]'
    float x = 2 * array;
              ^^^^^^^^^
error: 54: type mismatch: '*' cannot operate on 'int[1]', 'int[1]'
    float x = array * array;
              ^^^^^^^^^^^^^
12 errors
