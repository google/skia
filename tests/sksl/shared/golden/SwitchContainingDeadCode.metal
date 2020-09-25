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
    switch (int(sqrt(2.0))) {
        case 0:
            ;
        case 1:
            ;
        default:
            ;
    }
    _out->sk_FragColor = float4(2.0);
    return *_out;
}
