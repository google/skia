
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    bool TRUE = bool(colorGreen.y);
    ok = ok && 1 == int(TRUE);
    ok = ok && 1.0 == float(TRUE);
    ok = ok && TRUE;
    ok = ok && 1 == int(TRUE);
    ok = ok && 1.0 == float(TRUE);
    ok = ok && bvec2(true) == bvec2(TRUE);
    ok = ok && ivec2(1) == ivec2(int(TRUE));
    ok = ok && vec2(1.0) == vec2(float(TRUE));
    return ok ? colorGreen : colorRed;
}
