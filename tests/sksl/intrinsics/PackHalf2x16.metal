#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    uint xy = as_type<uint>(half2(_uniforms.testInputs.xy));
    uint zw = as_type<uint>(half2(_uniforms.testInputs.zw));
    _out.sk_FragColor = all(float2(as_type<half2>(xy)) == float2(-1.25, 0.0)) && all(float2(as_type<half2>(zw)) == float2(0.75, 2.25)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
