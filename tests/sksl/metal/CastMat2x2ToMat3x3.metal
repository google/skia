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
float3x3 float3x3_from_float2x2(float2x2 x0) {
    return float3x3(float3(x0[0].xy, 0.0), float3(x0[1].xy, 0.0), float3(0.0, 0.0, 1.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float3x3 a = float3x3(1.0);
    float3x3 b = float3x3_from_float2x2(float2x2(1.0));
    _out.sk_FragColor.x = half(all(a[0] == b[0]) ? 0 : 1);
    return _out;
}
