#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputVal;
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
    _out.sk_FragColor = ((((((cosh(_uniforms.inputVal.x) == _uniforms.expected.x && all(cosh(_uniforms.inputVal.xy) == _uniforms.expected.xy)) && all(cosh(_uniforms.inputVal.xyz) == _uniforms.expected.xyz)) && all(cosh(_uniforms.inputVal) == _uniforms.expected)) && 1.0 == _uniforms.expected.x) && all(float2(1.0, 1.0) == _uniforms.expected.xy)) && all(float3(1.0, 1.0, 1.0) == _uniforms.expected.xyz)) && all(float4(1.0, 1.0, 1.0, 1.0) == _uniforms.expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
