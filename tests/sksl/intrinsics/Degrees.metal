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
    _out.sk_FragColor = ((((((((_uniforms.input.x) * 57.2957795) == _uniforms.expected.x && all(((_uniforms.input.xy) * 57.2957795) == _uniforms.expected.xy)) && all(((_uniforms.input.xyz) * 57.2957795) == _uniforms.expected.xyz)) && all(((_uniforms.input) * 57.2957795) == _uniforms.expected)) && 90.0 == _uniforms.expected.x) && all(float2(90.0, 180.0) == _uniforms.expected.xy)) && all(float3(90.0, 180.0, 270.0) == _uniforms.expected.xyz)) && all(float4(90.0, 180.0, 270.0, 360.0) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
