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
    _out->sk_FragColor = float4(((1.0 - _in.src.w) * _in.dst.xyz + (1.0 - _in.dst.w) * _in.src.xyz) + _in.src.xyz * _in.dst.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);

    return *_out;
}
