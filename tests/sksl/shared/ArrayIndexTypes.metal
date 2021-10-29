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
    array<float, 4> array = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    int x = 0;
    uint y = 1u;
    int z = 2;
    uint w = 3u;
    _out.sk_FragColor = float4(array[x], array[y], array[z], array[w]);
    return _out;
}
