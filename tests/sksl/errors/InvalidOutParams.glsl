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
3 errors
