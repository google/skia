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
thread float3x3& operator*=(thread float3x3& left, thread const float3x3& right) {
    left = left * right;
    return left;
}
thread float4x4& operator*=(thread float4x4& left, thread const float4x4& right) {
    left = left * right;
    return left;
}

bool test_half() {
    float2x3 m23 = float2x3(23.0);
    float2x4 m24 = float2x4(24.0);
    float3x2 m32 = float3x2(32.0);
    float3x4 m34 = float3x4(34.0);
    float4x2 m42 = float4x2(42.0);
    float4x3 m43 = float4x3(44.0);
    float2x2 m22 = m32 * m23;
    m22 *= m22;
    float3x3 m33 = m43 * m34;
    m33 *= m33;
    float4x4 m44 = m24 * m42;
    m44 *= m44;
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x3 _0_m23 = float2x3(23.0);
    float2x4 _1_m24 = float2x4(24.0);
    float3x2 _2_m32 = float3x2(32.0);
    float3x4 _3_m34 = float3x4(34.0);
    float4x2 _4_m42 = float4x2(42.0);
    float4x3 _5_m43 = float4x3(44.0);
    float2x2 _6_m22 = _2_m32 * _0_m23;
    _6_m22 *= _6_m22;
    float3x3 _7_m33 = _5_m43 * _3_m34;
    _7_m33 *= _7_m33;
    float4x4 _8_m44 = _1_m24 * _4_m42;
    _8_m44 *= _8_m44;
    _out.sk_FragColor = true && test_half() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
