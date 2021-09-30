#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputVal;
    float4 expected;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 negativeVal = float4(-1.0, -4.0, -16.0, -64.0);
    _out.sk_FragColor = ((((((((((rsqrt(_uniforms.inputVal.x) == _uniforms.expected.x && all(rsqrt(_uniforms.inputVal.xy) == _uniforms.expected.xy)) && all(rsqrt(_uniforms.inputVal.xyz) == _uniforms.expected.xyz)) && all(rsqrt(_uniforms.inputVal) == _uniforms.expected)) && 1.0 == _uniforms.expected.x) && all(float2(1.0, 0.5) == _uniforms.expected.xy)) && all(float3(1.0, 0.5, 0.25) == _uniforms.expected.xyz)) && all(float4(1.0, 0.5, 0.25, 0.125) == _uniforms.expected)) && rsqrt(-1.0) == _uniforms.expected.x) && all(rsqrt(float2(-1.0, -4.0)) == _uniforms.expected.xy)) && all(rsqrt(float3(-1.0, -4.0, -16.0)) == _uniforms.expected.xyz)) && all(rsqrt(negativeVal) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
