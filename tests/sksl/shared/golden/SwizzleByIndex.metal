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
    float4 v = float4(sqrt(1.0));
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float w = v[3];
    _out->sk_FragColor = float4(x, y, z, w);
    return *_out;
}
