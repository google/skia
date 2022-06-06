
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
int globalValue = 0;
int side_effecting_ii(int value) {
    globalValue++;
    return value;
}
vec4 main() {
    const S _0_x = S(1, 2, 3);
    const S _2_y = S(1, 2, 4);
    int _6_two = 2;
    int _7_flatten0 = S(_0_x.a, _6_two, 3).a;
    int _8_flatten1 = S(_0_x.a, _6_two, 3).b;
    int _9_flatten2 = S(_0_x.a, _6_two, 3).c;
    int _10_noFlatten0 = S(--_6_two, side_effecting_ii(2), 3).a;
    int _11_noFlatten1 = S(side_effecting_ii(1), 2, 3).b;
    int _12_noFlatten2 = S(1, ++_6_two, 3).c;
    return ((((_0_x != _2_y && !(_0_x == _2_y)) && _0_x.a == _2_y.a) && _7_flatten0 == _10_noFlatten0) && _8_flatten1 == _11_noFlatten1) && _9_flatten2 == _12_noFlatten2 ? colorGreen : colorRed;
}
