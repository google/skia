
out vec4 sk_FragColor;
float f = 1.0;
int i = 1;
uint u = 1u;
bool b = true;
void main() {
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
    sk_FragColor.x = ((f1 + f2) + f3) + f4;
    sk_FragColor.x = float(((i1 + i2) + i3) + i4);
    sk_FragColor.x = float(((u1 + u2) + u3) + u4);
    sk_FragColor.x = float(((b1 || b2) || b3) || b4);
}
