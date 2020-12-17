#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float2 a;
    float2 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = _in.a.x * _in.b.y - _in.a.y * _in.b.x;

    return *_out;
}
