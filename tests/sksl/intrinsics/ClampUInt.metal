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
    uint4 uintValues = uint4(_uniforms.testInputs * 100.0h + 200.0h);
    uint4 expectedA = uint4(100u, 200u, 275u, 300u);
    const uint4 clampLow = uint4(100u, 0u, 0u, 300u);
    uint4 expectedB = uint4(100u, 200u, 250u, 425u);
    const uint4 clampHigh = uint4(300u, 400u, 250u, 500u);
    _out.sk_FragColor = ((((((((((((((clamp(uintValues.x, 100u, 300u) == expectedA.x && all(clamp(uintValues.xy, 100u, 300u) == expectedA.xy)) && all(clamp(uintValues.xyz, 100u, 300u) == expectedA.xyz)) && all(clamp(uintValues, 100u, 300u) == expectedA)) && 100u == expectedA.x) && all(uint2(100u, 200u) == expectedA.xy)) && all(uint3(100u, 200u, 275u) == expectedA.xyz)) && all(uint4(100u, 200u, 275u, 300u) == expectedA)) && clamp(uintValues.x, 100u, 300u) == expectedB.x) && all(clamp(uintValues.xy, uint2(100u, 0u), uint2(300u, 400u)) == expectedB.xy)) && all(clamp(uintValues.xyz, uint3(100u, 0u, 0u), uint3(300u, 400u, 250u)) == expectedB.xyz)) && all(clamp(uintValues, clampLow, clampHigh) == expectedB)) && 100u == expectedB.x) && all(uint2(100u, 200u) == expectedB.xy)) && all(uint3(100u, 200u, 250u) == expectedB.xyz)) && all(uint4(100u, 200u, 250u, 425u) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
