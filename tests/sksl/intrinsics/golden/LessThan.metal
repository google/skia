#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 a;
    float4 b;
    uint2 c;
    uint2 d;
    int3 e;
    int3 f;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};






fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float((_in.a < _in.b).x ? 1 : 0);
    _out->sk_FragColor.y = float((_in.c < _in.d).y ? 1 : 0);
    _out->sk_FragColor.z = float((_in.e < _in.f).z ? 1 : 0);
    return *_out;
}
