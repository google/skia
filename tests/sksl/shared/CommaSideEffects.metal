#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
    half4 colorWhite;
    half4 colorBlack;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void setToColorBlack_vh4(Uniforms _uniforms, thread half4& x);
void _skOutParamHelper0_setToColorBlack_vh4(Uniforms _uniforms, thread half4& d) {
    half4 _var0;
    setToColorBlack_vh4(_uniforms, _var0);
    d = _var0;
}
void setToColorBlack_vh4(Uniforms _uniforms, thread half4& x) {
    x = _uniforms.colorBlack;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 a;
    half4 b;
    half4 c;
    half4 d;
    (b = _uniforms.colorRed, c = _uniforms.colorGreen);
    a = (    _skOutParamHelper0_setToColorBlack_vh4(_uniforms, d), _uniforms.colorWhite);
    a *= a;
    b *= b;
    c *= c;
    d *= d;
    _out.sk_FragColor = ((all(a == _uniforms.colorWhite) && all(b == _uniforms.colorRed)) && all(c == _uniforms.colorGreen)) && all(d == _uniforms.colorBlack) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
