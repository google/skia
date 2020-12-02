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
    float array[4];
    array[0] = 0.0;
    array[1] = 1.0;
    array[2] = 2.0;
    array[3] = 3.0;
    _out->sk_FragColor = float4(array[0], array[1], array[2], array[3u]);
    return *_out;
}
