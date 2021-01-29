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
    _out.sk_FragColor = ((((((step(1.0, _uniforms.testInputs.x) == 0.0 && all(step(1.0, _uniforms.testInputs.xy) == float2(0.0, 0.0))) && all(step(1.0, _uniforms.testInputs.xyz) == float3(0.0, 0.0, 0.0))) && all(step(1.0, _uniforms.testInputs) == float4(0.0, 0.0, 0.0, 1.0))) && step(_uniforms.colorWhite.x, _uniforms.testInputs.x) == 0.0) && all(step(_uniforms.colorWhite.xy, _uniforms.testInputs.xy) == float2(0.0, 0.0))) && all(step(_uniforms.colorWhite.xyz, _uniforms.testInputs.xyz) == float3(0.0, 0.0, 0.0))) && all(step(_uniforms.colorWhite, _uniforms.testInputs) == float4(0.0, 0.0, 0.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
