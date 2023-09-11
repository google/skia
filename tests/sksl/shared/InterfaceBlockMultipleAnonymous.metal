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
struct testBlockA {
    float2 x;
};
struct testBlockB {
    float2 y;
};
struct Globals {
    constant testBlockA* _anonInterface0;
    constant testBlockB* _anonInterface1;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlockA& _anonInterface0 [[buffer(1)]], constant testBlockB& _anonInterface1 [[buffer(2)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0, &_anonInterface1};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(half2(_globals._anonInterface0->x), half2(_globals._anonInterface1->y));
    return _out;
}
