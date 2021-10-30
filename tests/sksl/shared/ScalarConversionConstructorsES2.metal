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
    bool b = bool(_uniforms.colorGreen.y);
    float f1 = f;
    float f2 = float(i);
    float f3 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(b);
    bool b1 = bool(f);
    bool b2 = bool(i);
    bool b3 = b;
    _out.sk_FragColor = (((((((half(f1) + half(f2)) + half(f3)) + half(i1)) + half(i2)) + half(i3)) + half(b1)) + half(b2)) + half(b3) == 9.0h ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
