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
    const float4 expected = float4(-0.021816615, 0.0, 0.01308997, 0.03926991);
    const float4 allowedDelta = float4(0.0005);
    _out.sk_FragColor = ((abs(((_uniforms.testInputs.x) * 0.0174532925) - -0.021816615) < 0.0005 && all((abs(((_uniforms.testInputs.xy) * 0.0174532925) - float2(-0.021816615, 0.0)) < float2(0.0005)))) && all((abs(((_uniforms.testInputs.xyz) * 0.0174532925) - float3(-0.021816615, 0.0, 0.01308997)) < float3(0.0005)))) && all((abs(((_uniforms.testInputs) * 0.0174532925) - expected) < allowedDelta)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
