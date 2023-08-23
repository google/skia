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
    const half4 constVal = half4(-1.25h, 0.0h, 0.75h, 2.25h);
    half4 expectedA = half4(0.0h, 0.0h, 0.84375h, 1.0h);
    half4 expectedB = half4(1.0h, 0.0h, 1.0h, 1.0h);
    _out.sk_FragColor = ((((((((((((((((((0.0h == expectedA.x && all(half2(0.0h) == expectedA.xy)) && all(half3(0.0h, 0.0h, 0.84375h) == expectedA.xyz)) && all(half4(0.0h, 0.0h, 0.84375h, 1.0h) == expectedA)) && 0.0h == expectedA.x) && all(half2(0.0h) == expectedA.xy)) && all(half3(0.0h, 0.0h, 0.84375h) == expectedA.xyz)) && all(half4(0.0h, 0.0h, 0.84375h, 1.0h) == expectedA)) && smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, -1.25h) == expectedA.x) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, half2(-1.25h, 0.0h)) == expectedA.xy)) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, half3(-1.25h, 0.0h, 0.75h)) == expectedA.xyz)) && all(smoothstep(_uniforms.colorRed.y, _uniforms.colorGreen.y, constVal) == expectedA)) && 1.0h == expectedB.x) && all(half2(1.0h, 0.0h) == expectedB.xy)) && all(half3(1.0h, 0.0h, 1.0h) == expectedB.xyz)) && all(half4(1.0h, 0.0h, 1.0h, 1.0h) == expectedB)) && smoothstep(_uniforms.colorRed.x, _uniforms.colorGreen.x, -1.25h) == expectedB.x) && all(smoothstep(_uniforms.colorRed.xy, _uniforms.colorGreen.xy, half2(-1.25h, 0.0h)) == expectedB.xy)) && all(smoothstep(_uniforms.colorRed.xyz, _uniforms.colorGreen.xyz, half3(-1.25h, 0.0h, 0.75h)) == expectedB.xyz)) && all(smoothstep(_uniforms.colorRed, _uniforms.colorGreen, constVal) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
