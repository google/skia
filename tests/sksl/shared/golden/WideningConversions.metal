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
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{1.0};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float h = _globals->f;
    int i = int(h);
    uint ui = uint(i);
    short s = short(ui);
    ushort us = ushort(s);
    char b = char(us);
    uchar ub = uchar(b);
    _out->sk_FragColor.x = ((((((_globals->f * h) * float(i)) * float(ui)) * float(s)) * float(us)) * float(b)) * float(ub);
    return *_out;
}
