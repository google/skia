
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    ok = ok && (colorGreen.y == 1.0 ? true : false);
    ok = ok && (colorGreen.x == 1.0 ? false : true);
    return ok ? colorGreen : colorRed;
}
