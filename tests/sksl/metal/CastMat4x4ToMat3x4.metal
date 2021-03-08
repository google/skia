#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float3x4 float3x4_from_float4x4(float4x4 x0) {
    return float3x4(float4(x0[0].xyzw), float4(x0[1].xyzw), float4(x0[2].xyzw));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float3x4 a = float3x4(1.0);
    float3x4 b = float3x4_from_float4x4(float4x4(1.0));
    _out.sk_FragColor.x = float(all(a[0] == b[0]) ? 0 : 1);
    return _out;
}
