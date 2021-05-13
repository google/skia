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
    _out.sk_FragColor = ((((((((_uniforms.input.x) * 0.0174532925) == _uniforms.expected.x && all(((_uniforms.input.xy) * 0.0174532925) == _uniforms.expected.xy)) && all(((_uniforms.input.xyz) * 0.0174532925) == _uniforms.expected.xyz)) && all(((_uniforms.input) * 0.0174532925) == _uniforms.expected)) && 0.0 == _uniforms.expected.x) && all(float2(0.0, 0.0) == _uniforms.expected.xy)) && all(float3(0.0, 0.0, 0.0) == _uniforms.expected.xyz)) && all(float4(0.0, 0.0, 0.0, 0.0) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
