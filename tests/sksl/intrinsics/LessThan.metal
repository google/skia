#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float4 a;
    float4 b;
    uint2 c;
    uint2 d;
    int3 e;
    int3 f;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float((_globals.a < _globals.b).x ? 1 : 0);
    _out.sk_FragColor.y = float((_globals.c < _globals.d).y ? 1 : 0);
    _out.sk_FragColor.z = float((_globals.e < _globals.f).z ? 1 : 0);
    return _out;
}
