#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool b = true;
    short s = short(_uniforms.unknownInput);
    int i = int(_uniforms.unknownInput);
    ushort us = ushort(_uniforms.unknownInput);
    uint ui = uint(_uniforms.unknownInput);
    half h = half(_uniforms.unknownInput);
    float f = _uniforms.unknownInput;
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
    float h2f = float(h);
    float f2f = f;
    float b2f = float(b);
    _out.sk_FragColor.x = (((((((((((((((((((((half(s) + half(i)) + half(us)) + half(ui)) + h) + half(f)) + half(s2s)) + half(i2s)) + half(us2s)) + half(ui2s)) + half(h2s)) + half(f2s)) + half(b2s)) + half(s2i)) + half(i2i)) + half(us2i)) + half(ui2i)) + half(h2i)) + half(f2i)) + half(b2i)) + half(s2us)) + half(i2us)) + half(us2us);
    _out.sk_FragColor.x = _out.sk_FragColor.x + (((((((((((((((((half(ui2us) + half(h2us)) + half(f2us)) + half(b2us)) + half(s2ui)) + half(i2ui)) + half(us2ui)) + half(ui2ui)) + half(h2ui)) + half(f2ui)) + half(b2ui)) + half(s2f)) + half(i2f)) + half(us2f)) + half(ui2f)) + half(h2f)) + half(f2f)) + half(b2f));
    return _out;
}
