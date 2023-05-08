#include <metal_stdlib>
#include <simd/simd.h>
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
float _skOutParamHelper0_frexp(float _var0, thread int4& _0_exp) {
    int _var1;
    float _skResult = frexp(_var0, _var1);
    _0_exp.x = _var1;
    return _skResult;
}
float2 _skOutParamHelper1_frexp(float2 _var0, thread int4& _0_exp) {
    int2 _var1;
    float2 _skResult = frexp(_var0, _var1);
    _0_exp.xy = _var1;
    return _skResult;
}
float3 _skOutParamHelper2_frexp(float3 _var0, thread int4& _0_exp) {
    int3 _var1;
    float3 _skResult = frexp(_var0, _var1);
    _0_exp.xyz = _var1;
    return _skResult;
}
float4 _skOutParamHelper3_frexp(float4 _var0, thread int4& _0_exp) {
    int4 _var1;
    float4 _skResult = frexp(_var0, _var1);
    _0_exp = _var1;
    return _skResult;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 value = float4(_uniforms.colorGreen.yyyy * 6.0h);
    int4 _0_exp;
    float4 result;
    bool4 ok;
    result.x =     _skOutParamHelper0_frexp(value.x, _0_exp);
    ok.x = result.x == 0.75 && _0_exp.x == 3;
    result.xy =     _skOutParamHelper1_frexp(value.xy, _0_exp);
    ok.y = result.y == 0.75 && _0_exp.y == 3;
    result.xyz =     _skOutParamHelper2_frexp(value.xyz, _0_exp);
    ok.z = result.z == 0.75 && _0_exp.z == 3;
    result =     _skOutParamHelper3_frexp(value, _0_exp);
    ok.w = result.w == 0.75 && _0_exp.w == 3;
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
