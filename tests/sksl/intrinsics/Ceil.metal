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
    _out.sk_FragColor = ((ceil(_uniforms.testInputs.x) == float4(-1.0, 0.0, 1.0, 3.0).x && all(ceil(_uniforms.testInputs.xy) == float4(-1.0, 0.0, 1.0, 3.0).xy)) && all(ceil(_uniforms.testInputs.xyz) == float4(-1.0, 0.0, 1.0, 3.0).xyz)) && all(ceil(_uniforms.testInputs) == float4(-1.0, 0.0, 1.0, 3.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
