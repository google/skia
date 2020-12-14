#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    int a;
    uint b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    int _skTemp0;
    uint _skTemp1;
    _out->sk_FragColor.x = float((_skTemp0 = (_in.a), select(ctz(_skTemp0), int(-1), _skTemp0 == int(0))));
    _out->sk_FragColor.y = float((_skTemp1 = (_in.b), select(ctz(_skTemp1), uint(-1), _skTemp1 == uint(0))));
    return *_out;
}
