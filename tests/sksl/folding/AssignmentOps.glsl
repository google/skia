
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bool ok = true;
    int a = 1;
    a = a + a;
    a += a;
    a = a + a;
    a += a;
    a = a + a;
    ok = ok && a == 32;
    int b = 10;
    b = b - 2;
    b -= 2;
    b = b - 1;
    b -= 3;
    ok = ok && b == 2;
    int c = 2;
    c = c * c;
    c *= c;
    c = c * 4;
    c *= 2;
    ok = ok && c == 128;
    int d = 256;
    d = d / 2;
    d /= 2;
    d = d / 4;
    d /= 4;
    ok = ok && d == 4;
    return ok ? colorGreen : colorRed;
}
