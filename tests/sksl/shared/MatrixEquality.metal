#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half2x2 testMatrix2x2;
    half3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(const half3x3 left, const half3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x3 left, const half3x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    _0_ok = _0_ok && _uniforms.testMatrix2x2 == half2x2(half2(1.0h, 2.0h), half2(3.0h, 4.0h));
    _0_ok = _0_ok && _uniforms.testMatrix3x3 == half3x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h));
    _0_ok = _0_ok && _uniforms.testMatrix2x2 != half2x2(100.0h);
    _0_ok = _0_ok && _uniforms.testMatrix3x3 != half3x3(half3(9.0h, 8.0h, 7.0h), half3(6.0h, 5.0h, 4.0h), half3(3.0h, 2.0h, 1.0h));
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
