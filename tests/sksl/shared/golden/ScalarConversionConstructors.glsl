
out vec4 sk_FragColor;
float f = 1.0;
int i = 1;
bool b = true;
void main() {
    float f1 = f;
    float f2 = float(i);
    float f3 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(b);
    bool b1 = bool(f);
    bool b2 = bool(i);
    bool b3 = b;
    sk_FragColor.x = (f1 + f2) + f3;
    sk_FragColor.x = float((i1 + i2) + i3);
    sk_FragColor.x = float((b1 || b2) || b3);
}
