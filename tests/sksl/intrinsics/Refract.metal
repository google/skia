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
    float c;
    float4 d;
    float4 e;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = (refract(float2(_globals.a, 0), float2(_globals.b, 0), _globals.c).x);
    _out.sk_FragColor = refract(_globals.d, _globals.e, _globals.c);
    return _out;
}
