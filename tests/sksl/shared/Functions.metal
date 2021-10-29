#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void bar_vf(thread float& x);
void _skOutParamHelper0_bar_vf(thread float& x) {
    float _var0 = x;
    bar_vf(_var0);
    x = _var0;
}
float foo_ff2(const float2 v) {
    return v.x * v.y;
}
void bar_vf(thread float& x) {
    array<float, 2> y;
    y[0] = x;
    y[1] = x * 2.0;
    x = foo_ff2(float2(y[0], y[1]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = 10.0;
    _skOutParamHelper0_bar_vf(x);
    _out.sk_FragColor = x == 200.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
