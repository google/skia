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
    float4 _0_v = _uniforms.testInputs;
    float _1_x = _0_v.x;
    float _2_y = _0_v.y;
    float _3_z = _0_v.z;
    float _4_w = _0_v.w;
    float4 a = float4(_1_x, _2_y, _3_z, _4_w);

    _out.sk_FragColor = all(a == float4(-1.25, 0.0, 0.75, 2.25)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
