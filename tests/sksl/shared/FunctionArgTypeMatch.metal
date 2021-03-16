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

bool takes_float2(float2 x) {
    return true;
}
bool takes_float3(float3 x) {
    return true;
}
bool takes_float4(float4 x) {
    return true;
}
bool takes_float2x2(float2x2 x) {
    return true;
}
bool takes_float3x3(float3x3 x) {
    return true;
}
bool takes_float4x4(float4x4 x) {
    return true;
}
bool takes_half(float x) {
    return true;
}
bool takes_half2(float2 x) {
    return true;
}
bool takes_half3(float3 x) {
    return true;
}
bool takes_half4(float4 x) {
    return true;
}
bool takes_half2x2(float2x2 x) {
    return true;
}
bool takes_half3x3(float3x3 x) {
    return true;
}
bool takes_half4x4(float4x4 x) {
    return true;
}
bool takes_bool(bool x) {
    return true;
}
bool takes_bool2(bool2 x) {
    return true;
}
bool takes_bool3(bool3 x) {
    return true;
}
bool takes_bool4(bool4 x) {
    return true;
}
bool takes_int(int x) {
    return true;
}
bool takes_int2(int2 x) {
    return true;
}
bool takes_int3(int3 x) {
    return true;
}
bool takes_int4(int4 x) {
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ((((((((((((((((((((true && takes_float2(float2(2.0))) && takes_float3(float3(3.0))) && takes_float4(float4(4.0))) && takes_float2x2(float2x2(2.0))) && takes_float3x3(float3x3(3.0))) && takes_float4x4(float4x4(4.0))) && takes_half(1.0)) && takes_half2(float2(2.0))) && takes_half3(float3(3.0))) && takes_half4(float4(4.0))) && takes_half2x2(float2x2(2.0))) && takes_half3x3(float3x3(3.0))) && takes_half4x4(float4x4(4.0))) && takes_bool(true)) && takes_bool2(bool2(true))) && takes_bool3(bool3(true))) && takes_bool4(bool4(true))) && takes_int(1)) && takes_int2(int2(2))) && takes_int3(int3(3))) && takes_int4(int4(4)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
