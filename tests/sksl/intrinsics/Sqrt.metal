#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float2x2 testMatrix2x2;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 negativeVal = float4(-1.0, -4.0, -16.0, -64.0);
    coords = sqrt(negativeVal).xy;
    float4 inputVal = float4_from_float2x2(_uniforms.testMatrix2x2) + float4(0.0, 2.0, 6.0, 12.0);
    const float4 expected = float4(1.0, 2.0, 3.0, 4.0);
    const float4 allowedDelta = float4(0.05);
    _out.sk_FragColor = ((abs(sqrt(inputVal.x) - 1.0) < 0.05 && all((abs(sqrt(inputVal.xy) - float2(1.0, 2.0)) < float2(0.05)))) && all((abs(sqrt(inputVal.xyz) - float3(1.0, 2.0, 3.0)) < float3(0.05)))) && all((abs(sqrt(inputVal) - expected) < allowedDelta)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
