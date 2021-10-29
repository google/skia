#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct testBlock {
    float x;
    int w;
    char pad0[8];
    array<float, 2> y;
    char pad1[24];
    float3x3 z;
};
struct Globals {
    constant testBlock* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = float4(_globals._anonInterface0->x, _globals._anonInterface0->y[0], _globals._anonInterface0->y[1], 0.0);
    return _out;
}
