### Compilation failed:

error: 4: layout qualifier 'binding' is not permitted on a struct field
    layout(binding=1) A y;  // Not allowed
    ^^^^^^^^^^^^^^^^^^^^^^
error: 5: layout qualifier 'set' is not permitted on a struct field
    layout(set=0) A z;      // Not allowed
    ^^^^^^^^^^^^^^^^^^
2 errors
