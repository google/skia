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
    const half4 constGreen = half4(0.0h, 1.0h, 0.0h, 1.0h);
    half4 expectedA = half4(0.0h, 0.0h, 1.0h, 1.0h);
    half4 expectedB = half4(1.0h, 1.0h, 0.0h, 0.0h);
    half4 expectedC = half4(0.0h, 1.0h, 1.0h, 1.0h);
    _out.sk_FragColor = ((((((((((((((((((((((step(0.5h, _uniforms.testInputs.x) == expectedA.x && all(step(0.5h, _uniforms.testInputs.xy) == expectedA.xy)) && all(step(0.5h, _uniforms.testInputs.xyz) == expectedA.xyz)) && all(step(0.5h, _uniforms.testInputs) == expectedA)) && 0.0h == expectedA.x) && all(half2(0.0h) == expectedA.xy)) && all(half3(0.0h, 0.0h, 1.0h) == expectedA.xyz)) && all(half4(0.0h, 0.0h, 1.0h, 1.0h) == expectedA)) && step(_uniforms.testInputs.x, 0.0h) == expectedB.x) && all(step(_uniforms.testInputs.xy, half2(0.0h, 1.0h)) == expectedB.xy)) && all(step(_uniforms.testInputs.xyz, half3(0.0h, 1.0h, 0.0h)) == expectedB.xyz)) && all(step(_uniforms.testInputs, constGreen) == expectedB)) && 1.0h == expectedB.x) && all(half2(1.0h) == expectedB.xy)) && all(half3(1.0h, 1.0h, 0.0h) == expectedB.xyz)) && all(half4(1.0h, 1.0h, 0.0h, 0.0h) == expectedB)) && step(_uniforms.colorRed.x, _uniforms.colorGreen.x) == expectedC.x) && all(step(_uniforms.colorRed.xy, _uniforms.colorGreen.xy) == expectedC.xy)) && all(step(_uniforms.colorRed.xyz, _uniforms.colorGreen.xyz) == expectedC.xyz)) && all(step(_uniforms.colorRed, _uniforms.colorGreen) == expectedC)) && 0.0h == expectedC.x) && all(half2(0.0h, 1.0h) == expectedC.xy)) && all(half3(0.0h, 1.0h, 1.0h) == expectedC.xyz)) && all(half4(0.0h, 1.0h, 1.0h, 1.0h) == expectedC) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
