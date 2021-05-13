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
    _out.sk_FragColor = (((((((f1 + f2) + f3) + float(i1)) + float(i2)) + float(i3)) + float(b1)) + float(b2)) + float(b3) == 9.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
