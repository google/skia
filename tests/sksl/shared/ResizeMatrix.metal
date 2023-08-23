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
float2x2 float2x2_from_float3x3(float3x3 x0) {
    return float2x2(float2(x0[0].xy), float2(x0[1].xy));
}
float2x2 float2x2_from_float4x4(float4x4 x0) {
    return float2x2(float2(x0[0].xy), float2(x0[1].xy));
}
float3x3 float3x3_from_float4x4(float4x4 x0) {
    return float3x3(float3(x0[0].xyz), float3(x0[1].xyz), float3(x0[2].xyz));
}
float3x3 float3x3_from_float2x2(float2x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(0.0, 0.0, 1.0));
}
float4x4 float4x4_from_float3x3(float3x3 x0) {
    return float4x4(float4(x0[0].xyz, 0.0), float4(x0[1].xyz, 0.0), float4(x0[2].xyz, 0.0), float4(0.0, 0.0, 0.0, 1.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float result = 0.0;
    float2x2 a = float2x2_from_float3x3(float3x3(1.0));
    result += a[0].x;
    float2x2 b = float2x2_from_float4x4(float4x4(1.0));
    result += b[0].x;
    float3x3 c = float3x3_from_float4x4(float4x4(1.0));
    result += c[0].x;
    float3x3 d = float3x3_from_float2x2(float2x2(1.0));
    result += d[0].x;
    float4x4 e = float4x4_from_float3x3(float3x3_from_float2x2(float2x2(1.0)));
    result += e[0].x;
    float2x2 f = float2x2_from_float3x3(float3x3_from_float4x4(float4x4(1.0)));
    result += f[0].x;
    _out.sk_FragColor = result == 6.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
