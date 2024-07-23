### Compilation failed:

error: 1: unsized arrays are not permitted here
struct S { int disallowed[]; } x;
                          ^
error: 2: struct 'S' must contain at least one field
struct S {} y;
^^^^^^^^^^^
error: 2: symbol 'S' was already defined
struct S {} y;
^^^^^^^^^^^
error: 5: type mismatch: '!=' cannot operate on 'S', 'S'
    return x != y;
           ^^^^^^
4 errors
