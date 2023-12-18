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
    int x = 0;
    int y = 0;
    int z = 0;
    if (true) x = 1;
    if (false) y = 1;
    if (true) z = 1;
    _out.sk_FragColor.xyz = half3(half(x), half(y), half(z));
    return _out;
}
