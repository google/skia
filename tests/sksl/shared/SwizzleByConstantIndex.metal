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
    float4 _0_constant_swizzle;
    float4 _1_v = _uniforms.testInputs;
    float _2_x = _1_v.x;
    float _3_y = _1_v.y;
    float _4_z = _1_v.z;
    float _5_w = _1_v.w;
    float4 a = float4(_2_x, _3_y, _4_z, _5_w);

    float4 _6_foldable;
    float4 _7_v = float4(0.0, 1.0, 2.0, 3.0);
    float _8_x = _7_v.x;
    float _9_y = _7_v.y;
    float _10_z = _7_v.z;
    float _11_w = _7_v.w;
    float4 b = float4(_8_x, _9_y, _10_z, _11_w);

    _out.sk_FragColor = all(a == float4(-1.25, 0.0, 0.75, 2.25)) && all(b == float4(0.0, 1.0, 2.0, 3.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
