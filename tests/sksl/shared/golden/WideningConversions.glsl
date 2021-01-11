
out vec4 sk_FragColor;
float f = 1.0;
void main() {
    float h = f;
    sk_FragColor.x = h * 0.5;
    int i = int(h);
    sk_FragColor.x = float(i) * 0.5;
    uint ui = uint(i);
    sk_FragColor.x = float(ui) * 0.5;
    int s = int(ui);
    sk_FragColor.x = float(s) * 0.5;
    uint us = uint(s);
    sk_FragColor.x = float(us) * 0.5;
    int b = int(us);
    sk_FragColor.x = float(b) * 0.5;
    uint ub = uint(b);
    sk_FragColor.x = float(ub) * 0.5;
}
