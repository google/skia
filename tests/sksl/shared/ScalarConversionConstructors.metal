#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float f;
    int i;
    uint u;
    bool b;
};




fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{1.0, 1, 1u, true};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float f1 = _globals.f;
    float f2 = float(_globals.i);
    float f3 = float(_globals.u);
    float f4 = float(_globals.b);
    int i1 = int(_globals.f);
    int i2 = _globals.i;
    int i3 = int(_globals.u);
    int i4 = int(_globals.b);
    uint u1 = uint(_globals.f);
    uint u2 = uint(_globals.i);
    uint u3 = _globals.u;
    uint u4 = uint(_globals.b);
    bool b1 = bool(_globals.f);
    bool b2 = bool(_globals.i);
    bool b3 = bool(_globals.u);
    bool b4 = _globals.b;
    _out.sk_FragColor.x = ((f1 + f2) + f3) + f4;
    _out.sk_FragColor.x = float(((i1 + i2) + i3) + i4);
    _out.sk_FragColor.x = float(((u1 + u2) + u3) + u4);
    _out.sk_FragColor.x = float(((b1 || b2) || b3) || b4);
    return _out;
}
