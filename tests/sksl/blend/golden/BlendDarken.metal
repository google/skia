#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _1_result = _in.src + (1.0 - _in.src.w) * _in.dst;

    _1_result.xyz = min(_1_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
    _out->sk_FragColor = _1_result;

    return *_out;
}
