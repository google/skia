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
    float4 expectedA = float4(0.5, 0.5, 0.75, 2.25);
    float4 expectedB = float4(0.0, 1.0, 0.75, 2.25);
    _out.sk_FragColor = ((((((((((((((max(_uniforms.testInputs.x, 0.5) == expectedA.x && all(max(_uniforms.testInputs.xy, 0.5) == expectedA.xy)) && all(max(_uniforms.testInputs.xyz, 0.5) == expectedA.xyz)) && all(max(_uniforms.testInputs, 0.5) == expectedA)) && 0.5 == expectedA.x) && all(float2(0.5, 0.5) == expectedA.xy)) && all(float3(0.5, 0.5, 0.75) == expectedA.xyz)) && all(float4(0.5, 0.5, 0.75, 2.25) == expectedA)) && max(_uniforms.testInputs.x, _uniforms.colorGreen.x) == expectedB.x) && all(max(_uniforms.testInputs.xy, _uniforms.colorGreen.xy) == expectedB.xy)) && all(max(_uniforms.testInputs.xyz, _uniforms.colorGreen.xyz) == expectedB.xyz)) && all(max(_uniforms.testInputs, _uniforms.colorGreen) == expectedB)) && 0.0 == expectedB.x) && all(float2(0.0, 1.0) == expectedB.xy)) && all(float3(0.0, 1.0, 0.75) == expectedB.xyz)) && all(float4(0.0, 1.0, 0.75, 2.25) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
