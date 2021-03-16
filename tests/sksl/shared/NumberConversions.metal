#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool b = true;
    short s = short(sqrt(1.0));
    int i = int(sqrt(1.0));
    ushort us = ushort(sqrt(1.0));
    uint ui = uint(sqrt(1.0));
    float h = sqrt(1.0);
    float f = sqrt(1.0);
    short s2s = s;
    short i2s = short(i);
    short us2s = short(us);
    short ui2s = short(ui);
    short h2s = short(h);
    short f2s = short(f);
    short b2s = short(b);
    int s2i = int(s);
    int i2i = i;
    int us2i = int(us);
    int ui2i = int(ui);
    int h2i = int(h);
    int f2i = int(f);
    int b2i = int(b);
    ushort s2us = ushort(s);
    ushort i2us = ushort(i);
    ushort us2us = us;
    ushort ui2us = ushort(ui);
    ushort h2us = ushort(h);
    ushort f2us = ushort(f);
    ushort b2us = ushort(b);
    uint s2ui = uint(s);
    uint i2ui = uint(i);
    uint us2ui = uint(us);
    uint ui2ui = ui;
    uint h2ui = uint(h);
    uint f2ui = uint(f);
    uint b2ui = uint(b);
    float s2f = float(s);
    float i2f = float(i);
    float us2f = float(us);
    float ui2f = float(ui);
    float h2f = h;
    float f2f = f;
    float b2f = float(b);
    _out.sk_FragColor.x = (((((((((((((((((((((float(s) + float(i)) + float(us)) + float(ui)) + h) + f) + float(s2s)) + float(i2s)) + float(us2s)) + float(ui2s)) + float(h2s)) + float(f2s)) + float(b2s)) + float(s2i)) + float(i2i)) + float(us2i)) + float(ui2i)) + float(h2i)) + float(f2i)) + float(b2i)) + float(s2us)) + float(i2us)) + float(us2us);
    _out.sk_FragColor.x = _out.sk_FragColor.x + ((((((((((((((((float(ui2us) + float(h2us)) + float(f2us)) + float(b2us)) + float(s2ui)) + float(i2ui)) + float(us2ui)) + float(ui2ui)) + float(h2ui)) + float(f2ui)) + float(b2ui)) + s2f) + i2f) + us2f) + ui2f) + h2f) + f2f) + b2f;
    return _out;
}
