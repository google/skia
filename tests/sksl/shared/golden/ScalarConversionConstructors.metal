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
    Globals _skGlobals{1.0, 1, 1u, true};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float f1 = _skGlobals.f;
    float f2 = float(_skGlobals.i);
    float f3 = float(_skGlobals.u);
    float f4 = float(_skGlobals.b);
    int i1 = int(_skGlobals.f);
    int i2 = _skGlobals.i;
    int i3 = int(_skGlobals.u);
    int i4 = int(_skGlobals.b);
    uint u1 = uint(_skGlobals.f);
    uint u2 = uint(_skGlobals.i);
    uint u3 = _skGlobals.u;
    uint u4 = uint(_skGlobals.b);
    bool b1 = bool(_skGlobals.f);
    bool b2 = bool(_skGlobals.i);
    bool b3 = bool(_skGlobals.u);
    bool b4 = _skGlobals.b;
    _out->sk_FragColor.x = ((f1 + f2) + f3) + f4;
    _out->sk_FragColor.x = float(((i1 + i2) + i3) + i4);
    _out->sk_FragColor.x = float(((u1 + u2) + u3) + u4);
    _out->sk_FragColor.x = float(((b1 || b2) || b3) || b4);
    return *_out;
}
