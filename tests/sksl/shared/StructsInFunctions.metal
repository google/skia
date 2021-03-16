#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float x;
    int y;
};
struct Uniforms {
    float4 colorRed;
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void modifies_a_struct(thread S& s);
void _skOutParamHelper0_modifies_a_struct(thread S& s) {
    S _var0 = s;
    modifies_a_struct(_var0);
    s = _var0;
}

S returns_a_struct() {
    S s;
    s.x = 1.0;
    s.y = 2;
    return s;
}
float accepts_a_struct(S s) {
    return s.x + float(s.y);
}
void modifies_a_struct(thread S& s) {
    s.x++;
    s.y++;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    S s = returns_a_struct();
    float x = accepts_a_struct(s);
    _skOutParamHelper0_modifies_a_struct(s);
    bool valid = (x == 3.0 && s.x == 2.0) && s.y == 3;
    _out.sk_FragColor = valid ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
