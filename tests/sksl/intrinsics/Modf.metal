#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
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
    float _skTemp0;
    float _skTemp1;
    float2 _skTemp2;
    float2 _skTemp3;
    float3 _skTemp4;
    float3 _skTemp5;
    float4 _skTemp6;
    float4 _skTemp7;
    float4 value = float4(2.5, -2.5, 8.0, -0.125);
    const float4 expectedWhole = float4(2.0, -2.0, 8.0, 0.0);
    const float4 expectedFraction = float4(0.5, -0.5, 0.0, -0.125);
    bool4 ok = bool4(false);
    float4 whole;
    float4 fraction;
    fraction.x = ((_skTemp1 = modf(value.x, _skTemp0)), (whole.x = _skTemp0), _skTemp1);
    ok.x = whole.x == 2.0 && fraction.x == 0.5;
    fraction.xy = ((_skTemp3 = modf(value.xy, _skTemp2)), (whole.xy = _skTemp2), _skTemp3);
    ok.y = all(whole.xy == float2(2.0, -2.0)) && all(fraction.xy == float2(0.5, -0.5));
    fraction.xyz = ((_skTemp5 = modf(value.xyz, _skTemp4)), (whole.xyz = _skTemp4), _skTemp5);
    ok.z = all(whole.xyz == float3(2.0, -2.0, 8.0)) && all(fraction.xyz == float3(0.5, -0.5, 0.0));
    fraction = ((_skTemp7 = modf(value, _skTemp6)), (whole = _skTemp6), _skTemp7);
    ok.w = all(whole == expectedWhole) && all(fraction == expectedFraction);
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
