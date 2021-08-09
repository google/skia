#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float unknownInput;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool b = true;
    int s = int(_uniforms.unknownInput);
    int i = int(_uniforms.unknownInput);
    uint us = uint(_uniforms.unknownInput);
    uint ui = uint(_uniforms.unknownInput);
    float h = _uniforms.unknownInput;
    float f = _uniforms.unknownInput;
    int s2s = s;
    int i2s = int(i);
    int us2s = int(us);
    int ui2s = int(ui);
    int h2s = int(h);
    int f2s = int(f);
    int b2s = int(b);
    int s2i = int(s);
    int i2i = i;
    int us2i = int(us);
    int ui2i = int(ui);
    int h2i = int(h);
    int f2i = int(f);
    int b2i = int(b);
    uint s2us = uint(s);
    uint i2us = uint(i);
    uint us2us = us;
    uint ui2us = uint(ui);
    uint h2us = uint(h);
    uint f2us = uint(f);
    uint b2us = uint(b);
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
