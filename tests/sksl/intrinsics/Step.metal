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
    const float4 constGreen = float4(0.0, 1.0, 0.0, 1.0);
    float4 expectedA = float4(0.0, 0.0, 1.0, 1.0);
    float4 expectedB = float4(1.0, 1.0, 0.0, 0.0);
    _out.sk_FragColor = ((((((((((((((step(0.5, _uniforms.testInputs.x) == expectedA.x && all(step(0.5, _uniforms.testInputs.xy) == expectedA.xy)) && all(step(0.5, _uniforms.testInputs.xyz) == expectedA.xyz)) && all(step(0.5, _uniforms.testInputs) == expectedA)) && 0.0 == expectedA.x) && all(float2(0.0, 0.0) == expectedA.xy)) && all(float3(0.0, 0.0, 1.0) == expectedA.xyz)) && all(float4(0.0, 0.0, 1.0, 1.0) == expectedA)) && step(_uniforms.testInputs.x, 0.0) == expectedB.x) && all(step(_uniforms.testInputs.xy, float2(0.0, 1.0)) == expectedB.xy)) && all(step(_uniforms.testInputs.xyz, float3(0.0, 1.0, 0.0)) == expectedB.xyz)) && all(step(_uniforms.testInputs, constGreen) == expectedB)) && 1.0 == expectedB.x) && all(float2(1.0, 1.0) == expectedB.xy)) && all(float3(1.0, 1.0, 0.0) == expectedB.xyz)) && all(float4(1.0, 1.0, 0.0, 0.0) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
