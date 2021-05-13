#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 input;
    float4 expected;
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
    _out.sk_FragColor = ((((((fract(_uniforms.input.x) == _uniforms.expected.x && all(fract(_uniforms.input.xy) == _uniforms.expected.xy)) && all(fract(_uniforms.input.xyz) == _uniforms.expected.xyz)) && all(fract(_uniforms.input) == _uniforms.expected)) && 0.0 == _uniforms.expected.x) && all(float2(0.0, 0.25) == _uniforms.expected.xy)) && all(float3(0.0, 0.25, 0.5) == _uniforms.expected.xyz)) && all(float4(0.0, 0.25, 0.5, 0.75) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
