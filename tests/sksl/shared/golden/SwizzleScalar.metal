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
    float x = sqrt(4.0);
    _out->sk_FragColor = float4(float2(x), 0.0, 1.0);
    _out->sk_FragColor = float4(float2(sqrt(4.0)), 0.0, 1.0);
    _out->sk_FragColor = float4(0.0, sqrt(4.0), 0.0, 1.0);
    _out->sk_FragColor = float3(float2(sqrt(4.0)), 0.0).zxzy;
    return *_out;
}
