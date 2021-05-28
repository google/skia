#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 expected = float4(-1.5625, 0.0, 0.75, 3.375);
    const float4 exponents = float4(2.0, 3.0, 1.0, 1.5);
    _out.sk_FragColor = ((((((pow(_uniforms.testInputs.x, 2.0) == expected.x && all(pow(_uniforms.testInputs.xy, float2(2.0, 3.0)) == expected.xy)) && all(pow(_uniforms.testInputs.xyz, float3(2.0, 3.0, 1.0)) == expected.xyz)) && all(pow(_uniforms.testInputs, exponents) == expected)) && 1.5625 == expected.x) && all(float2(1.5625, 0.0) == expected.xy)) && all(float3(1.5625, 0.0, 0.75) == expected.xyz)) && all(float4(1.5625, 0.0, 0.75, 3.375) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
