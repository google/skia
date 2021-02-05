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
    float4 _1_v = _uniforms.testInputs;
    float _2_x = _1_v.x;
    float _3_y = _1_v.y;
    float _4_z = _1_v.z;
    float _5_w = _1_v.w;
    float4 a = float4(_2_x, _3_y, _4_z, _5_w);

    _out.sk_FragColor = all(a == float4(-1.25, 0.0, 0.75, 2.25)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
