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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 _0_m22 = float3x2(32.0) * float2x3(23.0);
    _0_m22 *= _0_m22;
    float3x3 _1_m33 = float4x3(44.0) * float3x4(34.0);
    _1_m33 *= _1_m33;
    float4x4 _2_m44 = float2x4(24.0) * float4x2(42.0);
    _2_m44 *= _2_m44;
    float2x2 _3_m22 = float3x2(32.0) * float2x3(23.0);
    _3_m22 *= _3_m22;
    float3x3 _4_m33 = float4x3(44.0) * float3x4(34.0);
    _4_m33 *= _4_m33;
    float4x4 _5_m44 = float2x4(24.0) * float4x2(42.0);
    _5_m44 *= _5_m44;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;


}
