#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 constVal = float4(-1.25, 0.0, 0.75, 2.25);
    float4 expectedA = float4(0.0, 0.0, 0.84375, 1.0);
    float4 expectedB = float4(1.0, 0.0, 1.0, 1.0);
    _out.sk_FragColor = ((((((((((((((((((0.0 == expectedA.x && all(float2(0.0, 0.0) == expectedA.xy)) && all(float3(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(float4(0.0, 0.0, 0.84375, 1.0) == expectedA)) && 0.0 == expectedA.x) && all(float2(0.0, 0.0) == expectedA.xy)) && all(float3(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(float4(0.0, 0.0, 0.84375, 1.0) == expectedA)) && smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, -1.25) == expectedA.x) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, float2(-1.25, 0.0)) == expectedA.xy)) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, float3(-1.25, 0.0, 0.75)) == expectedA.xyz)) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, constVal) == expectedA)) && 1.0 == expectedB.x) && all(float2(1.0, 0.0) == expectedB.xy)) && all(float3(1.0, 0.0, 1.0) == expectedB.xyz)) && all(float4(1.0, 0.0, 1.0, 1.0) == expectedB)) && smoothstep(_uniforms.colorRed.x, _uniforms.colorGreen.x, -1.25) == expectedB.x) && all(smoothstep(_uniforms.colorRed.xy, _uniforms.colorGreen.xy, float2(-1.25, 0.0)) == expectedB.xy)) && all(smoothstep(_uniforms.colorRed.xyz, _uniforms.colorGreen.xyz, float3(-1.25, 0.0, 0.75)) == expectedB.xyz)) && all(smoothstep(_uniforms.colorRed, _uniforms.colorGreen, constVal) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
