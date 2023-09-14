
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
vec4 main() {
    float x = 1.0;
    float y = 2.0;
    int z = 3;
    x = (x - x) + ((y * x) * x) * (y - x);
    y = (x / y) / x;
    z = (((z / 2) % 3 << 4) >> 2) << 1;
    bool b = x > 4.0 == x < 2.0 || 2.0 >= unknownInput && y <= x;
    bool c = unknownInput > 2.0;
    bool d = b ^^ c;
    bool e = b && c;
    bool f = b || c;
    x += 12.0;
    x -= 12.0;
    x *= (y *= 0.1);
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = float((colorGreen.xy, 6));
    y = ((((float(b) * float(c)) * float(d)) * float(e)) * float(f), 6.0);
    z = (colorRed.zw, 6);
    ivec2 w = ivec2(~5);
    w = ~w;
    return (((w.x == 5 && w.y == 5) && x == 6.0) && y == 6.0) && z == 6 ? colorGreen : colorRed;
}
