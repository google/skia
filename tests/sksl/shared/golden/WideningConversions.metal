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
    _out->sk_FragColor.x = h;
    int i = int(h);
    _out->sk_FragColor.x = float(i);
    uint ui = uint(i);
    _out->sk_FragColor.x = float(ui);
    short s = short(ui);
    _out->sk_FragColor.x = float(s);
    ushort us = ushort(s);
    _out->sk_FragColor.x = float(us);
    char b = char(us);
    _out->sk_FragColor.x = float(b);
    uchar ub = uchar(b);
    _out->sk_FragColor.x = float(ub);
    return *_out;
}
