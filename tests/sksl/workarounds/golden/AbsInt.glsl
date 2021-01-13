#version 400
int _absemulation(int x) {
    return x * sign(x);
}
out vec4 sk_FragColor;
uniform int ui;
uniform float uf;
void main() {
    int i = _absemulation(ui);
    float f = abs(uf);
    sk_FragColor.x = float(i);
    sk_FragColor.y = f;
}
