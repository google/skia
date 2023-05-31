
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    int i = 5;
    i++;
    ok = ok && i++ == 6;
    ok = ok && i == 7;
    ok = ok && i-- == 7;
    ok = ok && i == 6;
    i--;
    ok = ok && i == 5;
    float f = 0.5;
    f++;
    ok = ok && f++ == 1.5;
    ok = ok && f == 2.5;
    ok = ok && f-- == 2.5;
    ok = ok && f == 1.5;
    f--;
    ok = ok && f == 0.5;
    return ok ? colorGreen : colorRed;
}
