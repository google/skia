### Compilation failed:

error: 2: layout qualifier 'binding' is not permitted on a struct field
    layout(binding = 1) int x;  // Not allowed
    ^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: layout qualifier 'set' is not permitted on a struct field
    layout(set = 0) int y;      // Not allowed
    ^^^^^^^^^^^^^^^^^^^^^
2 errors
