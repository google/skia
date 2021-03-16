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

bool test_half() {
    float2x2 m1 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 m3 = m1;
    float2x2 m4 = float2x2(1.0);
    m3 *= m4;
    float2x2 m5 = float2x2(m1[0].x);
    float2x2 m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    m6 += m5;
    float4x4 m10 = float4x4(1.0);
    float4x4 m11 = float4x4(2.0);
    m11 -= m10;
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 _2_m1 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 _4_m3 = _2_m1;
    float2x2 _5_m4 = float2x2(1.0);
    _4_m3 *= _5_m4;
    float2x2 _6_m5 = float2x2(_2_m1[0].x);
    float2x2 _7_m6 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _7_m6 += _6_m5;
    float4x4 _10_m10 = float4x4(1.0);
    float4x4 _11_m11 = float4x4(2.0);
    _11_m11 -= _10_m10;
    _out.sk_FragColor = true && test_half() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
