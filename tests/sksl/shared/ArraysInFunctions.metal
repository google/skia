#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    array<float, 2> a;
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
void modifies_an_array(thread array<float, 2>& a);
void _skOutParamHelper0_modifies_an_array(thread S& s) {
    array<float, 2> _var0 = s.a;
    modifies_an_array(_var0);
    s.a = _var0;
}


S returns_an_array_in_a_struct() {
    S s;
    s.a[0] = 1.0;
    s.a[1] = 2.0;
    return s;
}
float accepts_an_array(array<float, 2> a) {
    return a[0] + a[1];
}
void modifies_an_array(thread array<float, 2>& a) {
    a[0]++;
    a[1]++;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    S s = returns_an_array_in_a_struct();
    float x = accepts_an_array(s.a);
    _skOutParamHelper0_modifies_an_array(s);
    bool valid = (x == 3.0 && s.a[0] == 2.0) && s.a[1] == 3.0;
    _out.sk_FragColor = valid ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
