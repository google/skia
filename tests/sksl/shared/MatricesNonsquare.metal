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
thread bool operator==(const float2x3 left, const float2x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x3 left, const float2x3 right) {
    return !(left == right);
}
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
    return !(left == right);
}
thread bool operator==(const float3x2 left, const float3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x2 left, const float3x2 right) {
    return !(left == right);
}
thread bool operator==(const float3x4 left, const float3x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x4 left, const float3x4 right) {
    return !(left == right);
}
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
bool test_half_b() {
    bool ok = true;
    float2x3 m23 = float2x3(2.0);
    ok = ok && m23 == float2x3(float3(2.0, 0.0, 0.0), float3(0.0, 2.0, 0.0));
    float2x4 m24 = float2x4(3.0);
    ok = ok && m24 == float2x4(float4(3.0, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0));
    float3x2 m32 = float3x2(4.0);
    ok = ok && m32 == float3x2(float2(4.0, 0.0), float2(0.0, 4.0), float2(0.0, 0.0));
    float3x4 m34 = float3x4(5.0);
    ok = ok && m34 == float3x4(float4(5.0, 0.0, 0.0, 0.0), float4(0.0, 5.0, 0.0, 0.0), float4(0.0, 0.0, 5.0, 0.0));
    float4x3 m43 = float4x3(7.0);
    ok = ok && m43 == float4x3(float3(7.0, 0.0, 0.0), float3(0.0, 7.0, 0.0), float3(0.0, 0.0, 7.0), float3(0.0, 0.0, 0.0));
    float2x2 m22 = m32 * m23;
    ok = ok && m22 == float2x2(8.0);
    float3x3 m33 = m43 * m34;
    ok = ok && m33 == float3x3(35.0);
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float2x3 _1_m23 = float2x3(2.0);
    _0_ok = _0_ok && _1_m23 == float2x3(float3(2.0, 0.0, 0.0), float3(0.0, 2.0, 0.0));
    float2x4 _2_m24 = float2x4(3.0);
    _0_ok = _0_ok && _2_m24 == float2x4(float4(3.0, 0.0, 0.0, 0.0), float4(0.0, 3.0, 0.0, 0.0));
    float3x2 _3_m32 = float3x2(4.0);
    _0_ok = _0_ok && _3_m32 == float3x2(float2(4.0, 0.0), float2(0.0, 4.0), float2(0.0, 0.0));
    float3x4 _4_m34 = float3x4(5.0);
    _0_ok = _0_ok && _4_m34 == float3x4(float4(5.0, 0.0, 0.0, 0.0), float4(0.0, 5.0, 0.0, 0.0), float4(0.0, 0.0, 5.0, 0.0));
    float4x3 _6_m43 = float4x3(7.0);
    _0_ok = _0_ok && _6_m43 == float4x3(float3(7.0, 0.0, 0.0), float3(0.0, 7.0, 0.0), float3(0.0, 0.0, 7.0), float3(0.0, 0.0, 0.0));
    float2x2 _7_m22 = _3_m32 * _1_m23;
    _0_ok = _0_ok && _7_m22 == float2x2(8.0);
    float3x3 _8_m33 = _6_m43 * _4_m34;
    _0_ok = _0_ok && _8_m33 == float3x3(35.0);
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
