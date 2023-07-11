### Compilation failed:

error: 4: cannot assign to this expression
void test_a() { inc1(0); }
                     ^
error: 5: cannot assign to this expression
void test_b() { inc4(float4(0)); }
                     ^^^^^^^^^
error: 6: cannot assign to this expression
void test_c() { inc1(sqrt(1)); }
                     ^^^^^^^
error: 10: '$pure' is not permitted here
$pure void pure_function_with_out_param  (out float x)   { x = 1; }
^^^^^
error: 10: pure functions cannot have out parameters
$pure void pure_function_with_out_param  (out float x)   { x = 1; }
                                          ^^^
error: 10: unknown identifier 'x'
$pure void pure_function_with_out_param  (out float x)   { x = 1; }
                                                           ^
error: 11: '$pure' is not permitted here
$pure void pure_function_with_inout_param(inout float x) { x += 1; }
^^^^^
error: 11: pure functions cannot have out parameters
$pure void pure_function_with_inout_param(inout float x) { x += 1; }
                                          ^^^^^
error: 11: unknown identifier 'x'
$pure void pure_function_with_inout_param(inout float x) { x += 1; }
                                                           ^
9 errors
