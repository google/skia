### Compilation failed:

error: 1: unsized arrays are not permitted here
struct S { float x[2][]; };
                      ^
error: 1: multi-dimensional arrays are not supported
struct S { float x[2][]; };
           ^^^^^^^^^^^^
2 errors
