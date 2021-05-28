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
    float4 expectedA = float4(-1.0, 0.0, 0.75, 1.0);
    const float4 clampLow = float4(-1.0, -2.0, -2.0, 1.0);
    float4 expectedB = float4(-1.0, 0.0, 0.5, 2.25);
    const float4 clampHigh = float4(1.0, 2.0, 0.5, 3.0);
    _out.sk_FragColor = ((((((((((((((clamp(_uniforms.testInputs.x, -1.0, 1.0) == expectedA.x && all(clamp(_uniforms.testInputs.xy, -1.0, 1.0) == expectedA.xy)) && all(clamp(_uniforms.testInputs.xyz, -1.0, 1.0) == expectedA.xyz)) && all(clamp(_uniforms.testInputs, -1.0, 1.0) == expectedA)) && -1.0 == expectedA.x) && all(float2(-1.0, 0.0) == expectedA.xy)) && all(float3(-1.0, 0.0, 0.75) == expectedA.xyz)) && all(float4(-1.0, 0.0, 0.75, 1.0) == expectedA)) && clamp(_uniforms.testInputs.x, -1.0, 1.0) == expectedB.x) && all(clamp(_uniforms.testInputs.xy, float2(-1.0, -2.0), float2(1.0, 2.0)) == expectedB.xy)) && all(clamp(_uniforms.testInputs.xyz, float3(-1.0, -2.0, -2.0), float3(1.0, 2.0, 0.5)) == expectedB.xyz)) && all(clamp(_uniforms.testInputs, clampLow, clampHigh) == expectedB)) && -1.0 == expectedB.x) && all(float2(-1.0, 0.0) == expectedB.xy)) && all(float3(-1.0, 0.0, 0.5) == expectedB.xyz)) && all(float4(-1.0, 0.0, 0.5, 2.25) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
