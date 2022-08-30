
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
struct S {
    int a;
    int b;
    int c;
};
int numSideEffects = 0;
int side_effecting_ii(int value) {
    numSideEffects++;
    return value;
}
bool test_b() {
    int val1 = 2;
    int val2 = 1;
    int noFlatten0 = S(--val1, side_effecting_ii(2), 3).a;
    int noFlatten1 = S(side_effecting_ii(1), 2, 3).b;
    int noFlatten2 = S(1, val2 += 1, 3).c;
    return ((((noFlatten0 == 1 && noFlatten1 == 2) && noFlatten2 == 3) && val1 == 1) && val2 == 2) && numSideEffects == 2;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
