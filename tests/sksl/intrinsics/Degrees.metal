#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float4 testInputs;
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
    const float4 expected = float4(-71.61973, 0.0, 42.9718361, 128.915512);
    const float4 allowedDelta = float4(0.05);
    _out.sk_FragColor = ((abs(((_uniforms.testInputs.x) * 57.2957795) - -71.61973) < 0.05 && all((abs(((_uniforms.testInputs.xy) * 57.2957795) - float2(-71.61973, 0.0)) < float2(0.05)))) && all((abs(((_uniforms.testInputs.xyz) * 57.2957795) - float3(-71.61973, 0.0, 42.9718361)) < float3(0.05)))) && all((abs(((_uniforms.testInputs) * 57.2957795) - expected) < allowedDelta)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
