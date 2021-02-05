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
    _out.sk_FragColor = ((fract(_uniforms.testInputs.x) == 0.75 && all(fract(_uniforms.testInputs.xy) == float2(0.75, 0.0))) && all(fract(_uniforms.testInputs.xyz) == float3(0.75, 0.0, 0.75))) && all(fract(_uniforms.testInputs) == float4(0.75, 0.0, 0.75, 0.25)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
