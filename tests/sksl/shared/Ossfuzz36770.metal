#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct T {
    int x;
    float2 u_skRTFlip;
};
struct Globals {
    constant T* _anonInterface0;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant T& _anonInterface0 [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&_anonInterface0};
    (void)_globals;
    Outputs _out;
    (void)_out;
    return _out;
}
