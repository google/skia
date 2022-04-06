### Compilation failed:

error: 4: function 'testOut' never assigns a value to out parameter 'b'
void testOut(out float2 a, out float2 b, out float2 c)         { a = float2(1); }
                           ^^^^^^^^^^^^
error: 4: function 'testOut' never assigns a value to out parameter 'c'
void testOut(out float2 a, out float2 b, out float2 c)         { a = float2(1); }
                                         ^^^^^^^^^^^^
2 errors
