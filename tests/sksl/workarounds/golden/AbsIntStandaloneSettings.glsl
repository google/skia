
out vec4 sk_FragColor;
uniform int ui;
uniform float uf;
void main() {
    int i = abs(ui);
    float f = abs(uf);
    sk_FragColor.x = float(i);
    sk_FragColor.y = f;
}
