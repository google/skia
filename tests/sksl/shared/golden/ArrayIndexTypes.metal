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
    _out->sk_FragColor = float4(float[4](1.0, 2.0, 3.0, 4.0)[0], float[4](1.0, 2.0, 3.0, 4.0)[1], float[4](1.0, 2.0, 3.0, 4.0)[2], float[4](1.0, 2.0, 3.0, 4.0)[3u]);
    return *_out;
}
