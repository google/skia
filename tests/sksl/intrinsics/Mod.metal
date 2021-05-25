#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
    float4 colorWhite;
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
    float2 _skTemp2;
    float _skTemp3;
    float3 _skTemp4;
    float _skTemp5;
    float4 _skTemp6;
    float _skTemp7;
    float _skTemp8;
    float _skTemp9;
    float2 _skTemp10;
    float2 _skTemp11;
    float3 _skTemp12;
    float3 _skTemp13;
    float4 _skTemp14;
    float4 _skTemp15;
    float4 expectedA = float4(0.75, 0.0, 0.75, 0.25);
    float4 expectedB = float4(0.25, 0.0, 0.75, 1.0);
    _out.sk_FragColor = (((((((((((((((_skTemp0 = _uniforms.testInputs.x, _skTemp1 = 1.0, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1)) == expectedA.x && all((_skTemp2 = _uniforms.testInputs.xy, _skTemp3 = 1.0, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3)) == expectedA.xy)) && all((_skTemp4 = _uniforms.testInputs.xyz, _skTemp5 = 1.0, _skTemp4 - _skTemp5 * floor(_skTemp4 / _skTemp5)) == expectedA.xyz)) && all((_skTemp6 = _uniforms.testInputs, _skTemp7 = 1.0, _skTemp6 - _skTemp7 * floor(_skTemp6 / _skTemp7)) == expectedA)) && 0.75 == expectedA.x) && all(float2(0.75, 0.0) == expectedA.xy)) && all(float3(0.75, 0.0, 0.75) == expectedA.xyz)) && all(float4(0.75, 0.0, 0.75, 0.25) == expectedA)) && (_skTemp8 = _uniforms.testInputs.x, _skTemp9 = _uniforms.colorWhite.x, _skTemp8 - _skTemp9 * floor(_skTemp8 / _skTemp9)) == expectedA.x) && all((_skTemp10 = _uniforms.testInputs.xy, _skTemp11 = _uniforms.colorWhite.xy, _skTemp10 - _skTemp11 * floor(_skTemp10 / _skTemp11)) == expectedA.xy)) && all((_skTemp12 = _uniforms.testInputs.xyz, _skTemp13 = _uniforms.colorWhite.xyz, _skTemp12 - _skTemp13 * floor(_skTemp12 / _skTemp13)) == expectedA.xyz)) && all((_skTemp14 = _uniforms.testInputs, _skTemp15 = _uniforms.colorWhite, _skTemp14 - _skTemp15 * floor(_skTemp14 / _skTemp15)) == expectedA)) && 0.25 == expectedB.x) && all(float2(0.25, 0.0) == expectedB.xy)) && all(float3(0.25, 0.0, 0.75) == expectedB.xyz)) && all(float4(0.25, 0.0, 0.75, 1.0) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
