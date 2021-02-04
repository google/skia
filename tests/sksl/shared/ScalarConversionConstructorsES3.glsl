
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float f = colorGreen.y;
    int i = int(colorGreen.y);
    uint u = uint(colorGreen.y);
    bool b = bool(colorGreen.y);
    float f1 = f;
    float f2 = float(i);
    float f3 = float(u);
    float f4 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(u);
    int i4 = int(b);
    uint u1 = uint(f);
    uint u2 = uint(i);
    uint u3 = u;
    uint u4 = uint(b);
    bool b1 = bool(f);
    bool b2 = bool(i);
    bool b3 = bool(u);
    bool b4 = b;
    return ((((((((((((((f1 + f2) + f3) + f4) + float(i1)) + float(i2)) + float(i3)) + float(i4)) + float(u1)) + float(u2)) + float(u3)) + float(u4)) + float(b1)) + float(b2)) + float(b3)) + float(b4) == 16.0 ? colorGreen : colorRed;
}
