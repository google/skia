#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    bool b;
    short s;
    int i;
    ushort us;
    uint ui;
    float h;
    float f;
    short s2s;
    short i2s;
    short us2s;
    short ui2s;
    short h2s;
    short f2s;
    short b2s;
    int s2i;
    int i2i;
    int us2i;
    int ui2i;
    int h2i;
    int f2i;
    int b2i;
    ushort s2us;
    ushort i2us;
    ushort us2us;
    ushort ui2us;
    ushort h2us;
    ushort f2us;
    ushort b2us;
    uint s2ui;
    uint i2ui;
    uint us2ui;
    uint ui2ui;
    uint h2ui;
    uint f2ui;
    uint b2ui;
    float s2f;
    float i2f;
    float us2f;
    float ui2f;
    float h2f;
    float f2f;
    float b2f;
};










































fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _skGlobals{true, short(sqrt(1.0)), int(sqrt(1.0)), ushort(sqrt(1.0)), uint(sqrt(1.0)), sqrt(1.0), sqrt(1.0), _skGlobals.s, short(_skGlobals.i), short(_skGlobals.us), short(_skGlobals.ui), short(_skGlobals.h), short(_skGlobals.f), short(_skGlobals.b), int(_skGlobals.s), _skGlobals.i, int(_skGlobals.us), int(_skGlobals.ui), int(_skGlobals.h), int(_skGlobals.f), int(_skGlobals.b), ushort(_skGlobals.s), ushort(_skGlobals.i), _skGlobals.us, ushort(_skGlobals.ui), ushort(_skGlobals.h), ushort(_skGlobals.f), ushort(_skGlobals.b), uint(_skGlobals.s), uint(_skGlobals.i), uint(_skGlobals.us), _skGlobals.ui, uint(_skGlobals.h), uint(_skGlobals.f), uint(_skGlobals.b), float(_skGlobals.s), float(_skGlobals.i), float(_skGlobals.us), float(_skGlobals.ui), _skGlobals.h, _skGlobals.f, float(_skGlobals.b)};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (((((((((((((((((((((float(_skGlobals.s) + float(_skGlobals.i)) + float(_skGlobals.us)) + float(_skGlobals.ui)) + _skGlobals.h) + _skGlobals.f) + float(_skGlobals.s2s)) + float(_skGlobals.i2s)) + float(_skGlobals.us2s)) + float(_skGlobals.ui2s)) + float(_skGlobals.h2s)) + float(_skGlobals.f2s)) + float(_skGlobals.b2s)) + float(_skGlobals.s2i)) + float(_skGlobals.i2i)) + float(_skGlobals.us2i)) + float(_skGlobals.ui2i)) + float(_skGlobals.h2i)) + float(_skGlobals.f2i)) + float(_skGlobals.b2i)) + float(_skGlobals.s2us)) + float(_skGlobals.i2us)) + float(_skGlobals.us2us);
    _out->sk_FragColor.x = _out->sk_FragColor.x + ((((((((((((((((float(_skGlobals.ui2us) + float(_skGlobals.h2us)) + float(_skGlobals.f2us)) + float(_skGlobals.b2us)) + float(_skGlobals.s2ui)) + float(_skGlobals.i2ui)) + float(_skGlobals.us2ui)) + float(_skGlobals.ui2ui)) + float(_skGlobals.h2ui)) + float(_skGlobals.f2ui)) + float(_skGlobals.b2ui)) + _skGlobals.s2f) + _skGlobals.i2f) + _skGlobals.us2f) + _skGlobals.ui2f) + _skGlobals.h2f) + _skGlobals.f2f) + _skGlobals.b2f;
    return *_out;
}
