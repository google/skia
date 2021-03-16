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
    float2x2 m1 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _out.sk_FragColor = m1[0].xyxy;
    return _out;
}
