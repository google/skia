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
    float4 value = _uniforms.colorGreen.yyyy * 2.5;
    float4 whole;
    float4 fraction;
    bool4 ok;
    fraction.x =     _skOutParamHelper0_modf(value.x, whole);
    ok.x = whole.x == 2.0 && fraction.x == 0.5;
    fraction.xy =     _skOutParamHelper1_modf(value.xy, whole);
    ok.y = whole.y == 2.0 && fraction.y == 0.5;
    fraction.xyz =     _skOutParamHelper2_modf(value.xyz, whole);
    ok.z = whole.z == 2.0 && fraction.z == 0.5;
    fraction =     _skOutParamHelper3_modf(value, whole);
    ok.w = whole.w == 2.0 && fraction.w == 0.5;
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
