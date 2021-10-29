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
    float x;
    float y;
    x = (y = 1.0);
    float a;
    float b;
    float c;
    a = (b = (c = 0.0));
    _out.sk_FragColor = float4(a * b, x, c, y);
    return _out;
}
