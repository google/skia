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
float _skOutParamHelper0_modf(float _var0, thread float& b) {
    float _var1;
    float _skResult = modf(_var0, _var1);
    b = _var1;
    return _skResult;
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x =     _skOutParamHelper0_modf(_in.a, _in.b);
    return *_out;
}
