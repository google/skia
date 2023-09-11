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

half4 half4_from_half2x2(half2x2 x) {
    return half4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half2x2 x = half2x2(half2(0.0h, 1.0h), half2(2.0h, 3.0h));
    float2 y = float2(half4_from_half2x2(x).xy);
    _out.sk_FragColor = half4(float4(y, 0.0, 1.0));
    return _out;
}
