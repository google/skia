#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _0_x = 1.0;
    _0_x = abs(_0_x);
    _0_x = abs(_0_x - 2.0);
    _0_x = (_0_x * 2.0);
    _0_x = sign(_0_x);
    float2 _1_x = float2(1.0, 2.0);
    _1_x = float2(length(_1_x));
    _1_x = float2(distance(_1_x, float2(3.0, 4.0)));
    _1_x = float2(dot(_1_x, float2(3.0, 4.0)));
    _1_x = normalize(_1_x);
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
