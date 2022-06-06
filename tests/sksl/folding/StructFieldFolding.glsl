
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
vec4 main() {
    const S _0_x = S(1, 2, 3);
    const S _2_y = S(1, 2, 4);
    int _6_two = 2;
    int _7_flatten0 = 1;
    int _8_flatten1 = _6_two;
    int _9_flatten2 = 3;
    return (((_0_x != _2_y && !(_0_x == _2_y)) && _7_flatten0 == 1) && _8_flatten1 == 2) && _9_flatten2 == 3 ? colorGreen : colorRed;
}
