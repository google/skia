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
    bool3 B;
    B.x = true;
    B.y = true;
    B.z = true;
    float3 F;
    F.x = 1.23;
    F.y = 0.0;
    F.z = 1.0;
    int3 I;
    I.x = 1;
    I.y = 1;
    I.z = 1;
    _out.sk_FragColor = half4(half((F.x * F.y) * F.z), half((B.x && B.y) && B.z), 0.0h, half((I.x * I.y) * I.z));
    return _out;
}
