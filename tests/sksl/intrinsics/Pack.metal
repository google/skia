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
    float4 b;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(packHalf2x16(_globals.a));
    _out.sk_FragColor.x = float(packUnorm2x16(_globals.a));
    _out.sk_FragColor.x = float(packSnorm2x16(_globals.a));
    _out.sk_FragColor.x = float(packUnorm4x8(_globals.b));
    _out.sk_FragColor.x = float(packSnorm4x8(_globals.b));
    return _out;
}
