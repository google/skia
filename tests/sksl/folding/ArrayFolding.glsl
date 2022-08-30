
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
int globalValue = 0;
int side_effecting_ii(int value) {
    globalValue++;
    return value;
}
bool test_b() {
    int two = 2;
    int flatten0 = 1;
    int flatten1 = two;
    int flatten2 = 3;
    int noFlatten0 = int[3](--two, side_effecting_ii(2), 3)[0];
    int noFlatten1 = int[3](side_effecting_ii(1), 2, 3)[1];
    int noFlatten2 = int[3](1, ++two, 3)[2];
    return (flatten0 == noFlatten0 && flatten1 == noFlatten1) && flatten2 == noFlatten2;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
