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
    int _skTemp0;
    float _skTemp1;
    int2 _skTemp2;
    float2 _skTemp3;
    int3 _skTemp4;
    float3 _skTemp5;
    int4 _skTemp6;
    float4 _skTemp7;
    float4 value = float4(_uniforms.colorGreen.yyyy * 6.0h);
    int4 _0_exp;
    float4 result;
    bool4 ok;
    result.x = ((_skTemp1 = frexp(value.x, _skTemp0)), (_0_exp.x = _skTemp0), _skTemp1);
    ok.x = result.x == 0.75 && _0_exp.x == 3;
    result.xy = ((_skTemp3 = frexp(value.xy, _skTemp2)), (_0_exp.xy = _skTemp2), _skTemp3);
    ok.y = result.y == 0.75 && _0_exp.y == 3;
    result.xyz = ((_skTemp5 = frexp(value.xyz, _skTemp4)), (_0_exp.xyz = _skTemp4), _skTemp5);
    ok.z = result.z == 0.75 && _0_exp.z == 3;
    result = ((_skTemp7 = frexp(value, _skTemp6)), (_0_exp = _skTemp6), _skTemp7);
    ok.w = result.w == 0.75 && _0_exp.w == 3;
    _out.sk_FragColor = all(ok) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
