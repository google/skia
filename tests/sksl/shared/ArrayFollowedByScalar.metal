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
    array<half, 3> rgb;
    half a;
    rgb[0] = 0.0h;
    rgb[1] = 1.0h;
    rgb[2] = 0.0h;
    a = 1.0h;
    _out.sk_FragColor = half4(rgb[0], rgb[1], rgb[2], a);
    return _out;
}
