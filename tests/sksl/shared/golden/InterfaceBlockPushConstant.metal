#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct testBlock {
    char pad0[16];
    float2x2 m1;
    float2x2 m2;
};
struct Globals {
    constant testBlock* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& _anonInterface0 [[buffer(456)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = float4(_globals._anonInterface0->m1[0].x, _globals._anonInterface0->m1[1].y, _globals._anonInterface0->m2[0].x, _globals._anonInterface0->m2[1].y);
    return _out;
}
