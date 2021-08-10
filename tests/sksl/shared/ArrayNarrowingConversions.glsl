
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    int i2[2] = int[2](1, 2);
    int s2[2] = int[2](1, 2);
    float f2[2] = float[2](1.0, 2.0);
    float h2[2] = float[2](1.0, 2.0);
    i2 = s2;
    s2 = i2;
    f2 = h2;
    h2 = f2;
    const float cf2[2] = float[2](1.0, 2.0);
    return ((i2 == s2 && f2 == h2) && i2 == int[2](1, 2)) && h2 == cf2 ? colorGreen : colorRed;
}
