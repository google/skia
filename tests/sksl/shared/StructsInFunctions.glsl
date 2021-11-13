
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    float x;
    int y;
};
struct Nested {
    S a;
    S b;
};
struct Compound {
    vec4 f4;
    ivec3 i3;
};
S returns_a_struct_S() {
    S s;
    s.x = 1.0;
    s.y = 2;
    return s;
}
S constructs_a_struct_S() {
    return S(2.0, 3);
}
float accepts_a_struct_fS(S s) {
    return s.x + float(s.y);
}
void modifies_a_struct_vS(inout S s) {
    s.x++;
    s.y++;
}
vec4 main() {
    S s = returns_a_struct_S();
    float x = accepts_a_struct_fS(s);
    modifies_a_struct_vS(s);
    S expected = constructs_a_struct_S();
    Nested n1;
    Nested n2;
    Nested n3;
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    n2 = n1;
    n3 = n2;
    modifies_a_struct_vS(n3.b);
    Compound c1 = Compound(vec4(1.0, 2.0, 3.0, 4.0), ivec3(5, 6, 7));
    Compound c2 = Compound(vec4(colorGreen.y, 2.0, 3.0, 4.0), ivec3(5, 6, 7));
    Compound c3 = Compound(vec4(colorGreen.x, 2.0, 3.0, 4.0), ivec3(5, 6, 7));
    bool valid = (((((((((x == 3.0 && s.x == 2.0) && s.y == 3) && s == expected) && s == S(2.0, 3)) && s != returns_a_struct_S()) && n1 == n2) && n1 != n3) && n3 == Nested(S(1.0, 2), S(2.0, 3))) && c1 == c2) && c2 != c3;
    return valid ? colorGreen : colorRed;
}
