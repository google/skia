#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
    float4 c;
    float4 d;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float distance(float p0, float p1) {
    return abs(p0 - p1);
}




fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = distance(_in.a, _in.b);
    _out->sk_FragColor.x = distance(_in.c, _in.d);
    return *_out;
}
