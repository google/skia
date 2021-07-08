
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 testInputs;
vec4 main() {
    bool ok = true;
    ok = ok && mat2(testInputs) == mat2(-1.25, 0.0, 0.75, 2.25);
    ok = ok && mat2(testInputs) == mat2(-1.25, 0.0, 0.75, 2.25);
    return ok ? colorGreen : colorRed;
}
