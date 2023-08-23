#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float4 testInputs;
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
    const float4 expected = float4(0.75, 0.0, 0.75, 0.25);
    _out.sk_FragColor = ((fract(_uniforms.testInputs.x) == 0.75 && all(fract(_uniforms.testInputs.xy) == float2(0.75, 0.0))) && all(fract(_uniforms.testInputs.xyz) == float3(0.75, 0.0, 0.75))) && all(fract(_uniforms.testInputs) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
