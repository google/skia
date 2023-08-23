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
    half4 expectedA = half4(0.5h, 0.5h, 0.75h, 2.25h);
    half4 expectedB = half4(0.0h, 1.0h, 0.75h, 2.25h);
    _out.sk_FragColor = ((((((((((((((max(_uniforms.testInputs.x, 0.5h) == expectedA.x && all(max(_uniforms.testInputs.xy, 0.5h) == expectedA.xy)) && all(max(_uniforms.testInputs.xyz, 0.5h) == expectedA.xyz)) && all(max(_uniforms.testInputs, 0.5h) == expectedA)) && 0.5h == expectedA.x) && all(half2(0.5h) == expectedA.xy)) && all(half3(0.5h, 0.5h, 0.75h) == expectedA.xyz)) && all(half4(0.5h, 0.5h, 0.75h, 2.25h) == expectedA)) && max(_uniforms.testInputs.x, _uniforms.colorGreen.x) == expectedB.x) && all(max(_uniforms.testInputs.xy, _uniforms.colorGreen.xy) == expectedB.xy)) && all(max(_uniforms.testInputs.xyz, _uniforms.colorGreen.xyz) == expectedB.xyz)) && all(max(_uniforms.testInputs, _uniforms.colorGreen) == expectedB)) && 0.0h == expectedB.x) && all(half2(0.0h, 1.0h) == expectedB.xy)) && all(half3(0.0h, 1.0h, 0.75h) == expectedB.xyz)) && all(half4(0.0h, 1.0h, 0.75h, 2.25h) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
