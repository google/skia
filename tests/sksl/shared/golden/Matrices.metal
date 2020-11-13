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
float2x2 float2x2_from_float_float3(float x0, float3 x1) {
    return float2x2(float2(x0, x1[0]), float2(x1[1], x1[2]));
}
float3x2 float3x2_from_float2_float_float3(float2 x0, float x1, float3 x2) {
    return float3x2(float2(x0[0], x0[1]), float2(x1, x2[0]), float2(x2[1], x2[2]));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float3x4 z = float2x4(1.0) * float3x2(float2(1.0, 0.0), float2(0.0, 1.0), float2(2.0, 2.0));
    float3 v1 = float3x3(1.0) * float3(2.0);
    float3 v2 = float3(2.0) * float3x3(1.0);
    _out->sk_FragColor = float4(z[0].x, v1 + v2);
    float2x2 m5 = float2x2(float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]);
    _out->sk_FragColor = float4((((((((((float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0] + float2x2_from_float4(float4(0.0))[0][0]) + float2x2_from_float4(float4(1.0, 2.0, 3.0, 4.0))[0][0]) + float2x2(1.0)[0][0]) + m5[0][0]) + float2x2(float2(1.0, 2.0), float2(3.0, 4.0))[0][0]) + float2x2_from_float_float3(5.0, float3(6.0, 7.0, 8.0))[0][0]) + float3x2_from_float2_float_float3(float2(1.0, 2.0), 3.0, float3(4.0, 5.0, 6.0))[0][0]) + float3x3(1.0)[0][0]) + float4x4(1.0)[0][0]) + float4x4(2.0)[0][0]);
    return *_out;
}
