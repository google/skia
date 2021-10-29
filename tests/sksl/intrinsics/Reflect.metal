#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 I;
    float4 N;
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
    float _skTemp0;
    float _skTemp1;
    float expectedX = -49.0;
    float2 expectedXY = float2(-169.0, 202.0);
    float3 expectedXYZ = float3(-379.0, 454.0, -529.0);
    float4 expectedXYZW = float4(-699.0, 838.0, -977.0, 1116.0);
    _out.sk_FragColor = (((((((_skTemp0 = _uniforms.I.x, _skTemp1 = _uniforms.N.x, _skTemp0 - 2 * _skTemp1 * _skTemp0 * _skTemp1) == expectedX && all(reflect(_uniforms.I.xy, _uniforms.N.xy) == expectedXY)) && all(reflect(_uniforms.I.xyz, _uniforms.N.xyz) == expectedXYZ)) && all(reflect(_uniforms.I, _uniforms.N) == expectedXYZW)) && -49.0 == expectedX) && all(float2(-169.0, 202.0) == expectedXY)) && all(float3(-379.0, 454.0, -529.0) == expectedXYZ)) && all(float4(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
