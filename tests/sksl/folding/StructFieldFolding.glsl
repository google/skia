
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
void check_array_1_vi(int _skAnonymousParam0[1]) {
}
void check_array_2_vi(int _skAnonymousParam0[2]) {
}
void check_array_3_vi(int _skAnonymousParam0[3]) {
}
bool test_b() {
    int a[1];
    int b[2];
    int c[3];
    check_array_1_vi(a);
    check_array_2_vi(b);
    check_array_3_vi(c);
    int two = 2;
    int flatten0 = 1;
    int flatten1 = two;
    int flatten2 = 3;
    return (flatten0 == 1 && flatten1 == 2) && flatten2 == 3;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
