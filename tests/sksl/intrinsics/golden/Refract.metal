#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
    float c;
    float4 d;
    float4 e;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};





fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = refract(_in.a, _in.b, _in.c);
    _out->sk_FragColor = refract(_in.d, _in.e, _in.c);
    return *_out;
}
