#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float4 testMatrix2x2;
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
    float4 inputVal = _uniforms.testMatrix2x2 + float4(2.0, -2.0, 1.0, 8.0);
    float4 expected = float4(3.0, 3.0, 5.0, 13.0);
    const float allowedDelta = 0.05;
    _out.sk_FragColor = ((((((abs(abs(inputVal.x) - expected.x) < allowedDelta && abs(length(inputVal.xy) - expected.y) < allowedDelta) && abs(length(inputVal.xyz) - expected.z) < allowedDelta) && abs(length(inputVal) - expected.w) < allowedDelta) && abs(3.0 - expected.x) < allowedDelta) && abs(3.0 - expected.y) < allowedDelta) && abs(5.0 - expected.z) < allowedDelta) && abs(13.0 - expected.w) < allowedDelta ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
