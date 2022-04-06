### Compilation failed:

error: 1: no match for sin(int, int)
void sin_int_int() { float x = sin(1, 2); }
                               ^^^^^^^^^
error: 2: no match for sin(bool)
void sin_bool()    { float x = sin(true); }
                               ^^^^^^^^^
2 errors
