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
    int4 intValues = int4(_uniforms.testInputs * 100.0h);
    int4 expectedA = int4(-100, 0, 75, 100);
    const int4 clampLow = int4(-100, -200, -200, 100);
    int4 expectedB = int4(-100, 0, 50, 225);
    const int4 clampHigh = int4(100, 200, 50, 300);
    _out.sk_FragColor = ((((((((((((((clamp(intValues.x, -100, 100) == expectedA.x && all(clamp(intValues.xy, -100, 100) == expectedA.xy)) && all(clamp(intValues.xyz, -100, 100) == expectedA.xyz)) && all(clamp(intValues, -100, 100) == expectedA)) && -100 == expectedA.x) && all(int2(-100, 0) == expectedA.xy)) && all(int3(-100, 0, 75) == expectedA.xyz)) && all(int4(-100, 0, 75, 100) == expectedA)) && clamp(intValues.x, -100, 100) == expectedB.x) && all(clamp(intValues.xy, int2(-100, -200), int2(100, 200)) == expectedB.xy)) && all(clamp(intValues.xyz, int3(-100, -200, -200), int3(100, 200, 50)) == expectedB.xyz)) && all(clamp(intValues, clampLow, clampHigh) == expectedB)) && -100 == expectedB.x) && all(int2(-100, 0) == expectedB.xy)) && all(int3(-100, 0, 50) == expectedB.xyz)) && all(int4(-100, 0, 50, 225) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
