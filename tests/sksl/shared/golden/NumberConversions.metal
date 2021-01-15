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
    Globals globalStruct{true, short(sqrt(1.0)), int(sqrt(1.0)), ushort(sqrt(1.0)), uint(sqrt(1.0)), sqrt(1.0), sqrt(1.0), (&globalStruct)->s, short((&globalStruct)->i), short((&globalStruct)->us), short((&globalStruct)->ui), short((&globalStruct)->h), short((&globalStruct)->f), short((&globalStruct)->b), int((&globalStruct)->s), (&globalStruct)->i, int((&globalStruct)->us), int((&globalStruct)->ui), int((&globalStruct)->h), int((&globalStruct)->f), int((&globalStruct)->b), ushort((&globalStruct)->s), ushort((&globalStruct)->i), (&globalStruct)->us, ushort((&globalStruct)->ui), ushort((&globalStruct)->h), ushort((&globalStruct)->f), ushort((&globalStruct)->b), uint((&globalStruct)->s), uint((&globalStruct)->i), uint((&globalStruct)->us), (&globalStruct)->ui, uint((&globalStruct)->h), uint((&globalStruct)->f), uint((&globalStruct)->b), float((&globalStruct)->s), float((&globalStruct)->i), float((&globalStruct)->us), float((&globalStruct)->ui), (&globalStruct)->h, (&globalStruct)->f, float((&globalStruct)->b)};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (((((((((((((((((((((float(_globals->s) + float(_globals->i)) + float(_globals->us)) + float(_globals->ui)) + _globals->h) + _globals->f) + float(_globals->s2s)) + float(_globals->i2s)) + float(_globals->us2s)) + float(_globals->ui2s)) + float(_globals->h2s)) + float(_globals->f2s)) + float(_globals->b2s)) + float(_globals->s2i)) + float(_globals->i2i)) + float(_globals->us2i)) + float(_globals->ui2i)) + float(_globals->h2i)) + float(_globals->f2i)) + float(_globals->b2i)) + float(_globals->s2us)) + float(_globals->i2us)) + float(_globals->us2us);
    _out->sk_FragColor.x = _out->sk_FragColor.x + ((((((((((((((((float(_globals->ui2us) + float(_globals->h2us)) + float(_globals->f2us)) + float(_globals->b2us)) + float(_globals->s2ui)) + float(_globals->i2ui)) + float(_globals->us2ui)) + float(_globals->ui2ui)) + float(_globals->h2ui)) + float(_globals->f2ui)) + float(_globals->b2ui)) + _globals->s2f) + _globals->i2f) + _globals->us2f) + _globals->ui2f) + _globals->h2f) + _globals->f2f) + _globals->b2f;
    return *_out;
}
