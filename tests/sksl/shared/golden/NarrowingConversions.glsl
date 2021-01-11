
out vec4 sk_FragColor;
uint ub = 1u;
void main() {
    int b = int(ub);
    uint us = uint(b);
    int s = int(us);
    uint ui = uint(s);
    int i = int(ui);
    float h = float(i);
    float f = h;
    sk_FragColor.x = ((((((f * h) * float(i)) * float(ui)) * float(s)) * float(us)) * float(b)) * float(ub);
}
