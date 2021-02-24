#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0[0], x0[1]), float2(x0[2], x0[3]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 m1 = float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0));
    _out.sk_FragColor = m1[0].xyxy;
    return _out;
}
