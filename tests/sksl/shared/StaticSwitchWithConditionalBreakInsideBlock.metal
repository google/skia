#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float x = 0.0;
    switch (0) {
        case 0:
            x = 0.0;
            if (x < sqrt(1.0)) {
                _out.sk_FragColor = float4(x);
                break;
            }
        case 1:
            x = 1.0;
    }
    return _out;
}
