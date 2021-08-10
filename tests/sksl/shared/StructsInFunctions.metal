#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct S {
    float x;
    int y;
};
struct Nested {
    S a;
    S b;
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

thread bool operator==(thread const S& left, thread const S& right);
thread bool operator!=(thread const S& left, thread const S& right);

thread bool operator==(thread const Nested& left, thread const Nested& right);
thread bool operator!=(thread const Nested& left, thread const Nested& right);
void modifies_a_struct_vS(thread S& s);
void _skOutParamHelper0_modifies_a_struct_vS(thread S& s) {
    S _var0 = s;
    modifies_a_struct_vS(_var0);
    s = _var0;
}
void modifies_a_struct_vS(thread S& s);
void _skOutParamHelper1_modifies_a_struct_vS(thread Nested& n3) {
    S _var0 = n3.b;
    modifies_a_struct_vS(_var0);
    n3.b = _var0;
}
thread bool operator==(thread const S& left, thread const S& right) {
    return (left.x == right.x) &&
           (left.y == right.y);
}
thread bool operator!=(thread const S& left, thread const S& right) {
    return !(left == right);
}
thread bool operator==(thread const Nested& left, thread const Nested& right) {
    return (left.a == right.a) &&
           (left.b == right.b);
}
thread bool operator!=(thread const Nested& left, thread const Nested& right) {
    return !(left == right);
}
S returns_a_struct_S() {
    S s;
    s.x = 1.0;
    s.y = 2;
    return s;
}
S constructs_a_struct_S() {
    return S{2.0, 3};
}
float accepts_a_struct_fS(S s) {
    return s.x + float(s.y);
}
void modifies_a_struct_vS(thread S& s) {
    s.x++;
    s.y++;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    S s = returns_a_struct_S();
    float x = accepts_a_struct_fS(s);
    _skOutParamHelper0_modifies_a_struct_vS(s);
    S expected = constructs_a_struct_S();
    Nested n1;
    Nested n2;
    Nested n3;
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    n2 = n1;
    n3 = n2;
    _skOutParamHelper1_modifies_a_struct_vS(n3);
    bool valid = (((((((x == 3.0 && s.x == 2.0) && s.y == 3) && s == expected) && s == S{2.0, 3}) && s != returns_a_struct_S()) && n1 == n2) && n1 != n3) && n3 == Nested{S{1.0, 2}, S{2.0, 3}};
    _out.sk_FragColor = valid ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
