#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorBlack;
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
    int4 _1_i = int4(_uniforms.colorBlack);
    float _2_x = _0_v[_1_i.x];
    float _3_y = _0_v[_1_i.y];
    float _4_z = _0_v[_1_i.z];
    float _5_w = _0_v[_1_i.w];
    _out.sk_FragColor = all(float4(_2_x, _3_y, _4_z, _5_w) == float4(-1.25, -1.25, -1.25, 0.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
