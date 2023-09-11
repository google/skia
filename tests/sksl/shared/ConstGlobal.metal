#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const int SEVEN = 7;
constant const int TEN = 10;
constant const half4x4 MATRIXFIVE = half4x4(5.0h);
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half4x4 left, const half4x4 right);
thread bool operator!=(const half4x4 left, const half4x4 right);
thread bool operator==(const half4x4 left, const half4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x4 left, const half4x4 right) {
    return !(left == right);
}
bool verify_const_globals_biih44(int seven, int ten, half4x4 matrixFive) {
    return (seven == 7 && ten == 10) && matrixFive == half4x4(5.0h);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
