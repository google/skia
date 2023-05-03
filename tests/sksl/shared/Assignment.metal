#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float f;
    array<float, 5> af;
    half4 h4;
    array<half4, 5> ah4;
};
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    half4 globalVar;
    S globalStruct;
};
void keepAlive_vf(thread float& f);
void _skOutParamHelper0_keepAlive_vf(thread array<float4, 1>& af4) {
    float _var0 = af4[0].x;
    keepAlive_vf(_var0);
    af4[0].x = _var0;
}
void keepAlive_vh(thread half& h);
void _skOutParamHelper1_keepAlive_vh(thread array<half3x3, 1>& ah3x3) {
    half _var0 = ah3x3[0][0].x;
    keepAlive_vh(_var0);
    ah3x3[0][0].x = _var0;
}
void keepAlive_vi(thread int& i);
void _skOutParamHelper2_keepAlive_vi(thread int& i) {
    int _var0 = i;
    keepAlive_vi(_var0);
    i = _var0;
}
void keepAlive_vi(thread int& i);
void _skOutParamHelper3_keepAlive_vi(thread int4& i4) {
    int _var0 = i4.y;
    keepAlive_vi(_var0);
    i4.y = _var0;
}
void keepAlive_vi(thread int& i);
void _skOutParamHelper4_keepAlive_vi(thread array<int, 1>& ai) {
    int _var0 = ai[0];
    keepAlive_vi(_var0);
    ai[0] = _var0;
}
void keepAlive_vi(thread int& i);
void _skOutParamHelper5_keepAlive_vi(thread array<int4, 1>& ai4) {
    int _var0 = ai4[0].x;
    keepAlive_vi(_var0);
    ai4[0].x = _var0;
}
void keepAlive_vh(thread half& h);
void _skOutParamHelper6_keepAlive_vh(thread half4& x) {
    half _var0 = x.y;
    keepAlive_vh(_var0);
    x.y = _var0;
}
void keepAlive_vf(thread float& f);
void _skOutParamHelper7_keepAlive_vf(thread S& s) {
    float _var0 = s.f;
    keepAlive_vf(_var0);
    s.f = _var0;
}
void keepAlive_vh(thread half& h);
void _skOutParamHelper8_keepAlive_vh(thread half& l) {
    half _var0 = l;
    keepAlive_vh(_var0);
    l = _var0;
}
void keepAlive_vf(thread float& f);
void _skOutParamHelper9_keepAlive_vf(thread float3x3& f3x3) {
    float _var0 = f3x3[0].x;
    keepAlive_vf(_var0);
    f3x3[0].x = _var0;
}
void keepAlive_vf(thread float& f);
void _skOutParamHelper10_keepAlive_vf(thread float& repeat) {
    float _var0 = repeat;
    keepAlive_vf(_var0);
    repeat = _var0;
}
void keepAlive_vh(thread half& h) {
}
void keepAlive_vf(thread float& f) {
}
void keepAlive_vi(thread int& i) {
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    int i = 0;
    int4 i4 = int4(1, 2, 3, 4);
    float3x3 f3x3 = float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    half4 x;
    x.w = 0.0h;
    x.yx = half2(0.0h);
    array<int, 1> ai;
    ai[0] = 0;
    array<int4, 1> ai4;
    ai4[0] = int4(1, 2, 3, 4);
    array<half3x3, 1> ah3x3;
    ah3x3[0] = half3x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h));
    array<float4, 1> af4;
    af4[0].x = 0.0;
    af4[0].ywxz = float4(1.0);
    S s;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4.zxy = half3(9.0h);
    s.ah4[2].yw = half2(5.0h);
    _globals.globalVar = half4(0.0h);
    _globals.globalStruct.f = 0.0;
    half l;
    l = 0.0h;
    ai[0] += ai4[0].x;
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = half4(1.0h);
    s.ah4[0] = half4(2.0h);
    float repeat;
    repeat = (repeat = 1.0);
    _skOutParamHelper0_keepAlive_vf(af4);
    _skOutParamHelper1_keepAlive_vh(ah3x3);
    _skOutParamHelper2_keepAlive_vi(i);
    _skOutParamHelper3_keepAlive_vi(i4);
    _skOutParamHelper4_keepAlive_vi(ai);
    _skOutParamHelper5_keepAlive_vi(ai4);
    _skOutParamHelper6_keepAlive_vh(x);
    _skOutParamHelper7_keepAlive_vf(s);
    _skOutParamHelper8_keepAlive_vh(l);
    _skOutParamHelper9_keepAlive_vf(f3x3);
    _skOutParamHelper10_keepAlive_vf(repeat);
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
