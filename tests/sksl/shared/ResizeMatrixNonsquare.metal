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
float3x3 float3x3_from_float2x3(float2x3 x0) {
    return float3x3(float3(x0[0].xyz), float3(x0[1].xyz), float3(0.0, 0.0, 1.0));
}
float3x3 float3x3_from_float3x2(float3x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(x0[2].xy, 1.0));
}
float4x4 float4x4_from_float4x3(float4x3 x0) {
    return float4x4(float4(x0[0].xyz, 0.0), float4(x0[1].xyz, 0.0), float4(x0[2].xyz, 0.0), float4(x0[3].xyz, 1.0));
}
float4x3 float4x3_from_float4x2(float4x2 x0) {
    return float4x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(x0[2].xy, 1.0), float3(x0[3].xy, 0.0));
}
float4x4 float4x4_from_float3x4(float3x4 x0) {
    return float4x4(float4(x0[0].xyzw), float4(x0[1].xyzw), float4(x0[2].xyzw), float4(0.0, 0.0, 0.0, 1.0));
}
float3x4 float3x4_from_float2x4(float2x4 x0) {
    return float3x4(float4(x0[0].xyzw), float4(x0[1].xyzw), float4(0.0, 0.0, 1.0, 0.0));
}
float2x4 float2x4_from_float4x2(float4x2 x0) {
    return float2x4(float4(x0[0].xy, 0.0, 0.0), float4(x0[1].xy, 0.0, 0.0));
}
float4x2 float4x2_from_float2x4(float2x4 x0) {
    return float4x2(float2(x0[0].xy), float2(x0[1].xy), float2(0.0, 0.0), float2(0.0, 0.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float result = 0.0;
    float3x3 g = float3x3_from_float2x3(float2x3(1.0));
    result += g[0].x;
    float3x3 h = float3x3_from_float3x2(float3x2(1.0));
    result += h[0].x;
    float4x4 i = float4x4_from_float4x3(float4x3_from_float4x2(float4x2(1.0)));
    result += i[0].x;
    float4x4 j = float4x4_from_float3x4(float3x4_from_float2x4(float2x4(1.0)));
    result += j[0].x;
    float2x4 k = float2x4_from_float4x2(float4x2(1.0));
    result += k[0].x;
    float4x2 l = float4x2_from_float2x4(float2x4(1.0));
    result += l[0].x;
    _out.sk_FragColor = result == 6.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
