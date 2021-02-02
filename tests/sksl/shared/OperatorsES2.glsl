
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float x = 1.0;
    float y = 2.0;

    x = 2.0;
    y = 0.5;
    bool c = sqrt(2.0) > 2.0;
    bool d = true ^^ c;
    bool e = c;
    x += 12.0;
    x -= 12.0;
    x *= (y /= 10.0);
    x = 6.0;
    y = (float(c) * float(d)) * float(e);
    y = 6.0;
    return colorGreen;
}
