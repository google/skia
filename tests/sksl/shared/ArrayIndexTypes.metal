#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    array<float, 4> array = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    short x = 0;
    ushort y = 1u;
    int z = 2;
    uint w = 3u;
    _out.sk_FragColor = half4(half(array[x]), half(array[y]), half(array[z]), half(array[w]));
    return _out;
}
