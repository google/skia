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
    float _skTemp0;
    float _skTemp1;
    _out->sk_FragColor.x = (_skTemp0 = _in.a, _skTemp1 = _in.b, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1));
    return *_out;
}
