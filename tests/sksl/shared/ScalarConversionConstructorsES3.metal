#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float f = float(_uniforms.colorGreen.y);
    int i = int(_uniforms.colorGreen.y);
    uint u = uint(_uniforms.colorGreen.y);
    bool b = bool(_uniforms.colorGreen.y);
    float f1 = f;
    float f2 = float(i);
    float f3 = float(u);
    float f4 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(u);
    int i4 = int(b);
    uint u1 = uint(f);
    uint u2 = uint(i);
    uint u3 = u;
    uint u4 = uint(b);
    bool b1 = bool(f);
    bool b2 = bool(i);
    bool b3 = bool(u);
    bool b4 = b;
    _out.sk_FragColor = ((((((((((((((half(f1) + half(f2)) + half(f3)) + half(f4)) + half(i1)) + half(i2)) + half(i3)) + half(i4)) + half(u1)) + half(u2)) + half(u3)) + half(u4)) + half(b1)) + half(b2)) + half(b3)) + half(b4) == 16.0h ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
