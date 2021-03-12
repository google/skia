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
float _skOutParamHelper0_frexp(float _var0, thread int4& exp) {
    int _var1;
    float _skResult = frexp(_var0, _var1);
    exp.x = _var1;
    return _skResult;
}
float2 _skOutParamHelper1_frexp(float2 _var0, thread int4& exp) {
    int2 _var1;
    float2 _skResult = frexp(_var0, _var1);
    exp.xy = _var1;
    return _skResult;
}
float3 _skOutParamHelper2_frexp(float3 _var0, thread int4& exp) {
    int3 _var1;
    float3 _skResult = frexp(_var0, _var1);
    exp.xyz = _var1;
    return _skResult;
}
float4 _skOutParamHelper3_frexp(float4 _var0, thread int4& exp) {
    int4 _var1;
    float4 _skResult = frexp(_var0, _var1);
    exp = _var1;
    return _skResult;
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 value = _uniforms.colorGreen.yyyy * 6.0;
    int4 exp;
    float4 result;
    bool4 ok;
    result.x =     _skOutParamHelper0_frexp(value.x, exp);
    ok.x = result.x == 0.75 && exp.x == 3;
    result.xy =     _skOutParamHelper1_frexp(value.xy, exp);
    ok.y = result.y == 0.75 && exp.y == 3;
    result.xyz =     _skOutParamHelper2_frexp(value.xyz, exp);
    ok.z = result.z == 0.75 && exp.z == 3;
    result =     _skOutParamHelper3_frexp(value, exp);
    ok.w = result.w == 0.75 && exp.w == 3;
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
