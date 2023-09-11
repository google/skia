#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x3 left, const half2x3 right);
thread bool operator!=(const half2x3 left, const half2x3 right);

thread bool operator==(const half2x4 left, const half2x4 right);
thread bool operator!=(const half2x4 left, const half2x4 right);

thread bool operator==(const half3x2 left, const half3x2 right);
thread bool operator!=(const half3x2 left, const half3x2 right);

thread bool operator==(const half3x4 left, const half3x4 right);
thread bool operator!=(const half3x4 left, const half3x4 right);

thread bool operator==(const half4x2 left, const half4x2 right);
thread bool operator!=(const half4x2 left, const half4x2 right);

thread bool operator==(const half4x3 left, const half4x3 right);
thread bool operator!=(const half4x3 left, const half4x3 right);

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

thread bool operator==(const float2x3 left, const float2x3 right);
thread bool operator!=(const float2x3 left, const float2x3 right);

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);

thread bool operator==(const float3x2 left, const float3x2 right);
thread bool operator!=(const float3x2 left, const float3x2 right);

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
thread bool operator==(const half2x3 left, const half2x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x3 left, const half2x3 right) {
    return !(left == right);
}
thread bool operator==(const half2x4 left, const half2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x4 left, const half2x4 right) {
    return !(left == right);
}
thread bool operator==(const half3x2 left, const half3x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x2 left, const half3x2 right) {
    return !(left == right);
}
thread bool operator==(const half3x4 left, const half3x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x4 left, const half3x4 right) {
    return !(left == right);
}
thread bool operator==(const half4x2 left, const half4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x2 left, const half4x2 right) {
    return !(left == right);
}
thread bool operator==(const half4x3 left, const half4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x3 left, const half4x3 right) {
    return !(left == right);
}
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(const half3x3 left, const half3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x3 left, const half3x3 right) {
    return !(left == right);
}
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
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
bool test_half_b() {
    bool ok = true;
    half2x3 m23 = half2x3(2.0h);
    ok = ok && m23 == half2x3(half3(2.0h, 0.0h, 0.0h), half3(0.0h, 2.0h, 0.0h));
    half2x4 m24 = half2x4(3.0h);
    ok = ok && m24 == half2x4(half4(3.0h, 0.0h, 0.0h, 0.0h), half4(0.0h, 3.0h, 0.0h, 0.0h));
    half3x2 m32 = half3x2(4.0h);
    ok = ok && m32 == half3x2(half2(4.0h, 0.0h), half2(0.0h, 4.0h), half2(0.0h, 0.0h));
    half3x4 m34 = half3x4(5.0h);
    ok = ok && m34 == half3x4(half4(5.0h, 0.0h, 0.0h, 0.0h), half4(0.0h, 5.0h, 0.0h, 0.0h), half4(0.0h, 0.0h, 5.0h, 0.0h));
    half4x2 m42 = half4x2(6.0h);
    ok = ok && m42 == half4x2(half2(6.0h, 0.0h), half2(0.0h, 6.0h), half2(0.0h, 0.0h), half2(0.0h, 0.0h));
    half4x3 m43 = half4x3(7.0h);
    ok = ok && m43 == half4x3(half3(7.0h, 0.0h, 0.0h), half3(0.0h, 7.0h, 0.0h), half3(0.0h, 0.0h, 7.0h), half3(0.0h, 0.0h, 0.0h));
    half2x2 m22 = m32 * m23;
    ok = ok && m22 == half2x2(8.0h);
    half3x3 m33 = m43 * m34;
    ok = ok && m33 == half3x3(35.0h);
    m23 += (half2x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 1.0h);
    ok = ok && m23 == half2x3(half3(3.0h, 1.0h, 1.0h), half3(1.0h, 3.0h, 1.0h));
    m32 -= (half3x2(1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 2.0h);
    ok = ok && m32 == half3x2(half2(2.0h, -2.0h), half2(-2.0h, 2.0h), half2(-2.0h, -2.0h));
    m24 *= 0.25h;
    ok = ok && m24 == half2x4(half4(0.75h, 0.0h, 0.0h, 0.0h), half4(0.0h, 0.75h, 0.0h, 0.0h));
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
    float2x2 _7_m22 = _3_m32 * _1_m23;
    _0_ok = _0_ok && _7_m22 == float2x2(8.0);
    _1_m23 += (float2x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 1.0);
    _0_ok = _0_ok && _1_m23 == float2x3(float3(3.0, 1.0, 1.0), float3(1.0, 3.0, 1.0));
    _3_m32 -= (float3x2(1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * 2.0);
    _0_ok = _0_ok && _3_m32 == float3x2(float2(2.0, -2.0), float2(-2.0, 2.0), float2(-2.0, -2.0));
    _2_m24 *= 0.25;
    _0_ok = _0_ok && _2_m24 == float2x4(float4(0.75, 0.0, 0.0, 0.0), float4(0.0, 0.75, 0.0, 0.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
