#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float4 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float normalize(float x) {
    return sign(x);
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = normalize(_in.a);
    _out->sk_FragColor = normalize(_in.b);
    return *_out;
}
