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
    float4 result = float4(0.0);
    for (int a = 0;
    int b = 0;
    a < 10 && b < 10; (++a , ++b)) {
        result.x = result.x + 1.0;
        result.y = result.y + 2.0;
    }
    _out.sk_FragColor = result;
    return _out;
}
