#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct testBlock {
    half x;
    char pad0[2];
    int w;
    char pad1[8];
    array<half, 2> y;
    char pad2[28];
    half3x3 z;
};
struct Globals {
    constant testBlock* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(_globals._anonInterface0->x, _globals._anonInterface0->y[0], _globals._anonInterface0->y[1], 0.0h);
    return _out;
}
