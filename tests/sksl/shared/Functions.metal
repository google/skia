#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void bar(thread float& x);
void _skOutParamHelper0_bar(thread float& x) {
    float _var0 = x;
    bar(_var0);
    x = _var0;
}

float foo(float2 v) {
    return v.x * v.y;
}
void bar(thread float& x) {
    array<float, 2> y;
    y[0] = x;
    y[1] = x * 2.0;
    x = foo(float2(y[0], y[1]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = 10.0;
    _skOutParamHelper0_bar(x);
    _out.sk_FragColor = x == 200.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
