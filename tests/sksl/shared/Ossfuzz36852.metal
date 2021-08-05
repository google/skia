#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 x = float2x2(float2(0.0, 1.0), float2(2.0, 3.0));
    float2 y = float4_from_float2x2(x).xy;
    _out.sk_FragColor = y.xyxy;
    return _out;
}
