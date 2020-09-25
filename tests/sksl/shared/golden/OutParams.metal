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
    _out->sk_FragColor = float4(1.0, 2.0, 3.0, 4.0);
    _out->sk_FragColor = float4(float2x2(2.0)[0][0], float3x3(3.0)[0][0], float4x4(4.0)[0][0], 1.0);
    _out->sk_FragColor = float4(1.0, 2.0, 3.0, 4.0);
    _out->sk_FragColor = float4(1.0, 2.0, 3.0, 4.0);
    _out->sk_FragColor = float4(float2x2(2.0)[0][0], float3x3(3.0)[0][0], float4x4(4.0)[0][0], 1.0);
    _out->sk_FragColor = float4(1.0, bool2(false).x ? 1.0 : 0.0, bool3(true).x ? 1.0 : 0.0, bool4(false).x ? 1.0 : 0.0);
    return *_out;
}
