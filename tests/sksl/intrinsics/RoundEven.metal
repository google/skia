#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float4 expectedA = float4(-1.0, 0.0, 1.0, 2.0);
    _out.sk_FragColor = ((float(rint(_uniforms.testInputs.x)) == -1.0 && all(float2(rint(_uniforms.testInputs.xy)) == float2(-1.0, 0.0))) && all(float3(rint(_uniforms.testInputs.xyz)) == float3(-1.0, 0.0, 1.0))) && all(float4(rint(_uniforms.testInputs)) == expectedA) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
