#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    float x;
    int y;
};
struct Nested {
    S a;
    S b;
};
struct Compound {
    float4 f4;
    int3 i3;
};
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(thread const S& left, thread const S& right);
thread bool operator!=(thread const S& left, thread const S& right);

thread bool operator==(thread const Nested& left, thread const Nested& right);
thread bool operator!=(thread const Nested& left, thread const Nested& right);

thread bool operator==(thread const Compound& left, thread const Compound& right);
thread bool operator!=(thread const Compound& left, thread const Compound& right);
thread bool operator==(thread const S& left, thread const S& right) {
    return all(left.x == right.x) &&
           all(left.y == right.y);
}
thread bool operator!=(thread const S& left, thread const S& right) {
    return !(left == right);
}
thread bool operator==(thread const Nested& left, thread const Nested& right) {
    return all(left.a == right.a) &&
           all(left.b == right.b);
}
thread bool operator!=(thread const Nested& left, thread const Nested& right) {
    return !(left == right);
}
thread bool operator==(thread const Compound& left, thread const Compound& right) {
    return all(left.f4 == right.f4) &&
           all(left.i3 == right.i3);
}
thread bool operator!=(thread const Compound& left, thread const Compound& right) {
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
    S _skTemp0;
    S _skTemp1;
    S s = returns_a_struct_S();
    float x = accepts_a_struct_fS(s);
    ((modifies_a_struct_vS((_skTemp0 = s))), (s = _skTemp0));
    S expected = constructs_a_struct_S();
    Nested n1;
    Nested n2;
    Nested n3;
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    n2 = n1;
    n3 = n2;
    ((modifies_a_struct_vS((_skTemp1 = n3.b))), (n3.b = _skTemp1));
    Compound c1 = Compound{float4(1.0, 2.0, 3.0, 4.0), int3(5, 6, 7)};
    Compound c2 = Compound{float4(float(_uniforms.colorGreen.y), 2.0, 3.0, 4.0), int3(5, 6, 7)};
    Compound c3 = Compound{float4(float(_uniforms.colorGreen.x), 2.0, 3.0, 4.0), int3(5, 6, 7)};
    bool valid = (((((((((x == 3.0 && s.x == 2.0) && s.y == 3) && s == expected) && s == S{2.0, 3}) && s != returns_a_struct_S()) && n1 == n2) && n1 != n3) && n3 == Nested{S{1.0, 2}, S{2.0, 3}}) && c1 == c2) && c2 != c3;
    _out.sk_FragColor = valid ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
