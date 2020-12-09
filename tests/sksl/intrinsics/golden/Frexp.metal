#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    int b;
};
float _skOutParamHelper0_frexp(float _var0, thread int& b) {
    int _var1;
    float _skResult = frexp(_var0, _var1);
    b = _var1;
    return _skResult;
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{{}};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x =     _skOutParamHelper0_frexp(_in.a, _globals->b);
    return *_out;
}
