#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 a;
    float4 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    bool4 c;
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{{}};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(all(_in.a == _in.b == _globals->c) ? 1 : 0);
    return *_out;
}
