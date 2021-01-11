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
    _out->sk_FragColor.x = float(b);
    ushort us = ushort(b);
    _out->sk_FragColor.x = float(us);
    short s = short(us);
    _out->sk_FragColor.x = float(s);
    uint ui = uint(s);
    _out->sk_FragColor.x = float(ui);
    int i = int(ui);
    _out->sk_FragColor.x = float(i);
    float h = float(i);
    _out->sk_FragColor.x = h;
    float f = h;
    _out->sk_FragColor.x = f;
    return *_out;
}
