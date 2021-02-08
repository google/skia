
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float f = colorGreen.y;
    int i = int(colorGreen.y);
    bool b = bool(colorGreen.y);
    float f1 = f;
    float f2 = float(i);
    float f3 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(b);
    bool b1 = bool(f);
    bool b2 = bool(i);
    bool b3 = b;
    return (((((((f1 + f2) + f3) + float(i1)) + float(i2)) + float(i3)) + float(b1)) + float(b2)) + float(b3) == 9.0 ? colorGreen : colorRed;
}
