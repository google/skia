### Compilation failed:

error: 1: expected 'int', but found 'float'
void vardecl_assign_float_to_int()   { int x = 1.0; }
                                               ^^^
error: 2: type mismatch: '=' cannot operate on 'int', 'float'
void statement_assign_float_to_int() { int x; x = 1.0; }
                                              ^^^^^^^
error: 3: type mismatch: '*=' cannot operate on 'int3', 'float'
void times_equals_int3_by_float()    { int3 x = int3(0); x *= 1.0; }
                                                         ^^^^^^^^
error: 4: expected '(' to begin function call
void function_ref_in_comma_expr()    { int x = (radians, 1); }
                                                       ^
error: 5: expected '(' to begin constructor invocation
void type_ref_in_comma_expr()        { int x = (bool4, 1); }
                                                     ^
error: 6: expected '(' to begin function call
int function_ref_in_global_variable = mix;
                                         ^
error: 7: expected '(' to begin constructor invocation
float3x3 type_ref_in_global_variable = float3x3;
                                               ^
7 errors
