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
float _skOutParamHelper0_modf(float _var0, thread float4& whole) {
    float _var1;
    float _skResult = modf(_var0, _var1);
    whole.x = _var1;
    return _skResult;
}
float2 _skOutParamHelper1_modf(float2 _var0, thread float4& whole) {
    float2 _var1;
    float2 _skResult = modf(_var0, _var1);
    whole.xy = _var1;
    return _skResult;
}
float3 _skOutParamHelper2_modf(float3 _var0, thread float4& whole) {
    float3 _var1;
    float3 _skResult = modf(_var0, _var1);
    whole.xyz = _var1;
    return _skResult;
}
float4 _skOutParamHelper3_modf(float4 _var0, thread float4& whole) {
    float4 _var1;
    float4 _skResult = modf(_var0, _var1);
    whole = _var1;
    return _skResult;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 value = float4(2.5, -2.5, 8.0, -0.125);
    const float4 expectedWhole = float4(2.0, -2.0, 8.0, 0.0);
    const float4 expectedFraction = float4(0.5, -0.5, 0.0, -0.125);
    bool4 ok = bool4(false);
    float4 whole;
    float4 fraction;
    fraction.x =     _skOutParamHelper0_modf(value.x, whole);
    ok.x = whole.x == 2.0 && fraction.x == 0.5;
    fraction.xy =     _skOutParamHelper1_modf(value.xy, whole);
    ok.y = all(whole.xy == float2(2.0, -2.0)) && all(fraction.xy == float2(0.5, -0.5));
    fraction.xyz =     _skOutParamHelper2_modf(value.xyz, whole);
    ok.z = all(whole.xyz == float3(2.0, -2.0, 8.0)) && all(fraction.xyz == float3(0.5, -0.5, 0.0));
    fraction =     _skOutParamHelper3_modf(value, whole);
    ok.w = all(whole == expectedWhole) && all(fraction == expectedFraction);
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
