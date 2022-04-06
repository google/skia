### Compilation failed:

error: 1: expected array, but found 'int'
void int_not_array()      { int x = 2[0]; }
                                    ^
error: 2: expected array, but found 'float'
void index_float2_twice() { float2 x = float2(0); int y = x[0][0]; }
                                                          ^^^^
2 errors
