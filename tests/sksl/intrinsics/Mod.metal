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
    float4 vector2 = 2.0 * _uniforms.colorWhite;
    _out.sk_FragColor = (((((((_skTemp0 = _uniforms.testInputs.x, _skTemp1 = 2.0, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1)) == 0.75 && all((_skTemp2 = _uniforms.testInputs.xy, _skTemp3 = 2.0, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3)) == float2(0.75, 0.0))) && all((_skTemp4 = _uniforms.testInputs.xyz, _skTemp5 = 2.0, _skTemp4 - _skTemp5 * floor(_skTemp4 / _skTemp5)) == float3(0.75, 0.0, 0.75))) && all((_skTemp6 = _uniforms.testInputs, _skTemp7 = 2.0, _skTemp6 - _skTemp7 * floor(_skTemp6 / _skTemp7)) == float4(0.75, 0.0, 0.75, 0.25))) && (_skTemp8 = _uniforms.testInputs.x, _skTemp9 = vector2.x, _skTemp8 - _skTemp9 * floor(_skTemp8 / _skTemp9)) == 0.75) && all((_skTemp10 = _uniforms.testInputs.xy, _skTemp11 = vector2.xy, _skTemp10 - _skTemp11 * floor(_skTemp10 / _skTemp11)) == float2(0.75, 0.0))) && all((_skTemp12 = _uniforms.testInputs.xyz, _skTemp13 = vector2.xyz, _skTemp12 - _skTemp13 * floor(_skTemp12 / _skTemp13)) == float3(0.75, 0.0, 0.75))) && all((_skTemp14 = _uniforms.testInputs, _skTemp15 = vector2, _skTemp14 - _skTemp15 * floor(_skTemp14 / _skTemp15)) == float4(0.75, 0.0, 0.75, 0.25)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
