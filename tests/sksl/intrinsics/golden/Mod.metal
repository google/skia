#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float tmpX0, tmpY1;
    _out->sk_FragColor.x = (tmpX0 = _in.a, tmpY1 = _in.b, tmpX0 - tmpY1 * floor(tmpX0 / tmpY1));
    return *_out;
}
