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
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float3x4 z = float2x4(1.0) * float3x2(float2(1.0, 0.0), float2(0.0, 1.0), float2(2.0, 2.0));
    float3 v1 = float3x3(1.0) * float3(2.0);
    float3 v2 = float3(2.0) * float3x3(1.0);
    _out->sk_FragColor = float4(z[0].x, v1 + v2);
    float2x2 m5 = float2x2(float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x);
    _out->sk_FragColor = float4((((((((((float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x + float2x2_from_float4(float4(0.0))[0].x) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x) + float2x2(1.0)[0].x) + m5[0].x) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0].x) + float2x2(float2(5.0, 6.0), float2(7.0, 8.0))[0].x) + float3x2(float2(1.0, 2.0), float2(3.0, 4.0), float2(5.0, 6.0))[0].x) + float3x3(1.0)[0].x) + float4x4(1.0)[0].x) + float4x4(2.0)[0].x);
    return *_out;
}
