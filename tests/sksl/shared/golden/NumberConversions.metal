#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
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
    int s2i;
    int i2i;
    int us2i;
    int ui2i;
    int h2i;
    int f2i;
    ushort s2us;
    ushort i2us;
    ushort us2us;
    ushort ui2us;
    ushort h2us;
    ushort f2us;
    uint s2ui;
    uint i2ui;
    uint us2ui;
    uint ui2ui;
    uint h2ui;
    uint f2ui;
    float s2f;
    float i2f;
    float us2f;
    float ui2f;
    float h2f;
    float f2f;
};




































fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{short(sqrt(1.0)), int(sqrt(1.0)), ushort(sqrt(1.0)), uint(sqrt(1.0)), sqrt(1.0), sqrt(1.0), _globals->s, short(_globals->i), short(_globals->us), short(_globals->ui), short(_globals->h), short(_globals->f), int(_globals->s), _globals->i, int(_globals->us), int(_globals->ui), int(_globals->h), int(_globals->f), ushort(_globals->s), ushort(_globals->i), _globals->us, ushort(_globals->ui), ushort(_globals->h), ushort(_globals->f), uint(_globals->s), uint(_globals->i), uint(_globals->us), _globals->ui, uint(_globals->h), uint(_globals->f), float(_globals->s), float(_globals->i), float(_globals->us), float(_globals->ui), _globals->h, _globals->f};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (((((((((((((((((float((int(_globals->s) + _globals->i) + int(_globals->us)) + float(_globals->ui)) + _globals->h) + _globals->f) + float(_globals->s2s)) + float(_globals->i2s)) + float(_globals->us2s)) + float(_globals->ui2s)) + float(_globals->h2s)) + float(_globals->f2s)) + float(_globals->s2i)) + float(_globals->i2i)) + float(_globals->us2i)) + float(_globals->ui2i)) + float(_globals->h2i)) + float(_globals->f2i)) + float(_globals->s2us)) + float(_globals->i2us)) + float(_globals->us2us);
    _out->sk_FragColor.x = _out->sk_FragColor.x + (((((((((((float((_globals->ui2us + _globals->h2us) + _globals->f2us) + float(_globals->s2ui)) + float(_globals->i2ui)) + float(_globals->us2ui)) + float(_globals->ui2ui)) + float(_globals->h2ui)) + float(_globals->f2ui)) + _globals->s2f) + _globals->i2f) + _globals->us2f) + _globals->ui2f) + _globals->h2f) + _globals->f2f;
    return *_out;
}
