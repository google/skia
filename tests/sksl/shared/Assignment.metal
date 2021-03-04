#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float f;
    array<float, 5> af;
    float4 h4;
    array<float4, 5> ah4;
};
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int4 i4;
    i4 = int4(1, 2, 3, 4);
    float4 x;
    x.w = 0.0;
    x.yx = float2(0.0);
    array<int, 1> ai;
    ai[0] = 0;
    array<int4, 1> ai4;
    ai4[0] = int4(1, 2, 3, 4);
    array<float3x3, 1> ah2x4;
    ah2x4[0] = float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    array<float4, 1> af4;
    af4[0].x = 0.0;
    af4[0].ywxz = float4(1.0);
    S s;
    s.f = 0.0;
    s.af[1] = 0.0;
    s.h4.zxy = float3(9.0);
    s.ah4[2].yw = float2(5.0);
    ai[0] += ai4[0].x;
    s.f = 1.0;
    s.af[0] = 2.0;
    s.h4 = float4(1.0);
    s.ah4[0] = float4(2.0);
    af4[0] *= ah2x4[0][0].x;
    i4.y = i4.y * 0;
    x.y = x.y * 0.0;
    s.f *= 0.0;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
