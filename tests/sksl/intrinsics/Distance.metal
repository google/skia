#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float a;
    float b;
    float4 c;
    float4 d;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = abs(_globals.a - _globals.b);
    _out.sk_FragColor.x = distance(_globals.c, _globals.d);
    return _out;
}
