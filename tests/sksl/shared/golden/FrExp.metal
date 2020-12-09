#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _skOutParamHelper0_frexp(float _var0, thread int& exp1) {
    int _var1;
    float _skResult = frexp(_var0, _var1);
    exp1 = _var1;
    return _skResult;
}
float3 _skOutParamHelper1_frexp(float3 _var0, thread int3& exp3) {
    int3 _var1;
    float3 _skResult = frexp(_var0, _var1);
    exp3 = _var1;
    return _skResult;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    int exp1;
    float a =     _skOutParamHelper0_frexp(0.5, exp1);
    _out->sk_FragColor = float4(float(exp1));
    int3 exp3;
    _out->sk_FragColor.xyz =     _skOutParamHelper1_frexp(float3(3.5), exp3);
    return *_out;
}
