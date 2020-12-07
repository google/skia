#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct testBlock {
    float x;
    float y[2];
    float3x2 z;
    bool w;
};
struct Globals {
    constant testBlock* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& _anonInterface0 [[buffer(789)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{&_anonInterface0};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_globals->_anonInterface0->x, _globals->_anonInterface0->y[0], _globals->_anonInterface0->y[1], 0.0);
    return *_out;
}
