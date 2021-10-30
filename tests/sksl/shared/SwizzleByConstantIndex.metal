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
    half4 _0_v = _uniforms.testInputs;
    half _1_x = _0_v.x;
    half _2_y = _0_v.y;
    half _3_z = _0_v.z;
    half _4_w = _0_v.w;
    half4 a = half4(_1_x, _2_y, _3_z, _4_w);
    half _9_x = _uniforms.testInputs.x;
    half _10_y = _uniforms.testInputs.y;
    half _11_z = _uniforms.testInputs.z;
    half _12_w = _uniforms.testInputs.w;
    half4 b = half4(_9_x, _10_y, _11_z, _12_w);
    half4 _13_v = half4(0.0h, 1.0h, 2.0h, 3.0h);
    half _14_x = _13_v.x;
    half _15_y = _13_v.y;
    half _16_z = _13_v.z;
    half _17_w = _13_v.w;
    half4 c = half4(_14_x, _15_y, _16_z, _17_w);
    _out.sk_FragColor = (all(a == half4(-1.25h, 0.0h, 0.75h, 2.25h)) && all(b == half4(-1.25h, 0.0h, 0.75h, 2.25h))) && all(c == half4(0.0h, 1.0h, 2.0h, 3.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
