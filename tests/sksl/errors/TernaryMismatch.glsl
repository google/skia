### Compilation failed:

error: 1: ternary operator result mismatch: 'bool', 'float'
void bool_or_float()   { float x = 5 > 2 ? true : 1.0; }
                                           ^^^^^^^^^^
error: 2: ternary operator result mismatch: 'float3', 'float'
void float3_or_float() { float x = 5 > 2 ? float3(1) : 1.0; }
                                           ^^^^^^^^^^^^^^^
2 errors
