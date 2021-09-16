#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread float2x2& operator*=(thread float2x2& left, thread const float2x2& right) {
    left = left * right;
    return left;
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}

float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
bool test_half_b(Uniforms _uniforms) {
    bool ok = true;
    float2x2 m1 = _uniforms.testMatrix2x2;
    ok = ok && m1 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 m3 = m1;
    ok = ok && m3 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 m4 = float2x2(6.0);
    ok = ok && m4 == float2x2(float2(6.0, 0.0), float2(0.0, 6.0));
    m3 *= m4;
    ok = ok && m3 == float2x2(float2(6.0, 12.0), float2(18.0, 24.0));
    float2x2 m5 = float2x2(m1[1].y);
    ok = ok && m5 == float2x2(float2(4.0, 0.0), float2(0.0, 4.0));
    m1 += m5;
    ok = ok && m1 == float2x2(float2(5.0, 2.0), float2(3.0, 8.0));
    float2x2 m7 = float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    ok = ok && m7 == float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    float3x3 m9 = float3x3(9.0);
    ok = ok && m9 == float3x3(float3(9.0, 0.0, 0.0), float3(0.0, 9.0, 0.0), float3(0.0, 0.0, 9.0));
    float4x4 m10 = float4x4(11.0);
    ok = ok && m10 == float4x4(float4(11.0, 0.0, 0.0, 0.0), float4(0.0, 11.0, 0.0, 0.0), float4(0.0, 0.0, 11.0, 0.0), float4(0.0, 0.0, 0.0, 11.0));
    float4x4 m11 = float4x4(float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0));
    m11 -= m10;
    ok = ok && m11 == float4x4(float4(9.0, 20.0, 20.0, 20.0), float4(20.0, 9.0, 20.0, 20.0), float4(20.0, 20.0, 9.0, 20.0), float4(20.0, 20.0, 20.0, 9.0));
    float4 h4 = float4_from_float2x2(_uniforms.testMatrix2x2);
    ok = ok && float2x2(h4.xy, float2(h4.z, 4.0)) == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    ok = ok && float3x3(float3(h4.xy, h4.z), float3(h4.w, h4.xy), float3(h4.zw, h4.x)) == float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 1.0, 2.0), float3(3.0, 4.0, 1.0));
    ok = ok && float4x4(float4(h4.xy, h4.zw), float4(h4.xyz, h4.w), h4, float4(1.0, h4.yzw)) == float4x4(float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0));
    return ok;
}
bool test_comma_b() {
    float2x2 x;
    float2x2 y;
    return ((x = float2x2(float2(1.0, 2.0), float2(3.0, 4.0)) , y = 0.5 * float2x2(float2(2.0, 4.0), float2(6.0, 8.0))) , x == y);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float2x2 _1_m1 = _uniforms.testMatrix2x2;
    _0_ok = _0_ok && _1_m1 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 _2_m3 = _1_m1;
    _0_ok = _0_ok && _2_m3 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 _3_m4 = float2x2(6.0);
    _0_ok = _0_ok && _3_m4 == float2x2(float2(6.0, 0.0), float2(0.0, 6.0));
    _2_m3 *= _3_m4;
    _0_ok = _0_ok && _2_m3 == float2x2(float2(6.0, 12.0), float2(18.0, 24.0));
    float2x2 _4_m5 = float2x2(_1_m1[1].y);
    _0_ok = _0_ok && _4_m5 == float2x2(float2(4.0, 0.0), float2(0.0, 4.0));
    _1_m1 += _4_m5;
    _0_ok = _0_ok && _1_m1 == float2x2(float2(5.0, 2.0), float2(3.0, 8.0));
    float2x2 _5_m7 = float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    _0_ok = _0_ok && _5_m7 == float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    float3x3 _6_m9 = float3x3(9.0);
    _0_ok = _0_ok && _6_m9 == float3x3(float3(9.0, 0.0, 0.0), float3(0.0, 9.0, 0.0), float3(0.0, 0.0, 9.0));
    float4x4 _7_m10 = float4x4(11.0);
    _0_ok = _0_ok && _7_m10 == float4x4(float4(11.0, 0.0, 0.0, 0.0), float4(0.0, 11.0, 0.0, 0.0), float4(0.0, 0.0, 11.0, 0.0), float4(0.0, 0.0, 0.0, 11.0));
    float4x4 _8_m11 = float4x4(float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0));
    _8_m11 -= _7_m10;
    _0_ok = _0_ok && _8_m11 == float4x4(float4(9.0, 20.0, 20.0, 20.0), float4(20.0, 9.0, 20.0, 20.0), float4(20.0, 20.0, 9.0, 20.0), float4(20.0, 20.0, 20.0, 9.0));
    float4 _9_f4 = float4_from_float2x2(_uniforms.testMatrix2x2);
    _0_ok = _0_ok && float2x2(_9_f4.xy, float2(_9_f4.z, 4.0)) == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _0_ok = _0_ok && float3x3(float3(_9_f4.xy, _9_f4.z), float3(_9_f4.w, _9_f4.xy), float3(_9_f4.zw, _9_f4.x)) == float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 1.0, 2.0), float3(3.0, 4.0, 1.0));
    _0_ok = _0_ok && float4x4(float4(_9_f4.xy, _9_f4.zw), float4(_9_f4.xyz, _9_f4.w), _9_f4, float4(1.0, _9_f4.yzw)) == float4x4(float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 2.0, 3.0, 4.0));
    _out.sk_FragColor = (_0_ok && test_half_b(_uniforms)) && test_comma_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
