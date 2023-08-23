#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 testInputs;
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
    half4 expectedA = half4(-1.0h, 0.0h, 0.75h, 1.0h);
    const half4 clampLow = half4(-1.0h, -2.0h, -2.0h, 1.0h);
    half4 expectedB = half4(-1.0h, 0.0h, 0.5h, 2.25h);
    const half4 clampHigh = half4(1.0h, 2.0h, 0.5h, 3.0h);
    _out.sk_FragColor = ((((((((((((((clamp(_uniforms.testInputs.x, -1.0h, 1.0h) == expectedA.x && all(clamp(_uniforms.testInputs.xy, -1.0h, 1.0h) == expectedA.xy)) && all(clamp(_uniforms.testInputs.xyz, -1.0h, 1.0h) == expectedA.xyz)) && all(clamp(_uniforms.testInputs, -1.0h, 1.0h) == expectedA)) && clamp(_uniforms.testInputs.x, -1.0h, 1.0h) == expectedB.x) && all(clamp(_uniforms.testInputs.xy, half2(-1.0h, -2.0h), half2(1.0h, 2.0h)) == expectedB.xy)) && all(clamp(_uniforms.testInputs.xyz, half3(-1.0h, -2.0h, -2.0h), half3(1.0h, 2.0h, 0.5h)) == expectedB.xyz)) && all(clamp(_uniforms.testInputs, clampLow, clampHigh) == expectedB)) && -1.0h == expectedA.x) && all(half2(-1.0h, 0.0h) == expectedA.xy)) && all(half3(-1.0h, 0.0h, 0.75h) == expectedA.xyz)) && all(half4(-1.0h, 0.0h, 0.75h, 1.0h) == expectedA)) && -1.0h == expectedB.x) && all(half2(-1.0h, 0.0h) == expectedB.xy)) && all(half3(-1.0h, 0.0h, 0.5h) == expectedB.xyz)) && all(half4(-1.0h, 0.0h, 0.5h, 2.25h) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
