#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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
struct sksl_synthetic_uniforms {
    float2 u_skRTFlip;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 expected = half4(0.0h);
    _out.sk_FragColor = ((((((_anonInterface0.u_skRTFlip.y * dfdy(_uniforms.testInputs.x)) == expected.x && all((_anonInterface0.u_skRTFlip.y * dfdy(_uniforms.testInputs.xy)) == expected.xy)) && all((_anonInterface0.u_skRTFlip.y * dfdy(_uniforms.testInputs.xyz)) == expected.xyz)) && all((_anonInterface0.u_skRTFlip.y * dfdy(_uniforms.testInputs)) == expected)) && all(sign((_anonInterface0.u_skRTFlip.y * dfdy(coords.xx))) == float2(0.0))) && all(sign((_anonInterface0.u_skRTFlip.y * dfdy(coords.yy))) == float2(1.0))) && all(sign((_anonInterface0.u_skRTFlip.y * dfdy(coords))) == float2(0.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
