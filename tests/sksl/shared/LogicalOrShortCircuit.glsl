
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool TrueFalse_b() {
    int x = 1;
    int y = 1;
    if (x == 1 || (y += 1) == 3) {
        return x == 1 && y == 1;
    } else {
        return false;
    }
}
bool FalseTrue_b() {
    int x = 1;
    int y = 1;
    if (x == 2 || (y += 1) == 2) {
        return x == 1 && y == 2;
    } else {
        return false;
    }
}
bool FalseFalse_b() {
    int x = 1;
    int y = 1;
    if (x == 2 || (y += 1) == 3) {
        return false;
    } else {
        return x == 1 && y == 2;
    }
}
vec4 main() {
    bool _0_TrueTrue;
    int _2_y = 1;
    {
        _0_TrueTrue = _2_y == 1;
    }
    return ((_0_TrueTrue && TrueFalse_b()) && FalseTrue_b()) && FalseFalse_b() ? colorGreen : colorRed;
}
