#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct sksl_synthetic_uniforms {
    float u_skRTHeight;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant sksl_synthetic_uniforms& _anonInterface0 [[buffer(1)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float4(_fragCoord.x, _anonInterface0.u_skRTHeight - _fragCoord.y, 0.0, _fragCoord.w).y / _globals.sk_Height;
    return _out;
}
