
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat3 testMatrix3x3;
uniform mat4 testMatrix4x4;
bool test3x3_b() {
    vec3 expected = vec3(3.0, 2.0, 1.0);
    vec3 vec;
    for (int c = 0;c < 3; ++c) {
        for (int r = 0;r < 3; ++r) {
            vec.zyx[r] = testMatrix3x3[c][r];
        }
        if (vec != expected) {
            return false;
        }
        expected += 3.0;
    }
    return true;
}
bool test4x4_b() {
    vec4 expected = vec4(4.0, 3.0, 2.0, 1.0);
    vec4 vec;
    for (int c = 0;c < 4; ++c) {
        for (int r = 0;r < 4; ++r) {
            vec.wzyx[r] = testMatrix4x4[c][r];
        }
        if (vec != expected) {
            return false;
        }
        expected += 4.0;
    }
    return true;
}
vec4 main() {
    return test3x3_b() && test4x4_b() ? colorGreen : colorRed;
}
