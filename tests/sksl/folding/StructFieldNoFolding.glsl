
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
vec4 main() {
    int _0_val1 = 2;
    int _1_val2 = 1;
    int _2_noFlatten0 = S(--_0_val1, side_effecting_ii(2), 3).a;
    int _3_noFlatten1 = S(side_effecting_ii(1), 2, 3).b;
    int _4_noFlatten2 = S(1, _1_val2 += 1, 3).c;
    return ((((_2_noFlatten0 == 1 && _3_noFlatten1 == 2) && _4_noFlatten2 == 3) && _0_val1 == 1) && _1_val2 == 2) && numSideEffects == 2 ? colorGreen : colorRed;
}
