#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    bool4 a;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(not(_globals.a).x ? 1 : 0);
    return _out;
}
