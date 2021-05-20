
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
int scratchVar = 0;
bool test_flat_b() {
    return true;
}
bool test_if_b() {
    if (colorGreen.y > 0.0) {
        return true;
    } else {
        ++scratchVar;
    }
    ++scratchVar;
    return false;
}
bool test_else_b() {
    if (colorGreen.y == 0.0) {
        return false;
    } else {
        return true;
    }
}
bool test_loop_if_b() {
    for (int x = 0;x <= 1; ++x) {
        if (colorGreen.y == 0.0) {
            return false;
        } else {
            return true;
        }
    }
    ++scratchVar;
    return true;
}
vec4 main() {
    return ((test_flat_b() && test_if_b()) && test_else_b()) && test_loop_if_b() ? colorGreen : colorRed;
}
