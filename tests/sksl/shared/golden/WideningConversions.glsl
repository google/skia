
out vec4 sk_FragColor;
float f = 1.0;
void main() {
    float h = f;
    int i = int(h);
    uint ui = uint(i);
    int s = int(ui);
    uint us = uint(s);
    int b = int(us);
    uint ub = uint(b);
    sk_FragColor.x = ((((((f * h) * float(i)) * float(ui)) * float(s)) * float(us)) * float(b)) * float(ub);
}
