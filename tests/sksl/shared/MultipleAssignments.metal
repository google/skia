#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x;
    float y;
    x = (y = 1.0);
    half a;
    half b;
    half c;
    a = (b = (c = 0.0h));
    _out.sk_FragColor = half4(a * b, half(x), c, half(y));
    return _out;
}
