
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform mat2 testMatrix2x2;
vec4 main() {
    bool ok = true;
    int i = 5;
    ++i;
    ok = ok && i == 6;
    ok = ok && ++i == 7;
    ok = ok && --i == 6;
    --i;
    ok = ok && i == 5;
    float f = 0.5;
    ++f;
    ok = ok && f == 1.5;
    ok = ok && ++f == 2.5;
    ok = ok && --f == 1.5;
    --f;
    ok = ok && f == 0.5;
    vec2 f2 = vec2(0.5);
    ++f2.x;
    ok = ok && f2.x == 1.5;
    ok = ok && ++f2.x == 2.5;
    ok = ok && --f2.x == 1.5;
    --f2.x;
    ok = ok && f2.x == 0.5;
    ok = ok && colorGreen.x != 1.0;
    ok = ok && -1.0 == -colorGreen.y;
    ok = ok && vec4(0.0, -1.0, 0.0, -1.0) == -colorGreen;
    ok = ok && mat2(-1.0, -2.0, -3.0, -4.0) == -testMatrix2x2;
    ivec2 iv = ivec2(i, -i);
    ok = ok && -i == -5;
    ok = ok && -iv == ivec2(-5, 5);
    return ok ? colorGreen : colorRed;
}
