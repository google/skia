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
bool takes_void_b() {
    return true;
}
bool takes_float_bf(float x) {
    return true;
}
bool takes_float2_bf2(float2 x) {
    return true;
}
bool takes_float3_bf3(float3 x) {
    return true;
}
bool takes_float4_bf4(float4 x) {
    return true;
}
bool takes_float2x2_bf22(float2x2 x) {
    return true;
}
bool takes_float3x3_bf33(float3x3 x) {
    return true;
}
bool takes_float4x4_bf44(float4x4 x) {
    return true;
}
bool takes_half_bh(float x) {
    return true;
}
bool takes_half2_bh2(float2 x) {
    return true;
}
bool takes_half3_bh3(float3 x) {
    return true;
}
bool takes_half4_bh4(float4 x) {
    return true;
}
bool takes_half2x2_bh22(float2x2 x) {
    return true;
}
bool takes_half3x3_bh33(float3x3 x) {
    return true;
}
bool takes_half4x4_bh44(float4x4 x) {
    return true;
}
bool takes_bool_bb(bool x) {
    return true;
}
bool takes_bool2_bb2(bool2 x) {
    return true;
}
bool takes_bool3_bb3(bool3 x) {
    return true;
}
bool takes_bool4_bb4(bool4 x) {
    return true;
}
bool takes_int_bi(int x) {
    return true;
}
bool takes_int2_bi2(int2 x) {
    return true;
}
bool takes_int3_bi3(int3 x) {
    return true;
}
bool takes_int4_bi4(int4 x) {
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = ((((((((((((((((((((((true && takes_void_b()) && takes_float_bf(1.0)) && takes_float2_bf2(float2(2.0))) && takes_float3_bf3(float3(3.0))) && takes_float4_bf4(float4(4.0))) && takes_float2x2_bf22(float2x2(2.0))) && takes_float3x3_bf33(float3x3(3.0))) && takes_float4x4_bf44(float4x4(4.0))) && takes_half_bh(1.0)) && takes_half2_bh2(float2(2.0))) && takes_half3_bh3(float3(3.0))) && takes_half4_bh4(float4(4.0))) && takes_half2x2_bh22(float2x2(2.0))) && takes_half3x3_bh33(float3x3(3.0))) && takes_half4x4_bh44(float4x4(4.0))) && takes_bool_bb(true)) && takes_bool2_bb2(bool2(true))) && takes_bool3_bb3(bool3(true))) && takes_bool4_bb4(bool4(true))) && takes_int_bi(1)) && takes_int2_bi2(int2(2))) && takes_int3_bi3(int3(3))) && takes_int4_bi4(int4(4)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
