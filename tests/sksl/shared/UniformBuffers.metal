#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct testBlock {
    float myHalf;
    float4 myHalf4;
};
struct Globals {
    constant testBlock* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _globals._anonInterface0->myHalf4 * _globals._anonInterface0->myHalf;
    return _out;
}
