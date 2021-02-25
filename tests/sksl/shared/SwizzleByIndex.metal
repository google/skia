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
    float4 _0_non_constant_swizzle;
    float4 _1_v = _uniforms.testInputs;
    int4 _2_i = int4(_uniforms.colorBlack);
    float _3_x = _1_v[_2_i.x];
    float _4_y = _1_v[_2_i.y];
    float _5_z = _1_v[_2_i.z];
    float _6_w = _1_v[_2_i.w];
    _out.sk_FragColor = all(float4(_3_x, _4_y, _5_z, _6_w) == float4(-1.25, -1.25, -1.25, 0.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;

}
