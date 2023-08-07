### Compilation failed:

error: 2: ternary operator result mismatch: 'bool', 'float'
void bool_or_float()   { float x = 5 > 2 ? true : 1.0; }
                                           ^^^^^^^^^^
error: 3: ternary operator result mismatch: 'float3', 'float'
void float3_or_float() { float x = 5 > 2 ? float3(1) : 1.0; }
                                           ^^^^^^^^^^^^^^^
error: 4: ternary expression of type 'void' is not allowed
void void_or_void()    { 5 > 2 ? void_function() : void_function(); }
                                 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
