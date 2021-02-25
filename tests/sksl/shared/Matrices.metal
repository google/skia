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
thread float2x2& operator*=(thread float2x2& left, thread const float2x2& right) {
    left = left * right;
    return left;
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 _0_m3 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _0_m3 *= float2x2(1.0);
    float2x2 _1_m5 = float2x2(float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x);
    float2x2 _2_m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _2_m6 += _1_m5;
    float4x4 _3_m11 = float4x4(2.0);
    _3_m11 -= float4x4(1.0);
    float2x2 _4_m3 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _4_m3 *= float2x2(1.0);
    float2x2 _5_m5 = float2x2(float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x);
    float2x2 _6_m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _6_m6 += _5_m5;
    float4x4 _7_m11 = float4x4(2.0);
    _7_m11 -= float4x4(1.0);
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;


}
