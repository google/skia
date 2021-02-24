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
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) && all(left[1] == right[1]);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]);
}
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) && all(left[1] == right[1]) && all(left[2] == right[2]) && all(left[3] == right[3]);
}


float2 returns_float2() {
    return float2(2.0);
}
float3 returns_float3() {
    return float3(3.0);
}
float4 returns_float4() {
    return float4(4.0);
}
float2x2 returns_float2x2() {
    return float2x2(2.0);
}
float3x3 returns_float3x3() {
    return float3x3(3.0);
}
float4x4 returns_float4x4() {
    return float4x4(4.0);
}
float returns_half() {
    return 1.0;
}
float2 returns_half2() {
    return float2(2.0);
}
float3 returns_half3() {
    return float3(3.0);
}
float4 returns_half4() {
    return float4(4.0);
}
float2x2 returns_half2x2() {
    return float2x2(2.0);
}
float3x3 returns_half3x3() {
    return float3x3(3.0);
}
float4x4 returns_half4x4() {
    return float4x4(4.0);
}
bool returns_bool() {
    return true;
}
bool2 returns_bool2() {
    return bool2(true);
}
bool3 returns_bool3() {
    return bool3(true);
}
bool4 returns_bool4() {
    return bool4(true);
}
int returns_int() {
    return 1;
}
int2 returns_int2() {
    return int2(2);
}
int3 returns_int3() {
    return int3(3);
}
int4 returns_int4() {
    return int4(4);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _0_returns_float;
    float x1 = 1.0;

    float2 _1_returns_float2;
    float2 x2 = float2(2.0);

    float3 _2_returns_float3;
    float3 x3 = float3(3.0);

    float4 _3_returns_float4;
    float4 x4 = float4(4.0);

    float2x2 _4_returns_float2x2;
    float2x2 x5 = float2x2(2.0);

    float3x3 _5_returns_float3x3;
    float3x3 x6 = float3x3(3.0);

    float4x4 _6_returns_float4x4;
    float4x4 x7 = float4x4(4.0);

    float _7_returns_half;
    float x8 = 1.0;

    float2 _8_returns_half2;
    float2 x9 = float2(2.0);

    float3 _9_returns_half3;
    float3 x10 = float3(3.0);

    float4 _10_returns_half4;
    float4 x11 = float4(4.0);

    float2x2 _11_returns_half2x2;
    float2x2 x12 = float2x2(2.0);

    float3x3 _12_returns_half3x3;
    float3x3 x13 = float3x3(3.0);

    float4x4 _13_returns_half4x4;
    float4x4 x14 = float4x4(4.0);

    bool _14_returns_bool;
    bool x15 = true;

    bool2 _15_returns_bool2;
    bool2 x16 = bool2(true);

    bool3 _16_returns_bool3;
    bool3 x17 = bool3(true);

    bool4 _17_returns_bool4;
    bool4 x18 = bool4(true);

    int _18_returns_int;
    int x19 = 1;

    int2 _19_returns_int2;
    int2 x20 = int2(2);

    int3 _20_returns_int3;
    int3 x21 = int3(3);

    int4 _21_returns_int4;
    int4 x22 = int4(4);

    float _22_returns_float;
    _out.sk_FragColor = ((((((((((((((((((((x1 == 1.0 && all(x2 == returns_float2())) && all(x3 == returns_float3())) && all(x4 == returns_float4())) && x5 == returns_float2x2()) && x6 == returns_float3x3()) && x7 == returns_float4x4()) && x8 == returns_half()) && all(x9 == returns_half2())) && all(x10 == returns_half3())) && all(x11 == returns_half4())) && x12 == returns_half2x2()) && x13 == returns_half3x3()) && x14 == returns_half4x4()) && x15 == returns_bool()) && all(x16 == returns_bool2())) && all(x17 == returns_bool3())) && all(x18 == returns_bool4())) && x19 == returns_int()) && all(x20 == returns_int2())) && all(x21 == returns_int3())) && all(x22 == returns_int4()) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;

}
