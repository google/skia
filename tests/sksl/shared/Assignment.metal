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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    int i;
    i = 0;
    int4 i4;
    i4 = int4(1, 2, 3, 4);
    float3x3 f3x3;
    f3x3 = float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    half4 x;
    x.w = 0.0h;
    x.yx = half2(0.0h);
    array<int, 1> ai;
    ai[0] = 0;
    array<int4, 1> ai4;
    ai4[0] = int4(1, 2, 3, 4);
    array<half3x3, 1> ah2x4;
    ah2x4[0] = half3x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h));
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
    af4[0] *= float(ah2x4[0][0].x);
    i4.y = i4.y * i;
    x.y = x.y * l;
    s.f *= float(l);
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
