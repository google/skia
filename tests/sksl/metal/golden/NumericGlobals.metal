#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float attr1;
    int attr2;
    float attr3;
    float4 attr4;
};




fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{{}, 123, {}, float4(4.0, 5.0, 6.0, 7.0)};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_globals->attr1, float(_globals->attr2), _globals->attr3, _globals->attr4.x);
    return *_out;
}
