#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
    float4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};




fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 vector2 = 2.0 * _uniforms.colorWhite;
    _out.sk_FragColor = ((((((smoothstep(0.0, 2.0, _uniforms.testInputs.x) == 0.0 && all(smoothstep(0.0, 2.0, _uniforms.testInputs.xy) == float2(0.0, 0.0))) && all(smoothstep(0.0, 2.0, _uniforms.testInputs.xyz) == float3(0.0, 0.0, 0.31640625))) && all(smoothstep(0.0, 2.0, _uniforms.testInputs) == float4(0.0, 0.0, 0.31640625, 1.0))) && smoothstep(0.0, vector2.x, _uniforms.testInputs.x) == 0.0) && all(smoothstep(float2(0.0), vector2.xy, _uniforms.testInputs.xy) == float2(0.0, 0.0))) && all(smoothstep(float3(0.0), vector2.xyz, _uniforms.testInputs.xyz) == float3(0.0, 0.0, 0.31640625))) && all(smoothstep(float4(0.0), vector2, _uniforms.testInputs) == float4(0.0, 0.0, 0.31640625, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
