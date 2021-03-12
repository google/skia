
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 1.0;
    float y = 2.0;
    int z = 3;
    x = (x - x) + ((y * x) * x) * (y - x);
    y = (x / y) / x;
    z = ((z / 2) * 3 + 4) - 2;
    bool b = x > 4.0 == x < 2.0 || 2.0 >= sqrt(2.0) && y <= x;
    bool c = sqrt(2.0) > 2.0;
    bool d = b ^^ c;
    bool e = b && c;
    bool f = b || c;
    x += 12.0;
    x -= 12.0;
    x *= (y /= 10.0);
    x = 6.0;
    y = (((float(b) * float(c)) * float(d)) * float(e)) * float(f);
    y = 6.0;
    z = z - 1;
    z = 6;
    return (x == 6.0 && y == 6.0) && z == 6 ? colorGreen : colorRed;
}
