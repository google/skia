
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    ok = ok;
    ok = ok;
    return ok ? colorGreen : colorRed;
}
