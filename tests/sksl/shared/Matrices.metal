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
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0[0], x0[1]), float2(x0[2], x0[3]));
}
thread float2x2& operator*=(thread float2x2& left, thread const float2x2& right) {
    left = left * right;
    return left;
}
float2x2 float2x2_from_float_float3(float x0, float3 x1) {
    return float2x2(float2(x0, x1[0]), float2(x1[1], x1[2]));
}


bool test_half() {
    float3 v1 = float3x3(1.0) * float3(2.0);
    float3 v2 = float3(2.0) * float3x3(1.0);
    float2x2 m1 = float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0));
    float2x2 m2 = float2x2_from_float4(float4(0.0));
    float2x2 m3 = m1;
    float2x2 m4 = float2x2(1.0);
    m3 *= m4;
    float2x2 m5 = float2x2(m1[0].x);
    float2x2 m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    m6 += m5;
    float2x2 m7 = float2x2_from_float_float3(5.0, float3(6.0, 7.0, 8.0));
    float3x3 m9 = float3x3(1.0);
    float4x4 m10 = float4x4(1.0);
    float4x4 m11 = float4x4(2.0);
    m11 -= m10;
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_test_float;
    float3 _1_v1 = float3x3(1.0) * float3(2.0);
    float3 _2_v2 = float3(2.0) * float3x3(1.0);
    float2x2 _3_m1 = float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0));
    float2x2 _4_m2 = float2x2_from_float4(float4(0.0));
    float2x2 _5_m3 = _3_m1;
    float2x2 _6_m4 = float2x2(1.0);
    _5_m3 *= _6_m4;
    float2x2 _7_m5 = float2x2(_3_m1[0].x);
    float2x2 _8_m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _8_m6 += _7_m5;
    float2x2 _9_m7 = float2x2_from_float_float3(5.0, float3(6.0, 7.0, 8.0));
    float3x3 _10_m9 = float3x3(1.0);
    float4x4 _11_m10 = float4x4(1.0);
    float4x4 _12_m11 = float4x4(2.0);
    _12_m11 -= _11_m10;
    _out.sk_FragColor = true && test_half() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;

}
