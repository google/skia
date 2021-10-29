#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _skTemp0;
    half _skTemp1;
    half2 _skTemp2;
    half _skTemp3;
    half3 _skTemp4;
    half _skTemp5;
    half4 _skTemp6;
    half _skTemp7;
    half _skTemp8;
    half _skTemp9;
    half2 _skTemp10;
    half2 _skTemp11;
    half3 _skTemp12;
    half3 _skTemp13;
    half4 _skTemp14;
    half4 _skTemp15;
    half4 expectedA = half4(0.75h, 0.0h, 0.75h, 0.25h);
    half4 expectedB = half4(0.25h, 0.0h, 0.75h, 1.0h);
    _out.sk_FragColor = (((((((((((((((_skTemp0 = _uniforms.testInputs.x, _skTemp1 = 1.0h, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1)) == expectedA.x && all((_skTemp2 = _uniforms.testInputs.xy, _skTemp3 = 1.0h, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3)) == expectedA.xy)) && all((_skTemp4 = _uniforms.testInputs.xyz, _skTemp5 = 1.0h, _skTemp4 - _skTemp5 * floor(_skTemp4 / _skTemp5)) == expectedA.xyz)) && all((_skTemp6 = _uniforms.testInputs, _skTemp7 = 1.0h, _skTemp6 - _skTemp7 * floor(_skTemp6 / _skTemp7)) == expectedA)) && 0.75h == expectedA.x) && all(half2(0.75h, 0.0h) == expectedA.xy)) && all(half3(0.75h, 0.0h, 0.75h) == expectedA.xyz)) && all(half4(0.75h, 0.0h, 0.75h, 0.25h) == expectedA)) && (_skTemp8 = _uniforms.testInputs.x, _skTemp9 = _uniforms.colorWhite.x, _skTemp8 - _skTemp9 * floor(_skTemp8 / _skTemp9)) == expectedA.x) && all((_skTemp10 = _uniforms.testInputs.xy, _skTemp11 = _uniforms.colorWhite.xy, _skTemp10 - _skTemp11 * floor(_skTemp10 / _skTemp11)) == expectedA.xy)) && all((_skTemp12 = _uniforms.testInputs.xyz, _skTemp13 = _uniforms.colorWhite.xyz, _skTemp12 - _skTemp13 * floor(_skTemp12 / _skTemp13)) == expectedA.xyz)) && all((_skTemp14 = _uniforms.testInputs, _skTemp15 = _uniforms.colorWhite, _skTemp14 - _skTemp15 * floor(_skTemp14 / _skTemp15)) == expectedA)) && 0.25h == expectedB.x) && all(half2(0.25h, 0.0h) == expectedB.xy)) && all(half3(0.25h, 0.0h, 0.75h) == expectedB.xyz)) && all(half4(0.25h, 0.0h, 0.75h, 1.0h) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
