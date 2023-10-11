
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int c;
};
bool inner_test_b() {
    S x = S(1);
    S y = S(1);
    return x == y;
}
bool test_bb(bool S) {
    return inner_test_b();
}
vec4 main() {
    return test_bb(true) ? colorGreen : colorRed;
}
