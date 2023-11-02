### Compilation failed:

error: 1: interface block 'a' must contain at least one field
a {} S;
^
error: 2: struct 'S' must contain at least one field
struct S{} S[v];
^^^^^^^^^^
error: 2: symbol 'S' was already defined
struct S{} S[v];
^^^^^^^^^^
error: 2: unknown identifier 'v'
struct S{} S[v];
             ^
error: 2: symbol 'S' was already defined
struct S{} S[v];
           ^^^^
error: 3: expected an identifier, but found '{'
S {} p[]
  ^
6 errors
