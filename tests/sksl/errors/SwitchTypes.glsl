### Compilation failed:

error: 13: expected 'int', but found 'float2'
void switch_test_float2()          { switch (float2(1)) { case 1: break; } }
                                             ^^^^^^^^^
error: 14: expected 'int', but found 'float2'
void switch_case_float2()          { switch (1) { case float2(1): break; } }
                                                       ^^^^^^^^^
error: 15: expected 'int', but found 'float'
void switch_test_const_float_var() { switch (cf) { case 1: break; } }
                                             ^^
error: 16: expected 'int', but found 'float'
void switch_case_float()           { switch (1) { case 0.5: break; } }
                                                       ^^^
error: 17: expected 'int', but found 'float'
void switch_case_integral_float()  { switch (1) { case 1.0: break; } }
                                                       ^^^
error: 18: expected 'int', but found 'float'
void switch_case_uniform_float()   { switch (1) { case uf: break; } }
                                                       ^^
error: 19: case value must be a constant integer
void switch_case_uniform_int()     { switch (1) { case ui: break; } }
                                                       ^^
error: 20: expected 'int', but found 'float'
void switch_case_const_float_var() { switch (1) { case cf: break; } }
                                                       ^^
error: 21: case value must be a constant integer
void switch_case_int_var()         { switch (1) { case i: break; } }
                                                       ^
9 errors
