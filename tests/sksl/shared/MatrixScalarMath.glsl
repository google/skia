
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
const int minus = 2;
const int star = 3;
const int slash = 4;
bool test_bifffff22(int op, float m11, float m12, float m21, float m22, mat2 expected) {
    float one = colorRed.x;
    mat2 m2 = mat2(m11 * one, m12 * one, m21 * one, m22 * one);
    switch (op) {
        case 1:
            m2 += 1.0;
            break;
        case 2:
            m2 -= 1.0;
            break;
        case 3:
            m2 *= 2.0;
            break;
        case 4:
            m2 /= 2.0;
            break;
    }
    return ((m2[0].x == expected[0].x && m2[0].y == expected[0].y) && m2[1].x == expected[1].x) && m2[1].y == expected[1].y;
}
vec4 main() {
    float f1 = colorGreen.y;
    float f2 = 2.0 * colorGreen.y;
    float f3 = 3.0 * colorGreen.y;
    float f4 = 4.0 * colorGreen.y;
    mat2 _0_expected = mat2(f1 + 1.0, f2 + 1.0, f3 + 1.0, f4 + 1.0);
    float _1_one = colorRed.x;
    mat2 _2_m2 = mat2(f1 * _1_one, f2 * _1_one, f3 * _1_one, f4 * _1_one);
    {
        _2_m2 += 1.0;
    }
    return (((((_2_m2[0].x == _0_expected[0].x && _2_m2[0].y == _0_expected[0].y) && _2_m2[1].x == _0_expected[1].x) && _2_m2[1].y == _0_expected[1].y) && test_bifffff22(minus, f1, f2, f3, f4, mat2(f1 - 1.0, f2 - 1.0, f3 - 1.0, f4 - 1.0))) && test_bifffff22(star, f1, f2, f3, f4, mat2(f1 * 2.0, f2 * 2.0, f3 * 2.0, f4 * 2.0))) && test_bifffff22(slash, f1, f2, f3, f4, mat2(f1 / 2.0, f2 / 2.0, f3 / 2.0, f4 / 2.0)) ? colorGreen : colorRed;
}
