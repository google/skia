#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float f = _uniforms.colorGreen.y;
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
    _out.sk_FragColor = ((((((((((((((f1 + f2) + f3) + f4) + float(i1)) + float(i2)) + float(i3)) + float(i4)) + float(u1)) + float(u2)) + float(u3)) + float(u4)) + float(b1)) + float(b2)) + float(b3)) + float(b4) == 16.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
