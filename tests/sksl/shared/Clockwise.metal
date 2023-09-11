#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct sksl_synthetic_uniforms {
    float2 u_skRTFlip;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(half((_anonInterface0.u_skRTFlip.y < 0 ? _frontFacing : !_frontFacing) ? 1 : -1));
    return _out;
}
