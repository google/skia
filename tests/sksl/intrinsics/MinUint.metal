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
    uint4 uintValues = uint4(abs(_uniforms.testInputs) * 100.0h);
    uint4 uintGreen = uint4(_uniforms.colorGreen * 100.0h);
    uint4 expectedA = uint4(50u, 0u, 50u, 50u);
    uint4 expectedB = uint4(0u, 0u, 0u, 100u);
    _out.sk_FragColor = ((((((((((((((min(uintValues.x, 50u) == expectedA.x && all(min(uintValues.xy, 50u) == expectedA.xy)) && all(min(uintValues.xyz, 50u) == expectedA.xyz)) && all(min(uintValues, 50u) == expectedA)) && 50u == expectedA.x) && all(uint2(50u, 0u) == expectedA.xy)) && all(uint3(50u, 0u, 50u) == expectedA.xyz)) && all(uint4(50u, 0u, 50u, 50u) == expectedA)) && min(uintValues.x, uintGreen.x) == expectedB.x) && all(min(uintValues.xy, uintGreen.xy) == expectedB.xy)) && all(min(uintValues.xyz, uintGreen.xyz) == expectedB.xyz)) && all(min(uintValues, uintGreen) == expectedB)) && 0u == expectedB.x) && all(uint2(0u) == expectedB.xy)) && all(uint3(0u) == expectedB.xyz)) && all(uint4(0u, 0u, 0u, 100u) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
