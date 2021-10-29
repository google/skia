#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float4x3 float4x3_from_float4x4(float4x4 x0) {
    return float4x3(float3(x0[0].xyz), float3(x0[1].xyz), float3(x0[2].xyz), float3(x0[3].xyz));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4x3 a = float4x3(1.0);
    float4x3 b = float4x3_from_float4x4(float4x4(1.0));
    _out.sk_FragColor.x = float(all(a[0] == b[0]) ? 0 : 1);
    return _out;
}
