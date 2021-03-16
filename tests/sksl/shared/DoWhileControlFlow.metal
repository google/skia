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
    float4 x = float4(1.0, 1.0, 1.0, 1.0);
    do {
        x.x = x.x - 0.25;
        if (x.x <= 0.0) break;
    } while (x.w == 1.0);
    do {
        x.z = x.z - 0.25;
        if (x.w == 1.0) continue;
        x.y = 0.0;
    } while (x.z > 0.0);
    _out.sk_FragColor = x;
    return _out;
}
