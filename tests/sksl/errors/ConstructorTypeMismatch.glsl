### Compilation failed:

error: 5: expected 'int', but found 'float2x2'
void construct_struct_from_float2x2()   { Foo x = Foo(float2x2(0)); }
                                                      ^^^^^^^^^^^
error: 6: 'Foo' is not a valid parameter to 'float' constructor
void construct_float_from_struct()      { float x = float(foo); }
                                                    ^^^^^^^^^^
error: 7: 'Foo' is not a valid parameter to 'float2' constructor
void construct_float2_from_struct()     { float2 x = float2(foo); }
                                                     ^^^^^^^^^^^
error: 8: 'Foo' is not a valid parameter to 'float2x2' constructor
void construct_float2x2_from_struct()   { float2x2 x = float2x2(foo); }
                                                       ^^^^^^^^^^^^^
error: 9: '<INVALID>' is not a valid parameter to 'float2x2' constructor
void construct_float2x2_from_type()     { float2x2 x = float2x2(int); }
                                                       ^^^^^^^^^^^^^
error: 10: '<INVALID>' is not a valid parameter to 'float2x2' constructor
void construct_float2x2_from_function() { float2x2 x = float2x2(sqrt); }
                                                       ^^^^^^^^^^^^^^
6 errors
