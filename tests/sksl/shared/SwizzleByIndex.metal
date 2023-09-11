#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorBlack;
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
    half4 _0_v = _uniforms.testInputs;
    int4 _1_i = int4(_uniforms.colorBlack);
    half _2_x = _0_v[_1_i.x];
    half _3_y = _0_v[_1_i.y];
    half _4_z = _0_v[_1_i.z];
    half _5_w = _0_v[_1_i.w];
    _out.sk_FragColor = all(half4(_2_x, _3_y, _4_z, _5_w) == half4(-1.25h, -1.25h, -1.25h, 0.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
