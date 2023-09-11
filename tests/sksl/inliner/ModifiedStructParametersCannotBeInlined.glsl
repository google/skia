
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct S {
    float a;
    float b;
    float c;
};
float sumStructMutating_fS(S s) {
    s.a += s.b;
    s.a += s.c;
    return s.a;
}
vec4 main() {
    S s = S(1.0, 2.0, 3.0);
    float _0_sum = s.a;
    _0_sum += s.b;
    _0_sum += s.c;
    return _0_sum == sumStructMutating_fS(s) ? colorGreen : colorRed;
}
