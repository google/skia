#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    uchar ub;
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{1u};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    char b = char(_globals->ub);
    ushort us = ushort(b);
    short s = short(us);
    uint ui = uint(s);
    int i = int(ui);
    float h = float(i);
    float f = h;
    _out->sk_FragColor.x = ((((((f * h) * float(i)) * float(ui)) * float(s)) * float(us)) * float(b)) * float(_globals->ub);
    return *_out;
}
