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
    _out.sk_FragColor.x = atan(_globals.a);
    _out.sk_FragColor.x = atan2(_globals.a, _globals.b);
    _out.sk_FragColor = atan(_globals.c);
    _out.sk_FragColor = atan2(_globals.c, _globals.d);
    return _out;
}
