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
    bool b;
};



fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{1.0, 1, true};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float f1 = _globals->f;
    float f2 = float(_globals->i);
    float f3 = float(_globals->b);
    int i1 = int(_globals->f);
    int i2 = _globals->i;
    int i3 = int(_globals->b);
    bool b1 = bool(_globals->f);
    bool b2 = bool(_globals->i);
    bool b3 = _globals->b;
    _out->sk_FragColor.x = (f1 + f2) + f3;
    _out->sk_FragColor.x = float((i1 + i2) + i3);
    _out->sk_FragColor.x = float((b1 || b2) || b3);
    return *_out;
}
