#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    int a;
    uint b;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(popcount(_globals.a));
    _out.sk_FragColor.y = float(popcount(_globals.b));
    return _out;
}
