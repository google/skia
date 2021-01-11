
out vec4 sk_FragColor;
float f = 1.0;
void main() {
    float h = f;
    sk_FragColor.x = h;
    int i = int(h);
    sk_FragColor.x = float(i);
    uint ui = uint(i);
    sk_FragColor.x = float(ui);
    int s = int(ui);
    sk_FragColor.x = float(s);
    uint us = uint(s);
    sk_FragColor.x = float(us);
    int b = int(us);
    sk_FragColor.x = float(b);
    uint ub = uint(b);
    sk_FragColor.x = float(ub);
}
