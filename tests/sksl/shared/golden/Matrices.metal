#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float3x4 z = float2x4(1.0) * float3x2(float2(1.0, 0.0), float2(0.0, 1.0), float2(2.0, 2.0));
    float3 v1 = float3x3(1.0) * float3(2.0);
    float3 v2 = float3(2.0) * float3x3(1.0);
    _out->sk_FragColor = float4(z[0].x, v1 + v2);
    return *_out;
}
