#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint a;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.xy = unpackHalf2x16(_in.a);
    _out->sk_FragColor.xy = unpackUnorm2x16(_in.a);
    _out->sk_FragColor.xy = unpackSnorm2x16(_in.a);
    _out->sk_FragColor = unpackUnorm4x8(_in.a);
    _out->sk_FragColor = unpackSnorm4x8(_in.a);
    return *_out;
}
