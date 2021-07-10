
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    bool ok = true;
    ok = ok && mat2(testInputs) == mat2(-1.25, 0.0, 0.75, 2.25);
    ok = ok && mat2(testInputs) == mat2(-1.25, 0.0, 0.75, 2.25);
    ok = ok && mat2(colorGreen) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(vec4(ivec4(colorGreen))) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(colorGreen) == mat2(0.0, 1.0, 0.0, 1.0);
    ok = ok && mat2(vec4(bvec4(colorGreen))) == mat2(0.0, 1.0, 0.0, 1.0);
    return ok ? colorGreen : colorRed;
}
