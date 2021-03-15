#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float2 a;
    float2 b;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = _globals.a.x * _globals.b.y - _globals.a.y * _globals.b.x;
    return _out;
}
