#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 expectedA = float4(-1.25, 0.0, 0.5, 0.5);
    float4 expectedB = float4(-1.25, 0.0, 0.0, 1.0);
    _out.sk_FragColor = ((((((((((((((min(_uniforms.testInputs.x, 0.5) == expectedA.x && all(min(_uniforms.testInputs.xy, 0.5) == expectedA.xy)) && all(min(_uniforms.testInputs.xyz, 0.5) == expectedA.xyz)) && all(min(_uniforms.testInputs, 0.5) == expectedA)) && -1.25 == expectedA.x) && all(float2(-1.25, 0.0) == expectedA.xy)) && all(float3(-1.25, 0.0, 0.5) == expectedA.xyz)) && all(float4(-1.25, 0.0, 0.5, 0.5) == expectedA)) && min(_uniforms.testInputs.x, float(_uniforms.colorGreen.x)) == expectedB.x) && all(min(_uniforms.testInputs.xy, float2(_uniforms.colorGreen.xy)) == expectedB.xy)) && all(min(_uniforms.testInputs.xyz, float3(_uniforms.colorGreen.xyz)) == expectedB.xyz)) && all(min(_uniforms.testInputs, float4(_uniforms.colorGreen)) == expectedB)) && -1.25 == expectedB.x) && all(float2(-1.25, 0.0) == expectedB.xy)) && all(float3(-1.25, 0.0, 0.0) == expectedB.xyz)) && all(float4(-1.25, 0.0, 0.0, 1.0) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
