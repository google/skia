#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float2 v;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void initialize_vS(thread array<S, 2>& z);
void _skOutParamHelper0_initialize_vS(thread array<S, 2>& z) {
    array<S, 2> _var0;
    initialize_vS(_var0);
    z = _var0;
}
void initialize_vS(thread array<S, 2>& z) {
    z[0].v = float2(0.0, 1.0);
    z[1].v = float2(2.0, 1.0);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float2, 2> x;
    x[0] = float2(0.0, 0.0);
    x[1] = float2(1.0, 0.0);
    array<float2, 2> y;
    y[0] = float2(0.0, 1.0);
    y[1] = float2(-1.0, 2.0);
    array<S, 2> z;
    _skOutParamHelper0_initialize_vS(z);
    _out.sk_FragColor = half4(half(x[0].x * x[0].y + z[0].v.x), half(x[1].x - x[1].y * z[0].v.y), half((y[0].x / y[0].y) / z[1].v.x), half(y[1].x + y[1].y * z[1].v.y));
    return _out;
}
