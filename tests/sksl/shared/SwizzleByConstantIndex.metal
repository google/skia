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
    float _9_x = _uniforms.testInputs.x;
    float _10_y = _uniforms.testInputs.y;
    float _11_z = _uniforms.testInputs.z;
    float _12_w = _uniforms.testInputs.w;
    float4 b = float4(_9_x, _10_y, _11_z, _12_w);
    float4 _13_v = float4(0.0, 1.0, 2.0, 3.0);
    float _14_x = _13_v.x;
    float _15_y = _13_v.y;
    float _16_z = _13_v.z;
    float _17_w = _13_v.w;
    float4 c = float4(_14_x, _15_y, _16_z, _17_w);
    _out.sk_FragColor = (all(a == float4(-1.25, 0.0, 0.75, 2.25)) && all(b == float4(-1.25, 0.0, 0.75, 2.25))) && all(c == float4(0.0, 1.0, 2.0, 3.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
