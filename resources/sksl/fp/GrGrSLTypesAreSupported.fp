 int      test_i (int  a)       { for (;;) { return a; } }
 int2     test_i2(int2 a)       { for (;;) { return a; } }
 int3     test_i3(int3 a)       { for (;;) { return a; } }
 int4     test_i4(int4 a)       { for (;;) { return a; } }
 half3x3  test_h3x3(half3x3 a)  { for (;;) { return a; } }
 float2x2 test_f2x2(float2x2 a) { for (;;) { return a; } }

 half4 main() {
    return test_i(1).xxxx
           , test_i2(int2(1)).xxxx
           , test_i3(int3(1)).xxxx
           , test_i4(int4(1)).xxxx
           , test_h3x3(half3x3(1))[0]
           , test_f2x2(float2x2(1))[0]
           , half4(1);
}
