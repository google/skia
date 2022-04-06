### Compilation failed:

error: 1: invalid arguments to 'float3' constructor (expected 3 scalars, but found 2)
void construct_float3_from_float_float()             { float3 x = float3(1.0, 2.0); }
                                                                  ^^^^^^^^^^^^^^^^
error: 2: invalid arguments to 'float3' constructor (expected 3 scalars, but found 4)
void construct_float3_from_float_float_float_float() { float3 x = float3(1.0, 2.0, 3.0, 4.0); }
                                                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: 'int3' is not a valid parameter to 'bool' constructor
void construct_bool_from_int3()                      { bool   x = bool(int3(77)); }
                                                                  ^^^^^^^^^^^^^^
3 errors
