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
    float4 expected = float4(-2.0, 0.0, 0.0, 2.0);
    _out.sk_FragColor = ((((((floor(_uniforms.testInputs.x) == expected.x && all(floor(_uniforms.testInputs.xy) == expected.xy)) && all(floor(_uniforms.testInputs.xyz) == expected.xyz)) && all(floor(_uniforms.testInputs) == expected)) && -2.0 == expected.x) && all(float2(-2.0, 0.0) == expected.xy)) && all(float3(-2.0, 0.0, 0.0) == expected.xyz)) && all(float4(-2.0, 0.0, 0.0, 2.0) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
