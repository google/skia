
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    float x;
    int y;
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
    bool valid = (((x == 3.0 && s.x == 2.0) && s.y == 3) && s == S(2.0, 3)) && constructs_a_struct_S() == s;
    return valid ? colorGreen : colorRed;
}
