#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float2, 2> x;
    x[0] = float2(0.0, 0.0);
    x[1] = float2(1.0, 0.0);
    array<float2, 2> y;
    y[0] = float2(0.0, 1.0);
    y[1] = float2(-1.0, 2.0);
    _out.sk_FragColor = float4(x[0].x * x[0].y, x[1].x - x[1].y, y[0].x / y[0].y, y[1].x + y[1].y);
    return _out;
}
