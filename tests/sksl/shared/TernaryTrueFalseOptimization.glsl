
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    ok = ok && colorGreen.y == 1.0;
    ok = ok && colorGreen.x != 1.0;
    ok = ok && colorGreen.yx == colorRed.xy;
    ok = ok && colorGreen.yx == colorRed.xy;
    ok = ok && (colorGreen.yx == colorRed.xy || colorGreen.w != colorRed.w);
    ok = ok && (colorGreen.yx != colorRed.xy && colorGreen.w == colorRed.w);
    return ok ? colorGreen : colorRed;
}
