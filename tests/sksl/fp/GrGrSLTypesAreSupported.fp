 int      test_i (int  a)       { for (;;) { return a; } }
 int2     test_i2(int2 a)       { for (;;) { return a; } }
 int3     test_i3(int3 a)       { for (;;) { return a; } }
 int4     test_i4(int4 a)       { for (;;) { return a; } }
 half3x4  test_h3x4(half3x4 a)  { for (;;) { return a; } }
 float2x4 test_f2x4(float2x4 a) { for (;;) { return a; } }

 void main() {
    sk_OutColor = test_i(1).xxxx;
    sk_OutColor = test_i2(int2(1)).xxxx;
    sk_OutColor = test_i3(int3(1)).xxxx;
    sk_OutColor = test_i4(int4(1)).xxxx;
    sk_OutColor = test_h3x4(half3x4(1))[0];
    sk_OutColor = half4(test_f2x4(float2x4(1))[0]);
}
