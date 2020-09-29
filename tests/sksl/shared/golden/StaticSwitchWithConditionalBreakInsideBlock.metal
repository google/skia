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
    switch (0) {
        case 0:
            ;
            if (0.0 < sqrt(1.0)) {
                _out->sk_FragColor = float4(0.0);
                break;
            }
        case 1:
            ;
    }
    return *_out;
}
