#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float4 colorBlack;
    float4 colorWhite;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 expectedBW = float4(0.5, 0.5, 0.5, 1.0);
    float4 expectedWT = float4(1.0, 0.5, 1.0, 2.25);
    _out.sk_FragColor = ((((((((((all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.0) == float4(0.0, 1.0, 0.0, 1.0)) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.25) == float4(0.25, 0.75, 0.0, 1.0))) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.75) == float4(0.75, 0.25, 0.0, 1.0))) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 1.0) == float4(1.0, 0.0, 0.0, 1.0))) && mix(_uniforms.colorBlack.x, _uniforms.colorWhite.x, 0.5) == expectedBW.x) && all(mix(_uniforms.colorBlack.xy, _uniforms.colorWhite.xy, 0.5) == expectedBW.xy)) && all(mix(_uniforms.colorBlack.xyz, _uniforms.colorWhite.xyz, 0.5) == expectedBW.xyz)) && all(mix(_uniforms.colorBlack, _uniforms.colorWhite, 0.5) == expectedBW)) && mix(_uniforms.colorWhite.x, _uniforms.testInputs.x, 0.0) == expectedWT.x) && all(mix(_uniforms.colorWhite.xy, _uniforms.testInputs.xy, float2(0.0, 0.5)) == expectedWT.xy)) && all(mix(_uniforms.colorWhite.xyz, _uniforms.testInputs.xyz, float3(0.0, 0.5, 0.0)) == expectedWT.xyz)) && all(mix(_uniforms.colorWhite, _uniforms.testInputs, float4(0.0, 0.5, 0.0, 1.0)) == expectedWT) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
