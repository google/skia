#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Test {
    int x;
    int y;
    int z;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    Test t;
    t.x = 0;
    _out.sk_FragColor.x = half(t.x);
    return _out;
}
