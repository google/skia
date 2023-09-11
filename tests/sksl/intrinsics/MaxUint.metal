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
    uint4 expectedA = uint4(125u, 80u, 80u, 225u);
    uint4 expectedB = uint4(125u, 100u, 75u, 225u);
    _out.sk_FragColor = ((((((((((((((max(uintValues.x, 80u) == expectedA.x && all(max(uintValues.xy, 80u) == expectedA.xy)) && all(max(uintValues.xyz, 80u) == expectedA.xyz)) && all(max(uintValues, 80u) == expectedA)) && 125u == expectedA.x) && all(uint2(125u, 80u) == expectedA.xy)) && all(uint3(125u, 80u, 80u) == expectedA.xyz)) && all(uint4(125u, 80u, 80u, 225u) == expectedA)) && max(uintValues.x, uintGreen.x) == expectedB.x) && all(max(uintValues.xy, uintGreen.xy) == expectedB.xy)) && all(max(uintValues.xyz, uintGreen.xyz) == expectedB.xyz)) && all(max(uintValues, uintGreen) == expectedB)) && 125u == expectedB.x) && all(uint2(125u, 100u) == expectedB.xy)) && all(uint3(125u, 100u, 75u) == expectedB.xyz)) && all(uint4(125u, 100u, 75u, 225u) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
