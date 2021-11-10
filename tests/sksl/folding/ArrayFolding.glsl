
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
int globalValue = 0;
int side_effecting_ii(int value) {
    globalValue++;
    return value;
}
vec4 main() {
    int _7_flatten0 = 1;
    int _8_flatten1 = 2;
    int _9_flatten2 = 3;
    int _10_noFlatten0 = int[3](1, side_effecting_ii(2), 3)[0];
    int _11_noFlatten1 = int[3](side_effecting_ii(1), 2, 3)[1];
    int _12_noFlatten2 = int[3](1, 2, side_effecting_ii(3))[2];
    return (_7_flatten0 == _10_noFlatten0 && _8_flatten1 == _11_noFlatten1) && _9_flatten2 == _12_noFlatten2 ? colorGreen : colorRed;
}
