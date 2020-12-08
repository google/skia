#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    int exp1;
    float a = frexp(0.5, exp1);
    _out->sk_FragColor = float4(float(exp1));
    int3 exp3;
    _out->sk_FragColor.xyz = frexp(float3(3.5), exp3);
    return *_out;
}
