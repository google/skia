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

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

thread bool operator==(const half4x4 left, const half4x4 right);
thread bool operator!=(const half4x4 left, const half4x4 right);
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
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
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
thread bool operator==(const half4x4 left, const half4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x4 left, const half4x4 right) {
    return !(left == right);
}
float2 returns_float2_f2() {
    return float2(2.0);
}
float3 returns_float3_f3() {
    return float3(3.0);
}
float4 returns_float4_f4() {
    return float4(4.0);
}
float2x2 returns_float2x2_f22() {
    return float2x2(2.0);
}
float3x3 returns_float3x3_f33() {
    return float3x3(3.0);
}
float4x4 returns_float4x4_f44() {
    return float4x4(4.0);
}
half returns_half_h() {
    return 1.0h;
}
half2 returns_half2_h2() {
    return half2(2.0h);
}
half3 returns_half3_h3() {
    return half3(3.0h);
}
half4 returns_half4_h4() {
    return half4(4.0h);
}
half2x2 returns_half2x2_h22() {
    return half2x2(2.0h);
}
half3x3 returns_half3x3_h33() {
    return half3x3(3.0h);
}
half4x4 returns_half4x4_h44() {
    return half4x4(4.0h);
}
bool returns_bool_b() {
    return true;
}
bool2 returns_bool2_b2() {
    return bool2(true);
}
bool3 returns_bool3_b3() {
    return bool3(true);
}
bool4 returns_bool4_b4() {
    return bool4(true);
}
int returns_int_i() {
    return 1;
}
int2 returns_int2_i2() {
    return int2(2);
}
int3 returns_int3_i3() {
    return int3(3);
}
int4 returns_int4_i4() {
    return int4(4);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x1 = 1.0;
    float2 x2 = float2(2.0);
    float3 x3 = float3(3.0);
    float4 x4 = float4(4.0);
    float2x2 x5 = float2x2(2.0);
    float3x3 x6 = float3x3(3.0);
    float4x4 x7 = float4x4(4.0);
    half x8 = 1.0h;
    half2 x9 = half2(2.0h);
    half3 x10 = half3(3.0h);
    half4 x11 = half4(4.0h);
    half2x2 x12 = half2x2(2.0h);
    half3x3 x13 = half3x3(3.0h);
    half4x4 x14 = half4x4(4.0h);
    bool x15 = true;
    bool2 x16 = bool2(true);
    bool3 x17 = bool3(true);
    bool4 x18 = bool4(true);
    int x19 = 1;
    int2 x20 = int2(2);
    int3 x21 = int3(3);
    int4 x22 = int4(4);
    _out.sk_FragColor = ((((((((((((((((((((x1 == 1.0 && all(x2 == returns_float2_f2())) && all(x3 == returns_float3_f3())) && all(x4 == returns_float4_f4())) && x5 == returns_float2x2_f22()) && x6 == returns_float3x3_f33()) && x7 == returns_float4x4_f44()) && x8 == returns_half_h()) && all(x9 == returns_half2_h2())) && all(x10 == returns_half3_h3())) && all(x11 == returns_half4_h4())) && x12 == returns_half2x2_h22()) && x13 == returns_half3x3_h33()) && x14 == returns_half4x4_h44()) && x15 == returns_bool_b()) && all(x16 == returns_bool2_b2())) && all(x17 == returns_bool3_b3())) && all(x18 == returns_bool4_b4())) && x19 == returns_int_i()) && all(x20 == returns_int2_i2())) && all(x21 == returns_int3_i3())) && all(x22 == returns_int4_i4()) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
