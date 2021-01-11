
out vec4 sk_FragColor;
uint ub = 1u;
void main() {
    int b = int(ub);
    sk_FragColor.x = float(b);
    uint us = uint(b);
    sk_FragColor.x = float(us);
    int s = int(us);
    sk_FragColor.x = float(s);
    uint ui = uint(s);
    sk_FragColor.x = float(ui);
    int i = int(ui);
    sk_FragColor.x = float(i);
    float h = float(i);
    sk_FragColor.x = h;
    float f = h;
    sk_FragColor.x = f;
}
