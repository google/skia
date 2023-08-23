#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
void keepAlive_vh(thread half& h) {
}
void keepAlive_vf(thread float& f) {
}
void keepAlive_vi(thread int& i) {
}
void assignToFunctionParameter_vif(int x, thread float& y) {
    x = 1;
    y = 1.0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float _skTemp0;
    float _skTemp1;
    half _skTemp2;
    int _skTemp3;
    int _skTemp4;
    int _skTemp5;
    int _skTemp6;
    half _skTemp7;
    float _skTemp8;
    half _skTemp9;
    float _skTemp10;
    float _skTemp11;
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
    ((assignToFunctionParameter_vif(0, (_skTemp0 = f3x3[0].x))), (f3x3[0].x = _skTemp0));
    half l;
    l = 0.0h;
    ai[0] += ai4[0].x;
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = half4(1.0h);
    s.ah4[0] = half4(2.0h);
    float repeat;
    repeat = (repeat = 1.0);
    ((keepAlive_vf((_skTemp1 = af4[0].x))), (af4[0].x = _skTemp1));
    ((keepAlive_vh((_skTemp2 = ah3x3[0][0].x))), (ah3x3[0][0].x = _skTemp2));
    ((keepAlive_vi((_skTemp3 = i))), (i = _skTemp3));
    ((keepAlive_vi((_skTemp4 = i4.y))), (i4.y = _skTemp4));
    ((keepAlive_vi((_skTemp5 = ai[0]))), (ai[0] = _skTemp5));
    ((keepAlive_vi((_skTemp6 = ai4[0].x))), (ai4[0].x = _skTemp6));
    ((keepAlive_vh((_skTemp7 = x.y))), (x.y = _skTemp7));
    ((keepAlive_vf((_skTemp8 = s.f))), (s.f = _skTemp8));
    ((keepAlive_vh((_skTemp9 = l))), (l = _skTemp9));
    ((keepAlive_vf((_skTemp10 = f3x3[0].x))), (f3x3[0].x = _skTemp10));
    ((keepAlive_vf((_skTemp11 = repeat))), (repeat = _skTemp11));
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
