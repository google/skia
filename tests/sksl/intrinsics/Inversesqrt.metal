#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 inputVal;
    half4 expected;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const half4 negativeVal = half4(-1.0h, -4.0h, -16.0h, -64.0h);
    _out.sk_FragColor = ((((((((((rsqrt(_uniforms.inputVal.x) == _uniforms.expected.x && all(rsqrt(_uniforms.inputVal.xy) == _uniforms.expected.xy)) && all(rsqrt(_uniforms.inputVal.xyz) == _uniforms.expected.xyz)) && all(rsqrt(_uniforms.inputVal) == _uniforms.expected)) && 1.0h == _uniforms.expected.x) && all(half2(1.0h, 0.5h) == _uniforms.expected.xy)) && all(half3(1.0h, 0.5h, 0.25h) == _uniforms.expected.xyz)) && all(half4(1.0h, 0.5h, 0.25h, 0.125h) == _uniforms.expected)) && rsqrt(-1.0h) == _uniforms.expected.x) && all(rsqrt(half2(-1.0h, -4.0h)) == _uniforms.expected.xy)) && all(rsqrt(half3(-1.0h, -4.0h, -16.0h)) == _uniforms.expected.xyz)) && all(rsqrt(negativeVal) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
