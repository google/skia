
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
int globalValue = 0;
int side_effecting_ii(int value) {
    globalValue++;
    return value;
}
vec4 main() {
    int _7_two = 2;
    int _8_flatten0 = 1;
    int _9_flatten1 = _7_two;
    int _10_flatten2 = 3;
    int _11_noFlatten0 = int[3](--_7_two, side_effecting_ii(2), 3)[0];
    int _12_noFlatten1 = int[3](side_effecting_ii(1), 2, 3)[1];
    int _13_noFlatten2 = int[3](1, ++_7_two, 3)[2];
    return (_8_flatten0 == _11_noFlatten0 && _9_flatten1 == _12_noFlatten1) && _10_flatten2 == _13_noFlatten2 ? colorGreen : colorRed;
}
