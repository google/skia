#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _skOutParamHelper0_frexp(float var0, thread int& exp) {
    int var1 = exp;
    float _skResult = frexp(var0, var1);
    exp = var1;
    return _skResult;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    int exp;
    float foo =     _skOutParamHelper0_frexp(0.5, exp);
    _out->sk_FragColor = float4(float(exp));
    return *_out;
}
