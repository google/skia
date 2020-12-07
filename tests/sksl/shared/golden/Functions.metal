#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void bar(thread float& x);
void _skOutParamHelper0_bar(thread float& x) {
    float _var0 = x;
    bar(_var0);
    x = _var0;
}
float foo(float v[2]) {
    return v[0] * v[1];
}
void bar(thread float& x) {
    float y[2];

    y[0] = x;
    y[1] = x * 2.0;
    foo(y);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float x = 10.0;
    _skOutParamHelper0_bar(x);
    _out->sk_FragColor = float4(x);
    return *_out;
}
