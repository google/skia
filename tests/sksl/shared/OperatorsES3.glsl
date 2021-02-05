
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 1.0;
    float y = 2.0;

    int z = 3;
    x = 2.0;
    y = 0.5;
    z = 8;
    x += 12.0;
    x -= 12.0;
    x *= (y /= 10.0);
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = 6.0;
    y = 6.0;
    z = 6;
    return colorGreen;
}
