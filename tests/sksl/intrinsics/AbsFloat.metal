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
    float4 expected = float4(1.25, 0.0, 0.75, 2.25);
    _out.sk_FragColor = ((((((abs(_uniforms.testInputs.x) == expected.x && all(abs(_uniforms.testInputs.xy) == expected.xy)) && all(abs(_uniforms.testInputs.xyz) == expected.xyz)) && all(abs(_uniforms.testInputs) == expected)) && 1.25 == expected.x) && all(float2(1.25, 0.0) == expected.xy)) && all(float3(1.25, 0.0, 0.75) == expected.xyz)) && all(float4(1.25, 0.0, 0.75, 2.25) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
