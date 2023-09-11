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
    half4 x = half4(1.0h);
    while (x.w == 1.0h) {
        x.x = x.x - 0.25h;
        if (x.x <= 0.0h) break;
    }
    while (x.z > 0.0h) {
        x.z = x.z - 0.25h;
        if (x.w == 1.0h) continue;
        x.y = 0.0h;
    }
    _out.sk_FragColor = x;
    return _out;
}
