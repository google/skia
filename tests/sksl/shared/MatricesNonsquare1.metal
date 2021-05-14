#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    bool _0_ok = true;
    float2x3 _1_m23 = float2x3(2.0);
    _0_ok = _0_ok && _1_m23[0].x == 2.0;
    float2x4 _2_m24 = float2x4(3.0);
    _0_ok = _0_ok && _2_m24[0].x == 3.0;
    float3x2 _3_m32 = float3x2(4.0);
    _0_ok = _0_ok && _3_m32[0].x == 4.0;
    float3x4 _4_m34 = float3x4(5.0);
    _0_ok = _0_ok && _4_m34[0].x == 5.0;
    float4x2 _5_m42 = float4x2(6.0);
    _0_ok = _0_ok && _5_m42[0].x == 6.0;
    float4x3 _6_m43 = float4x3(7.0);
    _0_ok = _0_ok && _6_m43[0].x == 7.0;
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
