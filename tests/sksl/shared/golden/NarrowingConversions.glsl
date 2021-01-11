
out vec4 sk_FragColor;
uint ub = 1u;
void main() {
    int b = int(ub);
    sk_FragColor.x = float(b) * 0.5;
    uint us = uint(b);
    sk_FragColor.x = float(us) * 0.5;
    int s = int(us);
    sk_FragColor.x = float(s) * 0.5;
    uint ui = uint(s);
    sk_FragColor.x = float(ui) * 0.5;
    int i = int(ui);
    sk_FragColor.x = float(i) * 0.5;
    float h = float(i);
    sk_FragColor.x = h * 0.5;
    float f = h;
    sk_FragColor.x = f * 0.5;
}
