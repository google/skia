### Compilation failed:

error: 1: type mismatch: '*' cannot operate on 'int', 'bool'
void int_times_bool()      { float x = 3 * true; }
                                       ^^^^^^^^
error: 2: type mismatch: '||' cannot operate on 'int', 'float'
void int_or_float()        { bool x = 1 || 2.0; }
                                      ^^^^^^^^
error: 3: type mismatch: '==' cannot operate on 'float2', 'int'
void float2_eq_int()       { bool x = float2(0) == 0; }
                                      ^^^^^^^^^^^^^^
error: 4: type mismatch: '!=' cannot operate on 'float2', 'int'
void float2_neq_int()      { bool x = float2(0) != 0; }
                                      ^^^^^^^^^^^^^^
error: 5: type mismatch: '^^' cannot operate on 'int', 'int'
void int_logicalxor_int()  { bool x = 8 ^^ 6; }
                                      ^^^^^^
error: 7: type mismatch: '<' cannot operate on 'float2', 'float2'
void float2_lt_float2()    { bool x = float2(0) < float2(1); }
                                      ^^^^^^^^^^^^^^^^^^^^^
error: 8: type mismatch: '<' cannot operate on 'float2', 'float'
void float2_lt_float()     { bool x = float2(0) < 0.0; }
                                      ^^^^^^^^^^^^^^^
error: 9: type mismatch: '<' cannot operate on 'float', 'float2'
void float_lt_float2()     { bool x = 0.0 < float2(0); }
                                      ^^^^^^^^^^^^^^^
8 errors
